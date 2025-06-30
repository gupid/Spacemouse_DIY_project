/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_mpu9250_interface_template.c
 * @brief     driver mpu9250 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2022-08-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2022/08/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_mpu9250_interface.h"
#include "main.h"
/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t mpu9250_interface_iic_init(void)
{
    WA_I2C_Init(&wai2c0,GPIOB,GPIO_Pin_5,GPIOB,GPIO_Pin_4,1);
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t mpu9250_interface_iic_deinit(void)
{
    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 * - 0 success
 * - 1 read failed
 * @note       none
 */
uint8_t mpu9250_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
     // 将驱动传入的8位地址(如0xD0)右移一位，得到7位地址(0x68)
    uint8_t dev_addr_7bit = addr >> 1;

    // 调用底层库，传入正确的7位地址
    if (WA_Read_Len(&wai2c0, dev_addr_7bit, reg, (uint8_t)len, buf) == 0)
    {
        return 0; // 成功
    }
    else
    {
        return 1; // 失败
    }
}

/**
 * @brief      (新版) 接口I2C读函数 - 7位地址版本
 * @param[in]  addr iic 设备的7位地址 (例如 0x68)
 * @param[in]  reg iic 寄存器地址
 * @param[out] *buf 指向数据缓冲区的指针
 * @param[in]  len 数据缓冲区的长度
 * @return     状态码
 * - 0 成功
 * - 1 读取失败
 * @note       此版本直接使用传入的7位地址，不进行转换。
 */
uint8_t mpu9250_interface_iic_read_7bit_addr(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // 直接调用底层库，传入未修改的7位地址
    if (WA_Read_Len(&wai2c0, addr, reg, (uint8_t)len, buf) == 0)
    {
        return 0; // 成功
    }
    else
    {
        return 1; // 失败
    }
}

/**
 * @brief     (新版) 接口I2C写函数 - 7位地址版本
 * @param[in] addr iic 设备的7位地址 (例如 0x68)
 * @param[in] reg iic 寄存器地址
 * @param[in] *buf 指向数据缓冲区的指针
 * @param[in] len 数据缓冲区的长度
 * @return    状态码
 * - 0 成功
 * - 1 写入失败
 * @note      此版本直接使用传入的7位地址，不进行转换。
 */
uint8_t mpu9250_interface_iic_write_7bit_addr(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // 直接调用底层库，传入未修改的7位地址
    if (WA_Write_Len(&wai2c0, addr, reg, (uint8_t)len, buf) == 0)
    {
        return 0; // 成功
    }
    else
    {
        return 1; // 失败
    }
}

/**
 * @brief      interface iic bus write
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[in]  *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 * - 0 success
 * - 1 write failed
 * @note       none
 */
uint8_t mpu9250_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // 将驱动传入的8位地址(如0xD0)右移一位，得到7位地址(0x68)
    uint8_t dev_addr_7bit = addr >> 1;

    // 调用底层库，传入正确的7位地址
    if (WA_Write_Len(&wai2c0, dev_addr_7bit, reg, (uint8_t)len, buf) == 0)
    {
        return 0; // 成功
    }
    else
    {
        return 1; // 失败
    }
}

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t mpu9250_interface_spi_init(void)
{
    WA_SPI0_Hardware_Init(Master,Pin_Disable);
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t mpu9250_interface_spi_deinit(void)
{   
    R8_SPI0_CTRL_MOD = RB_SPI_ALL_CLEAR;
    R8_SPI0_CTRL_CFG &= ~(RB_SPI_AUTO_IF | RB_SPI_DMA_ENABLE);
    return 0;
}

/**
 * @brief      interface spi bus read
 * @param[in]  reg register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t mpu9250_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
     // 准备要发送的读命令字节 (寄存器地址的最高位置1)
    uint8_t read_cmd = reg | 0x80;
    SPI0_CSS_L(); 
    SPI0_MasterSendByte(read_cmd);
    SPI0_MasterRecv(buf, len);
    SPI0_CSS_H(); 

    return 0; 
}

/**
 * @brief     interface spi bus write
 * @param[in] reg register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t mpu9250_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t write_cmd = reg & 0x7F;
    SPI0_CSS_L(); 
    SPI0_MasterSendByte(write_cmd);
    SPI0_MasterTrans(buf,len);
    SPI0_CSS_H(); 

    return 0; 
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void mpu9250_interface_delay_ms(uint32_t ms)
{
    DelayMs(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void mpu9250_interface_debug_print(const char *const fmt, ...)
{
    char str[128];
    va_list args;  
    va_start(args, fmt);
    vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);
    PRINT("%s", str); 
}

/**
 * @brief     interface receive callback
 * @param[in] type irq type
 * @note      none
 */
void mpu9250_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case MPU9250_INTERRUPT_MOTION :
        {
            mpu9250_interface_debug_print("mpu9250: irq motion.\n");
            
            break;
        }
        case MPU9250_INTERRUPT_FIFO_OVERFLOW :
        {
            mpu9250_interface_debug_print("mpu9250: irq fifo overflow.\n");
            
            break;
        }
        case MPU9250_INTERRUPT_FSYNC_INT :
        {
            mpu9250_interface_debug_print("mpu9250: irq fsync int.\n");
            
            break;
        }
        case MPU9250_INTERRUPT_DMP :
        {
            mpu9250_interface_debug_print("mpu9250: irq dmp\n");
            
            break;
        }
        case MPU9250_INTERRUPT_DATA_READY :
        {
            mpu9250_interface_debug_print("mpu9250: irq data ready\n");
            
            break;
        }
        default :
        {
            mpu9250_interface_debug_print("mpu9250: irq unknown code.\n");
            
            break;
        }
    }
}

