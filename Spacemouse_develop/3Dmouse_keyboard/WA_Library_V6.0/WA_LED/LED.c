/*
    CH585�������LED��������
        PA0------�͵�ƽ��������LED0
        PA1------�͵�ƽ��������LED1
*/
#include "LED.h"

LED_HandleDef waLED0;
LED_HandleDef waLED1;


//��������
void WA_LED_GPIO_MspInit(LED_HandleDef *defled);


void WA_LED_GPIO_Init(LED_HandleDef *defled)
{
    if(defled == &waLED0)    // ��� LED0 �ĳ�ʼ������
    {
        defled->led = LED0;
        defled->led_pin = GPIO_Pin_0;
        defled->led_mode = GPIO_ModeOut_PP_20mA;
        defled->GPIOMODEInit = GPIOA_ModeCfg;
    }

    if(defled == &waLED1)    // ��� LED1 �ĳ�ʼ������
    {
        defled->led = LED1;
        defled->led_pin = GPIO_Pin_1;
        defled->led_mode = GPIO_ModeOut_PP_20mA;
        defled->GPIOMODEInit = GPIOA_ModeCfg;
    }
    
    //��������ӳ�ʼ�����룬��ʽͬ�ϣ�
    //begin

  

    //end

    WA_LED_GPIO_MspInit(defled);

}

//�����Ӻ��������ԡ������嵥�����д���
void WA_LED_GPIO_MspInit(LED_HandleDef *defled)
{
    defled->GPIOMODEInit(defled->led_pin,defled->led_mode);
}

/*
    LED�����ܺ���:
        BOOL��1������ߵ�ƽ��0������͵�ƽ��
*/
void WA_LED_Function(LED_HandleDef *defled,uint8_t BOOL)
{
    if(defled == &waLED0)    
    {
        if(BOOL == 0)
            GPIOA_ResetBits(defled->led_pin);
        else if(BOOL == Toggle)
            GPIOA_InverseBits(defled->led_pin);
        else
            GPIOA_SetBits(defled->led_pin);
    }

    if(defled == &waLED1)    
    {
        if(BOOL == 0)
            GPIOA_ResetBits(defled->led_pin);
        else if(BOOL == Toggle)
            GPIOA_InverseBits(defled->led_pin);
        else
            GPIOA_SetBits(defled->led_pin);
    }

    //��������ӳ�ʼ�����룬��ʽͬ�ϣ�
    //begin



    //end
}

