#include "ws2812b_control.h"

// ================== 用户配置 ==================
#define NUM_LEDS            25      // 定义您键盘上串联的LED总数
#define WS2812B_PIN         GPIO_Pin_2 // 使用 PA2 引脚
// ==============================================


// 根据系统时钟频率 FREQ_SYS 计算延时所需的 CPU 周期数
//[cite_start]// g_1us 在您的 PWM.h 中定义为 (FREQ_SYS / 1000000) [cite: 4]
#define CYCLES_PER_US       (FREQ_SYS / 1000000)
#define NS_PER_CYCLE        (1000000000 / FREQ_SYS)

// WS2812B 时序定义 (单位：纳秒 ns)
#define T0H                 400     // 码 '0' 高电平时间
#define T0L                 850     // 码 '0' 低电平时间
#define T1H                 800     // 码 '1' 高电平时间
#define T1L                 450     // 码 '1' 低电平时间
#define RESET_PULSE         300000  // 复位信号时长 (300us)

// 将纳秒转换为需要的CPU延时周期数
#define T0H_CYCLES          (T0H / NS_PER_CYCLE)
#define T0L_CYCLES          (T0L / NS_PER_CYCLE)
#define T1H_CYCLES          (T1H / NS_PER_CYCLE)
#define T1L_CYCLES          (T1L / NS_PER_CYCLE)
#define RESET_CYCLES        (RESET_PULSE / NS_PER_CYCLE)


// 使用内联汇编实现的精确延时函数 (RISC-V 版本)
// __attribute__((always_inline)) 建议编译器始终内联，减少函数调用开销
static inline void __attribute__((always_inline)) delay_cycles(uint32_t cycles)
{
    __asm__ volatile(
        "1: \n"
        "   addi %0, %0, -1 \n"  // addi: add immediate, 立即数加法，这里用-1实现减法
        "   bnez %0, 1b \n"      // bnez: branch if not equal to zero, 如果结果非零则跳转
        : "+r"(cycles)
    );
}

// 发送单个比特
static void WS2812B_SendBit(bool bit)
{
    if (bit) // 发送 '1'
    {
        GPIOA_SetBits(WS2812B_PIN);
        delay_cycles(T1H_CYCLES);
        GPIOA_ResetBits(WS2812B_PIN);
        delay_cycles(T1L_CYCLES);
    }
    else // 发送 '0'
    {
        GPIOA_SetBits(WS2812B_PIN);
        delay_cycles(T0H_CYCLES);
        GPIOA_ResetBits(WS2812B_PIN);
        delay_cycles(T0L_CYCLES);
    }
}

// 发送一个像素的24位颜色数据 (GRB顺序)
static void WS2812B_SendPixel(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = (g << 16) | (r << 8) | b;
    for (int i = 23; i >= 0; i--)
    {
        WS2812B_SendBit((color >> i) & 1);
    }
}

// 主函数：设置所有LED颜色
void WS2812B_SetAllLeds(uint8_t r, uint8_t g, uint8_t b)
{
    // 1. 初始化 PB10 为推挽输出模式
    GPIOA_ModeCfg(WS2812B_PIN, GPIO_ModeOut_PP_5mA);

    // 2. 关闭全局中断，防止在发送数据时被中断干扰时序
    PFIC_DisableAllIRQ();

    // 3. 循环为每个LED发送相同的颜色数据
    for (int i = 0; i < NUM_LEDS; i++)
    {
        WS2812B_SendPixel(r, g, b);
    }

    // 4. 发送完成后，重新开启全局中断
    PFIC_EnableAllIRQ();

    // 5. 发送复位脉冲，让所有LED同时更新颜色
    GPIOA_ResetBits(WS2812B_PIN);
    delay_cycles(RESET_CYCLES);
}