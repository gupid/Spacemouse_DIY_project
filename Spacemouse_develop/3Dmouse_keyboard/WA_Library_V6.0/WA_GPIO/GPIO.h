#ifndef __GPIO_H
#define __GPIO_H

#include "main.h"


// 定义一个函数指针指向GPIOA,GPIOB的端口引脚模式配置函数
typedef void(*GPIO_MODE_Cfg) (uint32_t, GPIOModeTypeDef);

// 定义一个评估板GPIO的结构体存放GPIO的引脚和端口引脚模式配置函数
typedef struct GPIO_HandleDef{
    uint8_t gpio;
    uint32_t gpio_pin;
    uint8_t gpio_mode;
    GPIO_MODE_Cfg GPIOMODECfg;
    uint8_t gpioit;               //触发中断的标志
}GPIO_HandleDef;

extern GPIO_HandleDef waGPIOA;
extern GPIO_HandleDef waGPIOB;

//宏定义两个组别A,B，因为CH585只有两组引脚
#define GPIOA 'A'
#define GPIOB 'B'

//定义推挽输出初始化默认的上下拉状态
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define GPIO_PIN_NULL 2

//GPIO初始化函数
void WA_GPIO_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpiomode,uint8_t output);
//GPIO外部中断初始化函数
void WA_GPIO_IT_Init(GPIO_HandleDef *defgpio,uint32_t gpiopin,uint8_t gpioedg);

//“厨子函数”：对“配置清单”进行处理
void WA_GPIO_MspInit(GPIO_HandleDef *defgpio,uint8_t output);
//“厨子函数”：对“配置清单”进行处理
void WA_GPIO_IT_MspInit(GPIO_HandleDef *defgpio);

//回调函数
void WA_GPIOA_EventCallBack(uint32_t gpiopin);
void WA_GPIOB_EventCallBack(uint32_t gpiopin);

#endif
