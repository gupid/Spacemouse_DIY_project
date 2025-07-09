#include "CH450_driver.h"

// CH450��ͨ��Э����ר�е�2��Э�飬���Ǳ�׼I2C��д��
// ����ĺ������ϸ���CH450�������ֲ�Э�飬�������ṩ��I2C�ײ㺯����ģ��ʱ��

void CH450_Init(I2C_HandleDef *wai2c, uint8_t scl_group, uint32_t scl_pin, uint8_t sda_group, uint32_t sda_pin, uint8_t delay_us)
{
    // �������ṩ��I2C��ʼ������������GPIO
    WA_I2C_Init(wai2c, scl_group, scl_pin, sda_group, sda_pin, delay_us);
}

uint8_t CH450_Write_Cmd(I2C_HandleDef *wai2c, uint16_t cmd)
{
    uint8_t byte1, byte2;

    // ����CH450Э����ϵ�һ���ֽ� (��ַ + �����λ)
    byte1 = ((uint8_t)(cmd >> 7) & CH450_I2C_MASK) | CH450_I2C_ADDR1;
    // �ڶ����ֽ��������λ
    byte2 = (uint8_t)cmd;

    // ��ʼͨ��
    MPU_IIC_Start(wai2c);

    // ���͵�һ���ֽ�
    MPU_IIC_Send_Byte(wai2c, byte1);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // ����ֹͣ�ź�
        return 1; // ���󣺵�һ��ACKʧ��
    }

    // ���͵ڶ����ֽ�
    MPU_IIC_Send_Byte(wai2c, byte2);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // ����ֹͣ�ź�
        return 2; // ���󣺵ڶ���ACKʧ��
    }

    // ����ͨ��
    MPU_IIC_Stop(wai2c);

    return 0; // �ɹ�
}


uint8_t CH450_Read_Key(I2C_HandleDef *wai2c, uint8_t *key_code)
{
    if (key_code == NULL)
    {
        return 3; // ���󣺴����˿�ָ��
    }

    // ��ʼͨ��
    MPU_IIC_Start(wai2c);

    // ���Ͷ�ȡ�����������ֽ�
    MPU_IIC_Send_Byte(wai2c, CH450_CMD_READ_KEY);
    if (MPU_IIC_Wait_Ack(wai2c) != 0)
    {
        MPU_IIC_Stop(wai2c); // ����ֹͣ�ź�
        return 1; // ����ACKʧ��
    }

    // �����߶�ȡһ���ֽڵİ�����
    // CH450�����ֲ�涨����ȡ���һ����Ҳ��Ψһһ�����ֽں�����Ӧ����NACK
    *key_code = MPU_IIC_Read_Byte(wai2c, 0); // ����0��ʾ����NACK

    // ����ͨ��
    MPU_IIC_Stop(wai2c);

    return 0; // �ɹ�
}

void CH450_Interrupt_Init(void)
{
    // 1. ����GPIOB_Pin_7Ϊ��������
    GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_PU);

    // 2. ��������Ϊ�½��ش����ж�
    GPIOB_ITModeCfg(GPIO_Pin_7, GPIO_ITMode_FallEdge);

    // 3. ʹ��GPIOB�˿ڵ��ж�
    PFIC_EnableIRQ(GPIO_B_IRQn);
}