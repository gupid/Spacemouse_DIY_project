#ifndef __LED_H
#define __LED_H

#include "main.h"


// 定义一个函数指针指向GPIOA,GPIOB的端口引脚模式配置函数
typedef void(*GPIO_MODE_Init) (uint32_t, GPIOModeTypeDef);

// 定义一个评估板板载LED的结构体存放LED的ID和端口引脚模式配置函数
typedef struct LED_HandleDef{
    uint8_t led;
    uint32_t led_pin;
    int led_mode;
    GPIO_MODE_Init GPIOMODEInit;
}LED_HandleDef;

//定义声明的LED结构体
extern LED_HandleDef waLED0;
extern LED_HandleDef waLED1;

//宏定义LED的ID号，后续对ID号统一操作
#define LED0 0
#define LED1 1


//定义转换电平的触发信号
#define Toggle 254

//LED初始化引脚函数
void WA_LED_GPIO_Init(LED_HandleDef *defled);

//LED主功能函数
void WA_LED_Function(LED_HandleDef *defled,uint8_t BOOL);

#endif
