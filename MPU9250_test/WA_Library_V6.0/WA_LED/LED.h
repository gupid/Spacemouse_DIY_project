#ifndef __LED_H
#define __LED_H

#include "main.h"


// ����һ������ָ��ָ��GPIOA,GPIOB�Ķ˿�����ģʽ���ú���
typedef void(*GPIO_MODE_Init) (uint32_t, GPIOModeTypeDef);

// ����һ�����������LED�Ľṹ����LED��ID�Ͷ˿�����ģʽ���ú���
typedef struct LED_HandleDef{
    uint8_t led;
    uint32_t led_pin;
    int led_mode;
    GPIO_MODE_Init GPIOMODEInit;
}LED_HandleDef;

//����������LED�ṹ��
extern LED_HandleDef waLED0;
extern LED_HandleDef waLED1;

//�궨��LED��ID�ţ�������ID��ͳһ����
#define LED0 0
#define LED1 1


//����ת����ƽ�Ĵ����ź�
#define Toggle 254

//LED��ʼ�����ź���
void WA_LED_GPIO_Init(LED_HandleDef *defled);

//LED�����ܺ���
void WA_LED_Function(LED_HandleDef *defled,uint8_t BOOL);

#endif
