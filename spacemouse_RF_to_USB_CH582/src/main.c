// 文件名: main.c (环形缓冲区最终方案 - 完整无省略)
// 描述: 使用环形缓冲区(FIFO队列)彻底解决高速、连续数据包丢失问题。

#include "CONFIG.h"
#include "HAL.h"
#include "rf_middleware.h"
#include "usb_hid_composite.h"

// --- USB发送状态机定义 ---
typedef enum {
    USB_SEND_IDLE,
    USB_SEND_KB,
    USB_SEND_SPACEMOUSE_ROT,
    USB_SEND_SPACEMOUSE_TRANS,
    USB_SEND_SPACEMOUSE_ZERO_ROT,
    USB_SEND_SPACEMOUSE_ZERO_TRANS
} usb_send_state_t;

// --- 数据包结构体定义 ---
#define MAX_PACKET_LEN 12
typedef struct {
    uint8_t data[MAX_PACKET_LEN];
    uint8_t length;
} rf_packet_t;


// --- RF数据环形缓冲区 (FIFO队列) ---
#define RF_QUEUE_SIZE 8 // 可缓存8个数据包，足以应对高速按键操作
static volatile rf_packet_t g_rf_queue[RF_QUEUE_SIZE];
static volatile uint8_t g_rf_queue_head = 0; // 消费者(主循环)从此位置读取
static volatile uint8_t g_rf_queue_tail = 0; // 生产者(RF回调)向此位置写入

// --- 全局变量 ---
static volatile usb_send_state_t g_usb_send_state = USB_SEND_IDLE;
__attribute__((aligned(4))) static int16_t g_trans_vals[3];
__attribute__((aligned(4))) static int16_t g_rot_vals[3];
__attribute__((aligned(4))) static uint8_t g_usb_report_buffer[8];

// --- CDC (虚拟串口) 相关全局变量 ---
#define CDC_RX_BUF_SIZE 64
uint8_t g_cdc_rx_buf[CDC_RX_BUF_SIZE];
volatile uint8_t g_cdc_rx_len = 0;
volatile bool g_cdc_data_received_flag = false;

// --- RF 配置 ---
#define RF_ACCESS_ADDRESS   0x4B65794D
#define RF_CHANNEL          40

// --- 函数声明 ---
void OnDataReceived_RF(const uint8_t *data, uint8_t length, int8_t rssi);

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];


/*********************************************************************
 * @fn      main
 *
 * @brief   接收器主函数
 *
 * @return  none
 */
int main(void)
{
    // --- 1. 系统初始化 ---
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    //HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    CH58X_BLEInit();
    HAL_Init();
    // GPIOA_SetBits(GPIO_Pin_9);
    // GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    // GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    // UART1_DefInit();

    // --- 2. USB HID复合设备初始化 ---
    usb_hid_composite_init();
    //PRINT("USB HID Composite Dongle Initialized.\n");
    RF_RoleInit();
    
    // --- 3. 2.4G RF模块初始化 ---
    RF_MW_Init(OnDataReceived_RF);
    //PRINT("Receiver Initialized with Ring Buffer.\n");
    
    // --- 4. 进入主循环 ---
    while(1)
    {
        // 4.1 核心：始终运行TMOS，处理底层RF事件
        TMOS_SystemProcess();

        // 4.2 检查队列是否为空，以及USB是否空闲，以启动新的发送任务
        if ((g_rf_queue_head != g_rf_queue_tail) && (g_usb_send_state == USB_SEND_IDLE)) {
            
            // 从队列头部取出一个数据包 (出队操作)
            rf_packet_t current_packet = g_rf_queue[g_rf_queue_head];
            g_rf_queue_head = (g_rf_queue_head + 1) % RF_QUEUE_SIZE;

            // 根据取出的数据包，设置USB状态机
            if (current_packet.length == 8) {
                memcpy(g_usb_report_buffer, current_packet.data, 8);
                g_usb_send_state = USB_SEND_KB;
            } else if (current_packet.length == 12) {
                memcpy(g_trans_vals, current_packet.data, 6);
                memcpy(g_rot_vals, current_packet.data + 6, 6);
                
                if (g_rot_vals[0] != 0 || g_rot_vals[1] != 0 || g_rot_vals[2] != 0) {
                    g_usb_send_state = USB_SEND_SPACEMOUSE_ROT;
                } else if (g_trans_vals[0] != 0 || g_trans_vals[1] != 0 || g_trans_vals[2] != 0) {
                    g_usb_send_state = USB_SEND_SPACEMOUSE_TRANS;
                }
            }
        }
        
        // 4.3 状态机：执行USB发送任务
        switch (g_usb_send_state)
        {
            case USB_SEND_KB:
                if (usb_hid_is_ep1_ready()) { 
                    usb_hid_send_report(g_usb_report_buffer);
                    g_usb_send_state = USB_SEND_IDLE;
                }
                break;

            case USB_SEND_SPACEMOUSE_ROT:
                if (usb_hid_is_ep2_ready()) { 
                    usb_hid_send_spacemouse_report(0x02, g_rot_vals);
                    g_usb_send_state = USB_SEND_SPACEMOUSE_ZERO_TRANS;
                }
                break;

            case USB_SEND_SPACEMOUSE_TRANS:
                if (usb_hid_is_ep2_ready()) {
                    usb_hid_send_spacemouse_report(0x01, g_trans_vals);
                    g_usb_send_state = USB_SEND_SPACEMOUSE_ZERO_ROT;
                }
                break;

            case USB_SEND_SPACEMOUSE_ZERO_ROT:
                 if (usb_hid_is_ep2_ready()) {
                    usb_hid_send_spacemouse_report(0x02, NULL);
                    g_usb_send_state = USB_SEND_IDLE;
                }
                break;
            
            case USB_SEND_SPACEMOUSE_ZERO_TRANS:
                if (usb_hid_is_ep2_ready()) {
                    usb_hid_send_spacemouse_report(0x01, NULL);
                    g_usb_send_state = USB_SEND_IDLE;
                }
                break;
                
            case USB_SEND_IDLE:
            default:
                // 空闲状态，等待新任务
                break;
        }

        // 4.4 处理从USB CDC收到的数据 
        if (g_cdc_data_received_flag) {
            RF_MW_SendData(g_cdc_rx_buf, g_cdc_rx_len);
            g_cdc_data_received_flag = false;
        }
    }
}


/*********************************************************************
 * @fn      OnDataReceived_RF
 *
 * @brief   RF数据接收回调函数 (修改为向环形缓冲区入队)
 *
 * @return  none
 */
void OnDataReceived_RF(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // 计算下一个尾部指针的位置
    uint8_t next_tail = (g_rf_queue_tail + 1) % RF_QUEUE_SIZE;

    // 检查队列是否已满 (下一个尾部指针追上头部指针)
    if (next_tail == g_rf_queue_head) {
        // 队列已满，丢弃当前数据包。
        // 这种情况在正常使用中几乎不会发生，除非USB长时间阻塞。
        return; 
    }

    // 验证数据长度是否为有效载荷
    if ((length == 8 || length == 12) && data != NULL)
    {
        // 将数据包存入队列尾部 (入队操作)
        g_rf_queue[g_rf_queue_tail].length = length;
        memcpy((void*)g_rf_queue[g_rf_queue_tail].data, data, length);
        
        // 移动尾部指针，完成入队
        g_rf_queue_tail = next_tail;
    }
}