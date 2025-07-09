#include "CH450_driver.h"

// CH450的通信协议是专有的2线协议，并非标准I2C读写。
// 这里的函数将严格按照CH450的数据手册协议，调用您提供的I2C底层函数来模拟时序。

void CH450_Init(I2C_HandleDef *wai2c, uint8_t scl_group, uint32_t scl_pin, uint8_t sda_group, uint32_t sda_pin, uint8_t delay_us)
{
    // 调用您提供的I2C初始化函数来配置GPIO
    WA_I2C_Init(wai2c, scl_group, scl_pin, sda_group, sda_pin, delay_us);
}

uint8_t CH450_Write_Cmd(I2C_HandleDef *wai2c, uint16_t cmd)
{
    uint8_t byte1, byte2;

    // 根据CH450协议组合第一个字节 (地址 + 命令高位)
    byte1 = ((uint8_t)(cmd >> 7) & CH450_I2C_MASK) | CH450_I2C_ADDR1;
    // 第二个字节是命令低位
    byte2 = (uint8_t)cmd;

    // 开始通信
    MPU_IIC_Start(wai2c);

    // 发送第一个字节
    MPU_IIC_Send_Byte(wai2c, byte1);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // 发送停止信号
        return 1; // 错误：第一次ACK失败
    }

    // 发送第二个字节
    MPU_IIC_Send_Byte(wai2c, byte2);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // 发送停止信号
        return 2; // 错误：第二次ACK失败
    }

    // 结束通信
    MPU_IIC_Stop(wai2c);

    return 0; // 成功
}


uint8_t CH450_Read_Key(I2C_HandleDef *wai2c, uint8_t *key_code)
{
    if (key_code == NULL)
    {
        return 3; // 错误：传入了空指针
    }

    // 开始通信
    MPU_IIC_Start(wai2c);

    // 发送读取按键的命令字节
    MPU_IIC_Send_Byte(wai2c, CH450_CMD_READ_KEY);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // 发送停止信号
        return 1; // 错误：ACK失败
    }

    // 从总线读取一个字节的按键码
    // CH450数据手册规定，读取最后一个（也是唯一一个）字节后，主机应回送NACK
    *key_code = MPU_IIC_Read_Byte(wai2c, 0); // 传入0表示发送NACK

    // 结束通信
    MPU_IIC_Stop(wai2c);

    return 0; // 成功
}

void CH450_Interrupt_Init(void)
{
    // 1. 配置GPIOB_Pin_7为上拉输入
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_PU);

    // 2. 配置引脚为下降沿触发中断
    GPIOB_ITModeCfg(GPIO_Pin_7, GPIO_ITMode_FallEdge);

    // 3. 使能GPIOB端口的中断
    PFIC_EnableIRQ(GPIO_B_IRQn);
}