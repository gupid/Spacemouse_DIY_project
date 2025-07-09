#include "CONFIG.h"
#include "HAL.h"
#include "hiddev.h"
#include "ble_hid_app.h"
#include "main.h"
#include "usb_hid_composite.h"
#include "motion_engine.h"
#include "mode_manager.h"
#include "keyboard.h"
#include "ws2812b_control.h"

MotionEngine_State mouse_motion;
volatile uint8_t g_ahrs_update_flag = 0;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

/*********************************************************************
 * @fn      WA_TIM_PeriodCallBack
 *
 * @brief   定时器周期中断回调函数 (我们需要重写这个__weak_symbol函数)
 *
 * @param   watime - 定时器句柄
 *
 * @return  none
 */
void WA_TIM_PeriodCallBack(TIM_HandleDef *watime)
{
    // 判断是否是我们用于AHRS的定时器TMR1触发了中断
    if (watime == &watime1)
    {
        // 在中断里只做最简单的事：设置标志位
        g_ahrs_update_flag = 1;
    }
}

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__((noinline))
void Main_Circulation()
{
    while(1)
    {
       ModeManager_MainLoop();
    }
}

int main(void)
{
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz);
#if(defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    WA_UART_Init(&wauart1,115200);
#endif
    WA_TIM_IT_Init(&watime1, WA_TIMER, Pin_Disable, 50, null, null);
    //键盘0，USB1，定时器（鼠标）2
    PFIC_SetPriority(TMR1_IRQn,2);
    PFIC_DisableIRQ(TMR1_IRQn);
    Keyboard_Init();
    WS2812B_SetAllLeds(120,0,0);
    ModeManager_Init();
    MotionEngine_Init(&mouse_motion);
    Main_Circulation(); // 进入主循环
}