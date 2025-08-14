// rf_middleware.c

#include "rf_middleware.h"
#include "CONFIG.h" // 依赖底层配置
#include "RF_PHY.h" // 依赖底层驱动

// --- 内部事件定义 ---
#define MW_START_RX_EVT         (1 << 0) // 启动/重启接收的事件
#define MW_RX_DATA_READY_EVT    (1 << 1) // 接收到数据，准备处理的事件
#define RF_MY_ACCESS_ADDRESS  0x4B65794D // "KeyM"的ASCII码，一个自定义地址
#define RF_MY_CHANNEL   40    // 选择一个信道 (避开Wi-Fi)
// --- 静态全局变量 ---
static tmosTaskID g_mw_task_id;                  // 中间件的TMOS任务ID
static rf_mw_data_callback_t g_data_recv_callback = NULL; // 用户注册的数据接收回调函数
static volatile bool g_is_tx_busy = false;      // 发送忙标志，volatile关键字确保多线程/中断安全访问

// 接收缓冲区，用于从中断上下文安全地传递数据到任务上下文
__attribute__((__aligned__(4))) static uint8_t g_rx_buffer[RF_MW_MAX_PAYLOAD_SIZE];
static uint8_t g_rx_len = 0;
static int8_t g_rx_rssi = 0;

// --- 内部函数声明 ---
static tmosEvents rf_mw_process_event(tmosTaskID task_id, tmosEvents events);
static void rf_mw_status_callback(uint8_t sta, uint8_t crc, uint8_t *rxBuf);
static void rf_mw_start_rx(void);

/**
 * @brief RF底层状态回调函数
 * @note  此函数在中断上下文中被调用，应尽量保持简短。
 * 主要工作是解析状态，必要时设置事件标志给TMOS任务处理。
 */
static void rf_mw_status_callback(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        // --- 发送模式事件 (Transmitter's perspective) ---
        case TX_MODE_TX_FINISH:
            // 在自动模式下，这个事件通常不会发生。我们会等待RX_DATA或RX_TIMEOUT
            break;

        case TX_MODE_RX_DATA: // 【新增处理】发送成功，并且收到了对方的ACK
            g_is_tx_busy = false; // 清除发送忙标志，表示整个事务完成
            // 可以在这里增加一个回调，通知上层应用“发送成功并已确认”
            tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
            break;

        case TX_MODE_RX_TIMEOUT: // 【新增处理】发送成功，但等待ACK超时
        case TX_MODE_TX_FAIL:    // 发送失败
            g_is_tx_busy = false; // 同样清除标志，但表示事务失败
            // 可以在这里增加回调，通知上层“发送失败，未收到确认”
            tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
            break;

        // --- 接收模式事件 (Receiver's perspective) ---
        case RX_MODE_RX_DATA: // 成功接收到一个数据包
            if (crc == 0)
            {
                g_rx_rssi = (int8_t)rxBuf[0];
                uint8_t len = rxBuf[1];

                if (len > 0 && len <= RF_MW_MAX_PAYLOAD_SIZE)
                {
                    g_rx_len = len;
                    tmos_memcpy(g_rx_buffer, rxBuf + 2, len);
                    tmos_set_event(g_mw_task_id, MW_RX_DATA_READY_EVT);
                }
                // 在这个case中不再重启接收，因为硬件会自动发送ACK，
                // 我们要等到ACK发送完成后（RX_MODE_TX_FINISH事件）再重启。
            }
            else
            {
                // CRC错误，直接重启接收，不发送ACK
                tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
            }
            break;
        
        case RX_MODE_TX_FINISH: 
            tmos_set_event(g_mw_task_id, MW_START_RX_EVT); // <-- 新增此行
            break;

        case RX_MODE_TX_FAIL: // 作为接收方，发送ACK失败
             // 同样需要重启接收，以防万一
            tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
            break;
    }
}
/**
 * @brief 启动RF接收
 */
static void rf_mw_start_rx(void)
{
    RF_Shut(); // 为确保状态正确，先关闭RF
    // 调用底层RF_Rx函数，这里的TX_DATA和长度参数在纯接收模式下无意义
    RF_Rx(NULL, 0, 0xFF, 0xFF);
}

/**
 * @brief 中间件的TMOS事件处理函数
 */
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

    if(events & MW_RX_DATA_READY_EVT)
    {
        // 如果用户注册了回调函数，则调用它
        if (g_data_recv_callback)
        {
            g_data_recv_callback(g_rx_buffer, g_rx_len, g_rx_rssi);
        }
        // 处理完数据后，立即重新启动接收，以准备接收下一个数据包
        //tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
        
        return (events ^ MW_RX_DATA_READY_EVT);
    }
    
    if(events & MW_START_RX_EVT)
    {
        // 启动RF接收
        rf_mw_start_rx();
        return (events ^ MW_START_RX_EVT);
    }

    return 0;
}

// --- 公共API函数实现 ---

void RF_MW_Init(rf_mw_data_callback_t data_cb)
{
    rfConfig_t rf_Config;
    
    // 1. 注册TMOS任务和用户回调
    g_mw_task_id = TMOS_ProcessEventRegister(rf_mw_process_event);
    g_data_recv_callback = data_cb;
    
    // 2. 配置RF参数 (与原RF_PHY.c保持一致)
    tmos_memset(&rf_Config, 0, sizeof(rfConfig_t));
    rf_Config.accessAddress = RF_MY_ACCESS_ADDRESS;
    rf_Config.CRCInit = 0x555555;
    rf_Config.Channel = RF_MY_CHANNEL;
    rf_Config.Frequency = 2482000;
    // 使用基础模式，不进行自动应答，发送和接收是独立操作
    rf_Config.LLEMode = LLE_MODE_AUTO; 
    rf_Config.rfStatusCB = rf_mw_status_callback; // 注册我们自己的内部回调
    rf_Config.RxMaxlen = 251; // 底层驱动需要的最大长度
    
    // 3. 应用配置
    RF_Config(&rf_Config);
    
    // 4. 初始化完成后，立即启动第一次接收
    tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
    
    PRINT("RF Middleware Initialized.\n");
}

bool RF_MW_SendData(const uint8_t *data, uint8_t length)
{
    // 如果当前正在发送，或者数据为空/超长，则返回失败
    if (g_is_tx_busy || data == NULL || length == 0 || length > RF_MW_MAX_PAYLOAD_SIZE)
    {
        return false;
    }
    
    g_is_tx_busy = true;
    RF_Shut();
    
    // 调用底层发送函数，这是一个非阻塞操作
    // RF_Tx成功启动会返回0
    if (RF_Tx((uint8_t *)data, length, 0xFF, 0xFF) == 0)
    {
        return true;
    }
    else
    {
        // 如果底层函数未能成功启动发送，则清除标志并返回失败
        g_is_tx_busy = false;
        tmos_set_event(g_mw_task_id, MW_START_RX_EVT);
        return false;
    }
}

void RF_MW_Deinit(void)
{
    // 停止中间件自己的TMOS任务
    if (g_mw_task_id != 0) {
        tmos_stop_task(g_mw_task_id, MW_START_RX_EVT | MW_RX_DATA_READY_EVT);
    }

    // 调用底层关闭RF硬件
    RF_Shut();

    // 清理内部状态
    g_data_recv_callback = NULL;
    g_is_tx_busy = false;

    PRINT("RF Middleware Deinitialized.\n");
}