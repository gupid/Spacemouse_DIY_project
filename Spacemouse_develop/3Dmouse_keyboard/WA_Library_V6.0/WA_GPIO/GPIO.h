#ifndef __GPIO_H
#define __GPIO_H

#include "main.h"


// ����һ������ָ��ָ��GPIOA,GPIOB�Ķ˿�����ģʽ���ú���
typedef void(*GPIO_MODE_Cfg) (uint32_t, GPIOModeTypeDef);

// ����һ��������GPIO�Ľṹ����GPIO�����źͶ˿�����ģʽ���ú���
typedef struct GPIO_HandleDef{
    uint8_t gpio;
    uint32_t gpio_pin;
    uint8_t gpio_mode;
    GPIO_MODE_Cfg GPIOMODECfg;
    uint8_t gpioit;               //�����жϵı�־
}GPIO_HandleDef;

extern GPIO_HandleDef waGPIOA;
extern GPIO_HandleDef waGPIOB;

//�궨���������A,B����ΪCH585ֻ����������
#define GPIOA 'A'
#define GPIOB 'B'

//�������������ʼ��Ĭ�ϵ�������״̬
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define GPIO_PIN_NULL 2

//GPIO��ʼ������
void WA_GPIO_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpiomode,uint8_t output);
//GPIO�ⲿ�жϳ�ʼ������
void WA_GPIO_IT_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpioedg);

//�����Ӻ��������ԡ������嵥�����д���
void WA_GPIO_MspInit(GPIO_HandleDef *defgpio,uint8_t output);
//�����Ӻ��������ԡ������嵥�����д���
void WA_GPIO_IT_MspInit(GPIO_HandleDef *defgpio);

//�ص�����
void WA_GPIOA_EventCallBack(uint32_t gpiopin);
void WA_GPIOB_EventCallBack(uint32_t gpiopin);

#endif
