#ifndef __WS2812B_CONTROL_H
#define __WS2812B_CONTROL_H

#include "CH58x_common.h"
#include "main.h"

/**
 * @brief 初始化WS2812B的硬件驱动（TIM3 PWM模式 和 Timer内置DMA）。
 * 此函数应在系统初始化时调用一次。
 */
void WS2812B_Init(void);

/**
 * @brief 设置所有串联的 WS2812B LED 的颜色。
 *
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 *
 * @note 此函数为非阻塞函数。它会填充数据并启动DMA传输，CPU可立即执行后续代码。
 * 在DMA传输完成前（可通过 WS2812B_IsBusy() 查询），请勿再次调用。
 */
void WS2812B_SetAllLeds(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 设置缓冲区中单个LED的颜色（高级用法）。
 * 此函数仅修改内存中的颜色数据，需要调用 WS2812B_Refresh() 来更新显示。
 * @param led_index LED的索引 (0 to NUM_LEDS-1)
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 */
void WS2812B_SetPixelColor(uint16_t led_index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 将缓冲区中的数据通过DMA发送出去，刷新所有LED的显示。
 */
void WS2812B_Refresh(void);

/**
 * @brief 查询DMA是否正在传输数据。
 *
 * @return 1: 忙, 0: 空闲
 */
uint8_t WS2812B_IsBusy(void);

#endif