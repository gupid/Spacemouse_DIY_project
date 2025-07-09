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

#endif // __KEYBOARD_H