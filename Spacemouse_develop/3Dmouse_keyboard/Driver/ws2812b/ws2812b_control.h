#ifndef __WS2812B_CONTROL_H
#define __WS2812B_CONTROL_H

#include "main.h" // 引入您的主头文件，以获取 FREQ_SYS 和 GPIO 相关定义

/**
 * @brief 设置所有串联的 WS2812B LED 的颜色。
 *
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 *
 * @note 此函数会占用CPU，执行期间会关闭全局中断。
 * @warning 此函数使用 PB10 引脚，与 USB UD- 功能冲突。
 * 请仅在确认USB功能完全不使用的情况下调用此函数。
 */
void WS2812B_SetAllLeds(uint8_t r, uint8_t g, uint8_t b);

#endif