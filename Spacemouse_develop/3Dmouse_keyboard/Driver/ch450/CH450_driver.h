#ifndef __CH450_DRIVER_H
#define __CH450_DRIVER_H

#include "I2C.h"

/*
 * CH450 命令码定义
 */
#define CH450_SYSOFF    0x0400					// 关闭显示、关闭键盘
#define CH450_SYSON1    0x0401					// 开启显示
#define CH450_SYSON2    0x0403					// 开启显示、键盘
#define CH450_GET_KEY	0x0700					// 获取按键命令 (用于计算实际发送的命令)
#define CH450_CMD_READ_KEY  0x4F                // 实际发送的读取按键命令字节

#define CH450_DIG2      0x1200		            // 数码管位2显示,需另加8位数据
#define CH450_DIG3      0x1300		            // 数码管位3显示,需另加8位数据
#define CH450_DIG4      0x1400		            // 数码管位4显示,需另加8位数据
#define CH450_DIG5      0x1500					// 数码管位5显示,需另加8位数据
#define CH450_DIG6      0x1600					// 数码管位6显示,需另加8位数据
#define CH450_DIG7      0x1700		            // 数码管位7显示,需另加8位数据

/*
 * CH450 I2C 接口定义
 */
#define	CH450_I2C_ADDR1		0x40			    // CH450的地址部分
#define	CH450_I2C_MASK		0x3E			    // CH450的高字节命令掩码

/**
 * @brief 初始化CH450所使用的I2C总线
 * @param wai2c     - I2C句柄指针
 * @param scl_group - SCL引脚所在的GPIO组 (e.g., GPIOA, GPIOB)
 * @param scl_pin   - SCL引脚
 * @param sda_group - SDA引脚所在的GPIO组 (e.g., GPIOA, GPIOB)
 * @param sda_pin   - SDA引脚
 * @param delay_us  - I2C通信的位延迟，影响速度
 */
void CH450_Init(I2C_HandleDef *wai2c, uint8_t scl_group, uint32_t scl_pin, uint8_t sda_group, uint32_t sda_pin, uint8_t delay_us);


/**
 * @brief 向CH450写入一个命令（包括显示数据）
 * @param wai2c - I2C句柄指针
 * @param cmd   - 16位的CH450命令
 * @return uint8_t - 0表示成功，非0表示失败
 */
uint8_t CH450_Write_Cmd(I2C_HandleDef *wai2c, uint16_t cmd);


/**
 * @brief 从CH450读取一个按键码
 * @param wai2c     - I2C句柄指针
 * @param key_code  - 用于存储读取到的按键码的指针
 * @return uint8_t - 0表示成功，非0表示失败
 */
uint8_t CH450_Read_Key(I2C_HandleDef *wai2c, uint8_t *key_code);

void CH450_Interrupt_Init(void);

#endif // __CH450_DRIVER_H