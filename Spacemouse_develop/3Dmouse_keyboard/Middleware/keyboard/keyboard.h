#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "main.h" // 引入您的主头文件
#include "CH450_driver.h"
#include "fifo.h"

extern I2C_HandleDef g_wai2c_keyboard;
extern KeyCodeFifo_t g_key_fifo;

// 定义一个结构体来表示一个按键事件
typedef struct {
    uint8_t row;        // 按键的行号 (0-7)
    uint8_t col;        // 按键的列号 (0-5)
    bool pressed;       // 按键状态: true为按下, false为释放
} KeyEvent_t;

/**
 * @brief  初始化键盘模块
 * @note   此函数会完成所有必要的底层初始化，包括GPIO, I2C, 中断以及CH450芯片。
 * @return 无
 */
void Keyboard_Init(void);

/**
 * @brief  从键盘事件队列中获取一个按键事件
 * @param  pEvent - 指向 KeyEvent_t 结构体的指针，用于存储获取到的事件
 * @return bool   - true: 成功获取到一个事件; false: 当前没有按键事件
 */
bool Keyboard_GetKeyEvent(KeyEvent_t *pEvent);

/**
 * @brief 将 HSV 颜色模型转换为 RGB 颜色模型
 * @param h 色相 (0.0f - 360.0f)
 * @param s 饱和度 (0.0f - 1.0f)
 * @param v 亮度 (0.0f - 1.0f)
 * @param r 返回的红色分量指针 (0-255)
 * @param g 返回的绿色分量指针 (0-255)
 * @param b 返回的蓝色分量指针 (0-255)
 */
void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

/**
 * @brief 更新并显示一帧彩虹呼吸灯动画。
 * @note  此函数为非阻塞函数。应在主循环中被反复调用。
 * 函数内部会检查DMA是否繁忙，若繁忙则直接返回。
 */
void WS2812B_RainbowBreathe_Update(void);

/**
 * @brief 设置“赛博朋克夜色”功能分区静态灯效
 * @note  此函数会根据 g_led_key_type_map 表为每个LED设置颜色。
 */
void Light_Set_FunctionalZoning_Cyberpunk(void);

void keyboard_LED_display();
void change_LED_display_mode();


#endif // __KEYBOARD_H