// 文件名: main.c (用于接收器Dongle)

#include "CONFIG.h"
#include "HAL.h"
#include "rf_middleware.h"
#include "usb_hid_composite.h"

// --- RF 配置 (必须与发射器完全一致) ---
#define RF_ACCESS_ADDRESS   0x4B65794D  // 与发射器一致的地址
#define RF_CHANNEL          40          // 与发射器一致的信道

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
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz);

    CH58x_BLEInit();
    HAL_Init();

    // --- 2. USB HID复合设备初始化 ---
    usb_hid_composite_init();
    PRINT("USB HID Composite Dongle Initialized.\n");

    // --- 3. 2.4G RF模块初始化 ---
    RF_MW_Init(RF_ACCESS_ADDRESS, RF_CHANNEL, OnDataReceived_RF);
    PRINT("RF Receiver Initialized. Listening on Channel %d.\n", RF_CHANNEL);

    // --- 4. 进入主循环 ---
    while(1)
    {
        // TMOS系统任务轮询，驱动RF库运行
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      OnDataReceived_RF
 *
 * @brief   RF数据接收回调函数 (核心逻辑)
 *
 * @param   data    - 指向接收到数据的指针
 * @param   length  - 接收到的数据长度
 * @param   rssi    - 信号强度
 *
 * @return  none
 */
void OnDataReceived_RF(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // 根据接收到的数据包长度，判断其类型并转发到对应的USB端点

    if (length == 8)
    {
        // 长度为8字节，判定为键盘报告
        // 直接通过USB端点1发送给主机
        usb_hid_send_report(data);
    }
    else if (length == 7)
    {
        // 长度为7字节，判定为空间鼠标的 "平移" 或 "旋转" 报告
        // data[0] 是 Report ID (0x01 或 0x02)
        // 通过USB端点2发送给主机
        usb_hid_send_spacemouse_report(data, length);
    }

}