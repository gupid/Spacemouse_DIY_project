/* 官方底层管脚映射有问题：TIM0的默认引脚是PA9，但是映射不到PB23，因为PB23默认做板载复位按键*/
/* 官方底层管脚映射有问题：TIM3的默认引脚是PB22，可以映射到PA2，他那个文件的注释是错的*/
#ifndef __TIM_H
#define __TIM_H

#include "main.h"

#define WA_TIMER 152
#define WA_COUNT 888 

/*  RB_PIN_TMR3   -  TMR3:  PB22 ->  PA2
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */
#define Tmr0_PA9  9
#define Tmr0_PB13 8
#define Tmr1_PA10  7
#define Tmr1_PB10  6
#define Tmr2_PA11  5
#define Tmr2_PB11  4
#define Tmr3_PA2  3
#define Tmr3_PB22  2

//引脚重映射使能位
#define Pin_Enable 1
#define Pin_Disable 0
#define null 2

//定义一个结构体存放定时器参数
/* 
mode:定时器模式 || 计数器模式
Frequency：频率Hz（定时器模式）
print:管脚映射使能
count_way:计数方式
count：计数触发值
*/
typedef struct{
    uint16_t mode;
    uint32_t Frequency;

    uint8_t print;
    uint8_t count_way;
    uint32_t count;
}TIM_HandleDef;

//只有四个定时器资源，直接全定义了
extern TIM_HandleDef watime0;
extern TIM_HandleDef watime1;
extern TIM_HandleDef watime2;
extern TIM_HandleDef watime3;

//初始化定时器||计数器
/* 
watime:地址
mode:模式   WA_TIMER    or  WA_COUNT
count_way：计数方式
count：计数值
*/
void WA_TIM_IT_Init(TIM_HandleDef *watime,uint16_t mode,uint8_t yingcshe,uint32_t fre,uint8_t count_way,uint32_t count);

/* 定义周期中断的回调函数 */
void WA_TIM_PeriodCallBack(TIM_HandleDef *watime);
/* 定义计数中断的回调函数 */
void WA_TIM_CountCallBack(TIM_HandleDef *watime);

#endif
