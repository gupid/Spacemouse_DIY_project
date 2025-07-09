#ifndef __I2C_H
#define __I2C_H

#include "main.h"


// I2C句柄结构体，保存各实例引脚配置
typedef struct {
    uint8_t scl_group;    // SCL引脚组
    uint32_t scl_pin;     // SCL引脚号
    uint8_t sda_group;    // SDA引脚组
    uint32_t sda_pin;     // SDA引脚号

    uint8_t delayus;
} I2C_HandleDef;

// 外部声明4个I2C实例
extern I2C_HandleDef wai2c0;
extern I2C_HandleDef wai2c1;
extern I2C_HandleDef wai2c2;
extern I2C_HandleDef wai2c3;

#define IIC_WR	0		/* 写控制bit */
#define IIC_RD	1		/* 读控制bit */

#define MPU_IIC_Start			IIC_Start
#define MPU_IIC_Stop			IIC_Stop
#define MPU_IIC_Send_Byte		IIC_Send_Byte
#define MPU_IIC_Read_Byte		IIC_Read_Byte
#define MPU_IIC_Wait_Ack		IIC_Wait_Ack

#define WA_I2C_OK                   0x00 // 操作成功
#define WA_I2C_ERR_INVALID_PARAMS   0xFF // 输入参数无效
#define WA_I2C_ERR_ADDR1_NACK       0x01 // 错误：阶段1发送从机地址+写后，从机NACK
#define WA_I2C_ERR_REG_NACK         0x02 // 错误：发送寄存器地址后，从机NACK
#define WA_I2C_ERR_ADDR2_NACK       0x03 // 错误：阶段2发送从机地址+读后，从机NACK（在t_MEAS延时后）

void IIC_Start(I2C_HandleDef *wai2c);
void IIC_Stop(I2C_HandleDef *wai2c);
void IIC_Send_Byte(I2C_HandleDef *wai2c,uint8_t _ucByte);
uint8_t IIC_Read_Byte(I2C_HandleDef *wai2c,uint8_t ack);
uint8_t IIC_Wait_Ack(I2C_HandleDef *wai2c);
void IIC_Ack(I2C_HandleDef *wai2c);
void IIC_NAck(I2C_HandleDef *wai2c);
void IIC_SCL_1(I2C_HandleDef *wai2c);
void IIC_SCL_0(I2C_HandleDef *wai2c);
void IIC_SDA_1(I2C_HandleDef *wai2c);
void IIC_SDA_0(I2C_HandleDef *wai2c);

void WA_I2C_Init(I2C_HandleDef *wai2c,uint8_t sclgroup,uint32_t sclpin,uint8_t sdagroup,uint32_t sdapin,uint8_t delayus);
uint8_t WA_Write_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf );//IIC连续写
uint8_t WA_Read_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf ); //IIC连续读
uint8_t TLV493D_Read_Bytes(
    I2C_HandleDef *wai2c,
    uint8_t slave_7bit_addr,
    uint8_t initial_bytes_to_skip,
    uint8_t num_bytes_to_read,
    uint8_t *data_buffer
);
#endif // __I2C_H
