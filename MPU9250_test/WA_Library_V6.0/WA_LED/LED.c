/*
    CH585评估板的LED有两个：
        PA0------低电平点亮——LED0
        PA1------低电平点亮——LED1
*/
#include "LED.h"

LED_HandleDef waLED0;
LED_HandleDef waLED1;


//函数声明
void WA_LED_GPIO_MspInit(LED_HandleDef *defled);


void WA_LED_GPIO_Init(LED_HandleDef *defled)
{
    if(defled == &waLED0)    // 针对 LED0 的初始化代码
    {
        defled->led = LED0;
        defled->led_pin = GPIO_Pin_0;
        defled->led_mode = GPIO_ModeOut_PP_20mA;
        defled->GPIOMODEInit = GPIOA_ModeCfg;
    }

    if(defled == &waLED1)    // 针对 LED1 的初始化代码
    {
        defled->led = LED1;
        defled->led_pin = GPIO_Pin_1;
        defled->led_mode = GPIO_ModeOut_PP_20mA;
        defled->GPIOMODEInit = GPIOA_ModeCfg;
    }
    
    //可自行添加初始化代码，格式同上：
    //begin

  

    //end

    WA_LED_GPIO_MspInit(defled);

}

//“厨子函数”：对“配置清单”进行处理
void WA_LED_GPIO_MspInit(LED_HandleDef *defled)
{
    defled->GPIOMODEInit(defled->led_pin,defled->led_mode);
}

/*
    LED主功能函数:
        BOOL：1：输出高电平；0：输出低电平。
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

    //可自行添加初始化代码，格式同上：
    //begin



    //end
}

