#include "ws2812b_control.h"

// ================== 用户配置 ==================
#define NUM_LEDS            25
#define WS2812B_DATA_PIN    GPIO_Pin_22
// ==============================================


// ================== 根据时钟和时序计算的参数 (Fsys=62.4MHz) ==================
#define TIMER_PERIOD_VALUE      77  // 定时器重载值: (62.4Mhz / 800kHz) - 1
#define PWM_0_DATA_VALUE        22  // '0'码的PWM比较值: (350ns / 1250ns) * 78
#define PWM_1_DATA_VALUE        50  // '1'码的PWM比较值: (800ns / 1250ns) * 78

#define RESET_PULSE_BITS        240 // 用于产生 >280us 复位信号的比特数
#define WS2812B_BUFFER_SIZE     (NUM_LEDS * 24 + RESET_PULSE_BITS)
// =========================================================================

// 定义DMA缓冲区, TMR3的DMA要求4字节对齐
__attribute__((aligned(4))) static uint32_t g_pwm_buffer[WS2812B_BUFFER_SIZE];

// DMA传输状态标志
volatile uint8_t g_ws2812b_dma_busy = 0;


/**
 * @brief  初始化WS2812B的硬件驱动（TMR3 PWM模式 和 Timer内置DMA）。
 * @note   此函数已根据官方CH58x_timer.h/c库文件进行精确修正。
 */
void WS2812B_Init(void)
{
    // // 1. 配置 PA2 为 TMR3 复用输出
    // // 1.1 将 TMR3 功能重映射到 PA2
    // GPIOPinRemap(ENABLE, RB_PIN_TMR3);
    // // 1.2 配置 PA2 为推挽输出模式
    // GPIOA_ModeCfg(WS2812B_DATA_PIN, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg(WS2812B_DATA_PIN, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(GPIO_PIN_2,GPIO_ModeOut_PP_5mA);
    GPIOA_ResetBits(GPIO_Pin_2);

    // 2. 配置 TMR3 工作在 PWM 模式;默认低电平，高电平有效，只重复一次
    TMR3_PWMInit(High_Level, PWM_Times_1); 

    // 2.2 设置PWM周期
    TMR3_PWMCycleCfg(TIMER_PERIOD_VALUE);

    // 2.3 使能PWM输出
    TMR3_PWMEnable();

    // 3. 配置 TMR3 的内置 DMA
    TMR3_DMACfg(ENABLE, (uint32_t)g_pwm_buffer, (uint32_t)(g_pwm_buffer + WS2812B_BUFFER_SIZE), Mode_Single);

    // 4. 配置并使能TMR3中断，用于在DMA传输结束后发出通知
    TMR3_ITCfg(ENABLE, TMR0_3_IT_DMA_END);
}

/**
 * @brief  填充缓冲区中单个LED的颜色数据。
 */
void WS2812B_SetPixelColor(uint16_t led_index, uint8_t r, uint8_t g, uint8_t b)
{
    if (led_index >= NUM_LEDS) return;

    uint32_t color_data = (uint32_t)g << 16 | (uint32_t)r << 8 | b;
    uint32_t buffer_offset = led_index * 24;

    for (int i = 23; i >= 0; i--)
    {
        g_pwm_buffer[buffer_offset] = ((color_data >> i) & 0x01) ? PWM_1_DATA_VALUE : PWM_0_DATA_VALUE;
        buffer_offset++;
    }
}

/**
 * @brief  将缓冲区中的数据通过DMA发送出去，刷新所有LED的显示。
 */
void WS2812B_Refresh(void)
{
    if (g_ws2812b_dma_busy) return;

    g_ws2812b_dma_busy = 1;

    for(int i = NUM_LEDS * 24; i < WS2812B_BUFFER_SIZE; i++)
    {
        g_pwm_buffer[i] = 0; // 填充末尾的RESET信号
    }

     //重新配置并使能DMA通道，确保其在单次模式下可以再次启动
    TMR3_DMACfg(ENABLE, (uint32_t)g_pwm_buffer, (uint32_t)(g_pwm_buffer + WS2812B_BUFFER_SIZE), Mode_Single);

    // 启动定时器计数，DMA将自动开始工作
    TMR3_Enable();
    PFIC_EnableIRQ(TMR3_IRQn);
    TMR3_ITCfg(ENABLE, TMR0_3_IT_DMA_END);
}

/**
 * @brief  设置所有串联的 WS2812B LED 的颜色。
 */
void WS2812B_SetAllLeds(uint8_t r, uint8_t g, uint8_t b)
{
    if (g_ws2812b_dma_busy) return;

    for (uint16_t i = 0; i < NUM_LEDS; i++)
    {
        WS2812B_SetPixelColor(i, r, g, b);
    }
    WS2812B_Refresh();
}

/**
 * @brief  查询DMA是否正在传输数据。
 */
uint8_t WS2812B_IsBusy(void)
{
    return g_ws2812b_dma_busy;
}

/**
 * @brief   TMR3中断服务函数，专门用于WS2812B驱动。
 */
__INTERRUPT
__HIGH_CODE
void TMR3_IRQHandler(void)
{
    // 检查是否是DMA传输完成中断
    if (TMR3_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR3_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR3_ClearITFlag(TMR0_3_IT_DMA_END);  // 清除中断标志
        g_ws2812b_dma_busy = 0;               // 更新软件标志位
    }
}