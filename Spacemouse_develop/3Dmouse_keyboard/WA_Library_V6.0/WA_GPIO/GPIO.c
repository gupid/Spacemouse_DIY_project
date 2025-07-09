#include "GPIO.h"

GPIO_HandleDef waGPIOA;
GPIO_HandleDef waGPIOB;


//GPIO��ʼ������
void WA_GPIO_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpiomode,uint8_t output)
{
    if(defgpio == &waGPIOA)
    {
        defgpio->gpio = GPIOA;
        defgpio->gpio_pin = gpiopin;
        defgpio->gpio_mode = gpiomode;
        defgpio->GPIOMODECfg = GPIOA_ModeCfg;
    }

     if(defgpio == &waGPIOB)
    {
        defgpio->gpio = GPIOB;
        defgpio->gpio_pin = gpiopin;
        defgpio->gpio_mode = gpiomode;
        defgpio->GPIOMODECfg = GPIOB_ModeCfg;
    }

    //�����Ӻ��������ԡ������嵥�����д���
    WA_GPIO_MspInit(defgpio,output);

}

//GPIO�ⲿ�жϳ�ʼ������
void WA_GPIO_IT_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpioedg)
{
    WA_GPIO_Init(defgpio,gpiopin,GPIO_ModeIN_PU,GPIO_PIN_NULL);

    if(defgpio == &waGPIOA)
        defgpio->gpioit = gpioedg;

     if(defgpio == &waGPIOB)
        defgpio->gpioit = gpioedg;

    WA_GPIO_IT_MspInit(defgpio);
    
}



//�����Ӻ��������ԡ������嵥�����д���
void WA_GPIO_MspInit(GPIO_HandleDef *defgpio,uint8_t output)
{
    if(defgpio == &waGPIOA)
    {
        defgpio->GPIOMODECfg(defgpio->gpio_pin,defgpio->gpio_mode);
        if(defgpio->gpio_mode == GPIO_ModeOut_PP_5mA || defgpio->gpio_mode == GPIO_ModeOut_PP_20mA)
        {
            if(output == GPIO_PIN_SET)
                GPIOA_SetBits(defgpio->gpio_pin);
            if(output == GPIO_PIN_RESET)
                GPIOA_ResetBits(defgpio->gpio_pin);
        }
    }

    if(defgpio == &waGPIOB)
    {
        defgpio->GPIOMODECfg(defgpio->gpio_pin,defgpio->gpio_mode);
        if(defgpio->gpio_mode == GPIO_ModeOut_PP_5mA || defgpio->gpio_mode == GPIO_ModeOut_PP_20mA)
        {
            if(output == GPIO_PIN_SET)
                GPIOB_SetBits(defgpio->gpio_pin);
            if(output == GPIO_PIN_RESET)
                GPIOB_ResetBits(defgpio->gpio_pin);
        }
    }
}

//�����Ӻ��������ԡ������嵥�����д���
void WA_GPIO_IT_MspInit(GPIO_HandleDef *defgpio)
{
    if(defgpio == &waGPIOA)
    {
        GPIOA_ITModeCfg(defgpio->gpio_pin,defgpio->gpioit);
        PFIC_EnableIRQ(GPIO_A_IRQn);
    }

    if(defgpio == &waGPIOB)
    {
        GPIOB_ITModeCfg(defgpio->gpio_pin,defgpio->gpioit);
        PFIC_EnableIRQ(GPIO_B_IRQn);
    }
}


__weak_symbol void WA_GPIOA_EventCallBack(uint32_t gpiopin)
{
    /*�û��Զ�������*/
    /* ��ʼ */


    /* ���� */
}

__weak_symbol void WA_GPIOB_EventCallBack(uint32_t gpiopin)
{
    /*�û��Զ�������*/
    /* ��ʼ */


    /* ���� */
}
