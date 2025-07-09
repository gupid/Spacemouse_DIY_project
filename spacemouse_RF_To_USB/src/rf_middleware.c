/********************************** (C) COPYRIGHT *******************************
 * @file     rf_middleware.c
 * @author   WCH, organized by Gemini
 * @version  V1.2
 * @date     2025/07/03
 * @brief    RF通信中间件，封装了2.4G库的基本收发功能。
 *******************************************************************************/

#include "rf_middleware.h"
#include "wchrf.h"
#include "hal.h"
#include "CONFIG.h"

// --- 内部宏定义和全局变量 ---
static tmosTaskID g_rf_task_id;
static rfRoleParam_t g_rf_param;
static rfipTx_t      g_rf_tx_param;
static rfipRx_t      g_rf_rx_param;
//一次发送和接收长度不超过此限制，即发送不大于64字节、接收不大于264字节
__attribute__((__aligned__(4))) static uint8_t g_tx_buffer[64];
__attribute__((__aligned__(4))) static uint8_t g_rx_buffer[264];

static rf_mw_data_callback_t g_data_recv_callback = NULL;
static volatile bool g_is_tx_busy = false;

// --- 内部函数声明 ---
__HIGH_CODE static void rf_mw_tx_start_internal(const uint8_t *pBuf, uint8_t length);
__HIGH_CODE static void rf_mw_rx_start_internal(void);
__HIGH_CODE static void rf_mw_rx_process_data(void);
__HIGH_CODE static void rf_mw_process_callback(rfRole_States_t sta, uint8_t id);
static tmosEvents rf_mw_process_event(tmosTaskID task_id, tmosEvents events);

// --- API函数实现 ---

void RF_MW_Init(uint32_t accessAddress, uint8_t channel, rf_mw_data_callback_t data_cb) {
    g_rf_task_id = TMOS_ProcessEventRegister(rf_mw_process_event);
    g_data_recv_callback = data_cb;
    
    rfRoleConfig_t conf = {0};
    conf.TxPower = LL_TX_POWEER_4_DBM;
    conf.rfProcessCB = rf_mw_process_callback;
    conf.processMask = RF_STATE_RX | RF_STATE_RX_CRCERR | RF_STATE_TX_FINISH | RF_STATE_TIMEOUT;
    RFRole_BasicInit(&conf);

    g_rf_param.accessAddress = accessAddress;
    g_rf_param.crcInit = 0x555555;
    g_rf_param.properties = LLE_MODE_PHY_2M;
    g_rf_param.sendInterval = 1999 * 2;
    g_rf_param.sendTime = 10 * 2;
    RFRole_SetParam(&g_rf_param);

    g_rf_tx_param.accessAddress = g_rf_param.accessAddress;
    g_rf_tx_param.crcInit = g_rf_param.crcInit;
    g_rf_tx_param.properties = g_rf_param.properties;
    g_rf_tx_param.sendCount = 1;
    g_rf_tx_param.txDMA = (uint32_t)g_tx_buffer;
    g_rf_tx_param.frequency = channel;

    g_rf_rx_param.accessAddress = g_rf_param.accessAddress;
    g_rf_rx_param.crcInit = g_rf_param.crcInit;
    g_rf_rx_param.properties = g_rf_param.properties;
    g_rf_rx_param.rxDMA = (uint32_t)g_rx_buffer;
    g_rf_rx_param.rxMaxLen = sizeof(g_tx_buffer) - 1;
    g_rf_rx_param.frequency = channel;
    g_rf_rx_param.timeOut = 0;

    PFIC_EnableIRQ(BLEB_IRQn);
    PFIC_EnableIRQ(BLEL_IRQn);

    RF_MW_EnableReceiver(true);
    
    PRINT("RF Middleware Initialized. TaskID=%d\n", g_rf_task_id);
}

bool RF_MW_SendData(const uint8_t *data, uint8_t length) {
    if (g_is_tx_busy || length == 0 || length > sizeof(g_tx_buffer)) {
        return false;
    }
    g_is_tx_busy = true;
    tmos_memcpy(g_tx_buffer, data, length);
    rf_mw_tx_start_internal(g_tx_buffer, length);
    return true;
}

void RF_MW_EnableReceiver(bool enable) {
    if (enable) {
        tmos_set_event(g_rf_task_id, (1 << 1));
    } else {
        // RFIP_RxDisable(); // 实际的关闭接收函数
    }
}

// --- 内部核心函数 ---

__HIGH_CODE static void rf_mw_tx_start_internal(const uint8_t *pBuf, uint8_t length) {
    g_rf_tx_param.txDMA = (uint32_t)pBuf;
    RFIP_SetTxStart();
    RFIP_SetTxParm(&g_rf_tx_param);
}

__HIGH_CODE static void rf_mw_rx_start_internal(void) {
    RFIP_SetRx(&g_rf_rx_param);
}

__HIGH_CODE static void rf_mw_rx_process_data(void) {
    int8_t rssi = RFIP_ReadRssi();
    uint8_t *pData = (uint8_t *)g_rf_rx_param.rxDMA;
    // 此处假设长度信息包含在数据包中，需要根据你的协议来解析
    uint8_t data_len = pData[1]; 

    if (g_data_recv_callback != NULL) {
        g_data_recv_callback(pData, data_len, rssi);
    }
}

__HIGH_CODE static void rf_mw_process_callback(rfRole_States_t sta, uint8_t id) {
    if (sta & RF_STATE_RX) {
        rf_mw_rx_process_data();
        rf_mw_rx_start_internal();
    } else if (sta & RF_STATE_RX_CRCERR) {
        rf_mw_rx_start_internal();
    } else if (sta & RF_STATE_TX_FINISH) {
        g_is_tx_busy = false;
        rf_mw_rx_start_internal();
    } else if (sta & RF_STATE_TIMEOUT) {
        rf_mw_rx_start_internal();
    }
}

static tmosEvents rf_mw_process_event(tmosTaskID task_id, tmosEvents events) {
    if (events & SYS_EVENT_MSG) {
        uint8_t *msgPtr = tmos_msg_receive(task_id);
        if (msgPtr) tmos_msg_deallocate(msgPtr);
        return events ^ SYS_EVENT_MSG;
    }
    if (events & (1 << 1)) {
        rf_mw_rx_start_internal();
        return events ^ (1 << 1);
    }
    return 0;
}