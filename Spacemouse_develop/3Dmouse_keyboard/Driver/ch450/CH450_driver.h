#ifndef __CH450_DRIVER_H
#define __CH450_DRIVER_H

#include "I2C.h"

/*
 * CH450 �����붨��
 */
#define CH450_SYSOFF    0x0400					// �ر���ʾ���رռ���
#define CH450_SYSON1    0x0401					// ������ʾ
#define CH450_SYSON2    0x0403					// ������ʾ������
#define CH450_GET_KEY	0x0700					// ��ȡ�������� (���ڼ���ʵ�ʷ��͵�����)
#define CH450_CMD_READ_KEY  0x4F                // ʵ�ʷ��͵Ķ�ȡ���������ֽ�

#define CH450_DIG2      0x1200		            // �����λ2��ʾ,�����8λ����
#define CH450_DIG3      0x1300		            // �����λ3��ʾ,�����8λ����
#define CH450_DIG4      0x1400		            // �����λ4��ʾ,�����8λ����
#define CH450_DIG5      0x1500					// �����λ5��ʾ,�����8λ����
#define CH450_DIG6      0x1600					// �����λ6��ʾ,�����8λ����
#define CH450_DIG7      0x1700		            // �����λ7��ʾ,�����8λ����

/*
 * CH450 I2C �ӿڶ���
 */
#define	CH450_I2C_ADDR1		0x40			    // CH450�ĵ�ַ����
#define	CH450_I2C_MASK		0x3E			    // CH450�ĸ��ֽ���������

/**
 * @brief ��ʼ��CH450��ʹ�õ�I2C����
 * @param wai2c     - I2C���ָ��
 * @param scl_group - SCL�������ڵ�GPIO�� (e.g., GPIOA, GPIOB)
 * @param scl_pin   - SCL����
 * @param sda_group - SDA�������ڵ�GPIO�� (e.g., GPIOA, GPIOB)
 * @param sda_pin   - SDA����
 * @param delay_us  - I2Cͨ�ŵ�λ�ӳ٣�Ӱ���ٶ�
 */
void CH450_Init(I2C_HandleDef *wai2c, uint8_t scl_group, uint32_t scl_pin, uint8_t sda_group, uint32_t sda_pin, uint8_t delay_us);


/**
 * @brief ��CH450д��һ�����������ʾ���ݣ�
 * @param wai2c - I2C���ָ��
 * @param cmd   - 16λ��CH450����
 * @return uint8_t - 0��ʾ�ɹ�����0��ʾʧ��
 */
uint8_t CH450_Write_Cmd(I2C_HandleDef *wai2c, uint16_t cmd);


/**
 * @brief ��CH450��ȡһ��������
 * @param wai2c     - I2C���ָ��
 * @param key_code  - ���ڴ洢��ȡ���İ������ָ��
 * @return uint8_t - 0��ʾ�ɹ�����0��ʾʧ��
 */
uint8_t CH450_Read_Key(I2C_HandleDef *wai2c, uint8_t *key_code);

void CH450_Interrupt_Init(void);

#endif // __CH450_DRIVER_H