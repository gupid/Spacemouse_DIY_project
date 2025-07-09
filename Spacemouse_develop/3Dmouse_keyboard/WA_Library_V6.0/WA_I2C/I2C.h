#ifndef __I2C_H
#define __I2C_H

#include "main.h"


// I2C����ṹ�壬�����ʵ����������
typedef struct {
    uint8_t scl_group;    // SCL������
    uint32_t scl_pin;     // SCL���ź�
    uint8_t sda_group;    // SDA������
    uint32_t sda_pin;     // SDA���ź�

    uint8_t delayus;
} I2C_HandleDef;

// �ⲿ����4��I2Cʵ��
extern I2C_HandleDef wai2c0;
extern I2C_HandleDef wai2c1;
extern I2C_HandleDef wai2c2;
extern I2C_HandleDef wai2c3;

#define IIC_WR	0		/* д����bit */
#define IIC_RD	1		/* ������bit */

#define MPU_IIC_Start			IIC_Start
#define MPU_IIC_Stop			IIC_Stop
#define MPU_IIC_Send_Byte		IIC_Send_Byte
#define MPU_IIC_Read_Byte		IIC_Read_Byte
#define MPU_IIC_Wait_Ack		IIC_Wait_Ack

#define WA_I2C_OK                   0x00 // �����ɹ�
#define WA_I2C_ERR_INVALID_PARAMS   0xFF // ���������Ч
#define WA_I2C_ERR_ADDR1_NACK       0x01 // ���󣺽׶�1���ʹӻ���ַ+д�󣬴ӻ�NACK
#define WA_I2C_ERR_REG_NACK         0x02 // ���󣺷��ͼĴ�����ַ�󣬴ӻ�NACK
#define WA_I2C_ERR_ADDR2_NACK       0x03 // ���󣺽׶�2���ʹӻ���ַ+���󣬴ӻ�NACK����t_MEAS��ʱ��

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
uint8_t WA_Write_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf );//IIC����д
uint8_t WA_Read_Len(I2C_HandleDef *wai2c,uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf ); //IIC������
uint8_t TLV493D_Read_Bytes(
    I2C_HandleDef *wai2c,
    uint8_t slave_7bit_addr,
    uint8_t initial_bytes_to_skip,
    uint8_t num_bytes_to_read,
    uint8_t *data_buffer
);
#endif // __I2C_H
