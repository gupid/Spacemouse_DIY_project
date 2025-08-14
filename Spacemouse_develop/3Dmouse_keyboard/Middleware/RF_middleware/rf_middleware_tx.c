// 文件名: rf_middleware_tx.c (最终修复版)
// 描述: 移除了应用层的越权硬件操作，修复了状态机死锁问题。

#include "rf_middleware_tx.h"
#include "CONFIG.h"
#include "motion_engine.h"
#include "mode_manager.h"
#include <stdlib.h>
#include <string.h>

extern MotionEngine_State mouse_motion;

// --- 内部事件定义 ---
#define MW_SPACEMOUSE_REPORT_EVT (1 << 0)
#define MW_SEND_NEXT_EVT         (1 << 1)
#define MW_RX_DATA_READY_EVT     (1 << 2)

#define RF_MY_ACCESS_ADDRESS  0x4B65794D
#define RF_MY_CHANNEL   40

// --- 优先级队列定义 ---
#define RF_TX_HIGH_PRIO_QUEUE_SIZE 4
#define RF_TX_LOW_PRIO_QUEUE_SIZE  4

typedef struct {
    uint8_t data[RF_MW_MAX_PAYLOAD_SIZE];
    uint8_t len;
} rf_packet_t;

static rf_packet_t g_tx_high_prio_queue[RF_TX_HIGH_PRIO_QUEUE_SIZE];
static volatile uint8_t g_tx_high_prio_head = 0;
static volatile uint8_t g_tx_high_prio_tail = 0;

static rf_packet_t g_tx_low_prio_queue[RF_TX_LOW_PRIO_QUEUE_SIZE];
static volatile uint8_t g_tx_low_prio_head = 0;
static volatile uint8_t g_tx_low_prio_tail = 0;

// --- 静态全局变量 ---
static tmosTaskID g_mw_task_id;
static rf_mw_data_callback_t g_data_recv_callback = NULL;
static volatile bool g_is_tx_busy = false;
static volatile bool g_send_rotation_next = false;
__attribute__((__aligned__(4))) static uint8_t g_rx_buffer[RF_MW_MAX_PAYLOAD_SIZE];
static volatile uint8_t g_rx_len = 0;
static volatile int8_t g_rx_rssi = 0;

int32_t sample_count_rf = 0;
static float yaw_offset = 0.0f, roll_offset = 0.0f, pitch_offset = 0.0f;
bool is_calibrated_rf = false;

// --- 内部函数声明 ---
static tmosEvents rf_mw_process_event(tmosTaskID task_id, tmosEvents events);
static void rf_mw_status_callback(uint8_t sta, uint8_t crc, uint8_t *rxBuf);
static void RF_MW_StartRX(void);
static void rf_mw_try_send_next(void);

// RF底层状态回调函数
static void rf_mw_status_callback(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case TX_MODE_RX_DATA:
        case TX_MODE_RX_TIMEOUT:
        case TX_MODE_TX_FAIL:
            g_is_tx_busy = false;
            tmos_set_event(g_mw_task_id, MW_SEND_NEXT_EVT);
            break;

        case RX_MODE_RX_DATA:
            if (crc == 0)
            {
                uint8_t len = rxBuf[1];
                if (len > 0 && len <= RF_MW_MAX_PAYLOAD_SIZE)
                {
                    g_rx_len = len;
                    g_rx_rssi = (int8_t)rxBuf[0];
                    tmos_memcpy((void*)g_rx_buffer, rxBuf + 2, len);
                    tmos_set_event(g_mw_task_id, MW_RX_DATA_READY_EVT);
                }
            }
            break;

        case RX_MODE_TX_FINISH:
        case RX_MODE_TX_FAIL:
            RF_MW_StartRX();
            break;
    }
}

// 启动RF接收的内部函数
static void RF_MW_StartRX(void)
{
    RF_Shut();
    RF_Rx(NULL, 0, 0xFF, 0xFF);
}

// 尝试发送下一个包，如果队列为空，则切换到接收模式
static void rf_mw_try_send_next(void)
{
    if (g_is_tx_busy) {
        return;
    }

    rf_packet_t* packet_to_send = NULL;

    if (g_tx_high_prio_head != g_tx_high_prio_tail) {
        packet_to_send = &g_tx_high_prio_queue[g_tx_high_prio_head];
        g_tx_high_prio_head = (g_tx_high_prio_head + 1) % RF_TX_HIGH_PRIO_QUEUE_SIZE;
    }
    else if (g_tx_low_prio_head != g_tx_low_prio_tail) {
        packet_to_send = &g_tx_low_prio_queue[g_tx_low_prio_head];
        g_tx_low_prio_head = (g_tx_low_prio_head + 1) % RF_TX_LOW_PRIO_QUEUE_SIZE;
    }

    if (packet_to_send != NULL) {
        g_is_tx_busy = true;
        RF_Shut(); // 发送前由状态机统一关闭RF，确保状态正确

        if (RF_Tx(packet_to_send->data, packet_to_send->len, 0xFF, 0xFF) != 0) {
            g_is_tx_busy = false; // 如果启动发送失败，立即清除标志并重新调度
            tmos_set_event(g_mw_task_id, MW_SEND_NEXT_EVT);
        }
    }
    else {
        RF_MW_StartRX(); // 队列为空，进入接收模式
    }
}

