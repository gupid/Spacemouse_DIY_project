// usb_hid_composite.h

#ifndef __USB_HID_COMPOSITE_H
#define __USB_HID_COMPOSITE_H

#include "CH58x_common.h"
// --- Public Function Prototypes ---

/**
 * @brief 初始化USB HID复合设备（键鼠）
 * @note  该函数会初始化USB引脚、设置端点缓冲区并使能USB设备。
 */
void usb_hid_composite_init(void);

/**
 * @brief 通过USB发送鼠标报告
 * @param buttons - 按键状态 (bit0: 左键, bit1: 右键, bit2: 中键)
 * @param dx      - X轴相对位移 (-127 to 127)
 * @param dy      - Y轴相对位移 (-127 to 127)
 * @param wheel   - 滚轮相对位移 (-127 to 127)
 */
void usb_hid_composite_report_mouse(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);

/**
 * @brief 发送一个包含修饰键和普通键的完整键盘报告
 * * @param modifier - 修饰键字节 (例如 MOD_LEFT_CTRL)
 * @param key_code - 普通按键的HID Usage ID (例如 KEY_C)
 */
void usb_hid_report_keyboard(uint8_t modifier, uint8_t key_code);

//直接发送报告
void usb_hid_send_report(const uint8_t* report);

/**
 * @brief 设备模式唤醒主机
 */
void usb_hid_composite_wakeup(void);

void usb_hid_report_axes(int16_t trans[3], int16_t rot[3]);
void usb_hid_report_buttons(uint32_t buttons);
void usb_hid_send_spacemouse_report(const uint8_t* report, uint8_t len);
#endif // __USB_HID_COMPOSITE_H