/**
 * @brief     interface dmp tap callback
 * @param[in] count tap count
 * @param[in] direction tap direction
 * @note      none
 */
void mpu9250_interface_dmp_tap_callback(uint8_t count, uint8_t direction)
{
    switch (direction)
    {
        case MPU9250_DMP_TAP_X_UP :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq x up with %d.\n", count);
            
            break;
        }
        case MPU9250_DMP_TAP_X_DOWN :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq x down with %d.\n", count);
            
            break;
        }
        case MPU9250_DMP_TAP_Y_UP :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq y up with %d.\n", count);
            
            break;
        }
        case MPU9250_DMP_TAP_Y_DOWN :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq y down with %d.\n", count);
            
            break;
        }
        case MPU9250_DMP_TAP_Z_UP :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq z up with %d.\n", count);
            
            break;
        }
        case MPU9250_DMP_TAP_Z_DOWN :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq z down with %d.\n", count);
            
            break;
        }
        default :
        {
            mpu9250_interface_debug_print("mpu9250: tap irq unknown code.\n");
            
            break;
        }
    }
}

/**
 * @brief     interface dmp orient callback
 * @param[in] orientation dmp orientation
 * @note      none
 */
void mpu9250_interface_dmp_orient_callback(uint8_t orientation)
{
    switch (orientation)
    {
        case MPU9250_DMP_ORIENT_PORTRAIT :
        {
            mpu9250_interface_debug_print("mpu9250: orient irq portrait.\n");
            
            break;
        }
        case MPU9250_DMP_ORIENT_LANDSCAPE :
        {
            mpu9250_interface_debug_print("mpu9250: orient irq landscape.\n");
            
            break;
        }
        case MPU9250_DMP_ORIENT_REVERSE_PORTRAIT :
        {
            mpu9250_interface_debug_print("mpu9250: orient irq reverse portrait.\n");
            
            break;
        }
        case MPU9250_DMP_ORIENT_REVERSE_LANDSCAPE :
        {
            mpu9250_interface_debug_print("mpu9250: orient irq reverse landscape.\n");
            
            break;
        }
        default :
        {
            mpu9250_interface_debug_print("mpu9250: orient irq unknown code.\n");
            
            break;
        }
    }
}