// 中间件的TMOS事件处理函数
static tmosEvents rf_mw_process_event(tmosTaskID task_id, tmosEvents events)
{
     if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            tmos_msg_deallocate(pMsg);
        }
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & MW_SEND_NEXT_EVT)
    {
        rf_mw_try_send_next();
        return (events ^ MW_SEND_NEXT_EVT);
    }

    if(events & MW_RX_DATA_READY_EVT)
    {
        if (g_data_recv_callback)
        {
            g_data_recv_callback((const uint8_t*)g_rx_buffer, g_rx_len, g_rx_rssi);
        }
        return (events ^ MW_RX_DATA_READY_EVT);
    }

    if(events & MW_SPACEMOUSE_REPORT_EVT)
    {
        ahrs_task();
        tmos_start_task(g_mw_task_id, MW_SPACEMOUSE_REPORT_EVT, 50);
        return (events ^ MW_SPACEMOUSE_REPORT_EVT);
    }
    return 0;
}

// --- 公共API函数实现 ---

void RF_MW_Init(rf_mw_data_callback_t data_cb)
{
    rfConfig_t rf_Config;
    g_mw_task_id = TMOS_ProcessEventRegister(rf_mw_process_event);
    g_data_recv_callback = data_cb;
    g_tx_high_prio_head = g_tx_high_prio_tail = 0;
    g_tx_low_prio_head = g_tx_low_prio_tail = 0;

    tmos_memset(&rf_Config, 0, sizeof(rfConfig_t));
    rf_Config.accessAddress = RF_MY_ACCESS_ADDRESS;
    rf_Config.CRCInit = 0x555555;
    rf_Config.Channel = RF_MY_CHANNEL;
    rf_Config.Frequency = 2482000;
    rf_Config.LLEMode = LLE_MODE_AUTO; 
    rf_Config.rfStatusCB = rf_mw_status_callback;
    rf_Config.RxMaxlen = 251;
    
    RF_Config(&rf_Config);
    
    tmos_start_task(g_mw_task_id, MW_SEND_NEXT_EVT, 10); // 初始启动状态机，进入接收模式
    tmos_start_task(g_mw_task_id, MW_SPACEMOUSE_REPORT_EVT, 500);
    PRINT("RF Middleware Initialized with Priority Queues.\n");
}

// 发送高优先级数据
bool RF_MW_SendHighPrioData(const uint8_t *data, uint8_t length)
{
    if (data == NULL || length == 0 || length > RF_MW_MAX_PAYLOAD_SIZE) {
        return false;
    }

    uint8_t next_tail = (g_tx_high_prio_tail + 1) % RF_TX_HIGH_PRIO_QUEUE_SIZE;
    if (next_tail == g_tx_high_prio_head) {
        return false; // 队列满
    }

    // 【已移除】此处不再有 if(was_empty) { RF_Shut(); }

    rf_packet_t* new_packet = &g_tx_high_prio_queue[g_tx_high_prio_tail];
    new_packet->len = length;
    tmos_memcpy(new_packet->data, data, length);
    g_tx_high_prio_tail = next_tail;

    tmos_set_event(g_mw_task_id, MW_SEND_NEXT_EVT);
    return true;
}

// 发送低优先级数据
bool RF_MW_SendData(const uint8_t *data, uint8_t length)
{
    if (data == NULL || length == 0 || length > RF_MW_MAX_PAYLOAD_SIZE) {
        return false;
    }
    
    uint8_t next_tail = (g_tx_low_prio_tail + 1) % RF_TX_LOW_PRIO_QUEUE_SIZE;
    if (next_tail == g_tx_low_prio_head) {
        return false; // 队列满
    }
    
    // 【已移除】此处不再有 if(was_empty) { RF_Shut(); }
    
    rf_packet_t* new_packet = &g_tx_low_prio_queue[g_tx_low_prio_tail];
    new_packet->len = length;
    tmos_memcpy(new_packet->data, data, length);
    g_tx_low_prio_tail = next_tail;
    
    tmos_set_event(g_mw_task_id, MW_SEND_NEXT_EVT);
    return true;
}

void RF_MW_Deinit(void)
{
    if (g_mw_task_id != 0) {
        tmos_stop_task(g_mw_task_id, MW_SPACEMOUSE_REPORT_EVT | MW_SEND_NEXT_EVT | MW_RX_DATA_READY_EVT);
    }
    RF_Shut();
    g_data_recv_callback = NULL;
    g_is_tx_busy = false;
    PRINT("RF Middleware Deinitialized.\n");
}