/* 官方底层管脚映射有问题：TIM0的默认引脚是PA9，但是映射不到PB23,因为PB23默认做板载复位按键*/
/* 官方底层管脚映射有问题：TIM3的默认引脚是PB22，可以映射到PA2，他那个文件的注释是错的*/
#ifndef __PWM_H
#define __PWM_H

#include "main.h"

#define  g_1us  (FREQ_SYS/1000000)


/*  RB_PIN_TMR3   -  TMR3:  PA2 ->  PB22
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */
//定义一个结构体存放定时器的PWM参数
/* 
print:管脚映射使能
function:有效电平
times:有效输出重复次数
period:周期（us）
pwm:占空比
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t times;
    uint16_t period;
    uint16_t pwm;
}PWM_Tim_HandleDef;

//定义一个结构体存放低级PWM参数
/* 
 // 默认PA12 - PWM4--->映射PA6
 // 默认PA13 - PWM5--->映射PA7
 // 默认PB0 - PWM6**************不可映射
 // 默认PB4 - PWM7--->映射PB1
 // 默认PB6 - PWM8--->映射PB2
 // 默认PB7 - PWM9--->映射PB3
 // 默认PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 此脚复用为外部复位脚，需要关闭此功能才能开启PWM功能
print:管脚映射使能
function:有效电平
timfre:基准时钟 = timfre/系统时钟主频62400000  (单位s)
period:周期（us）
pwm:占空比
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t timfre;
    uint16_t period;
    uint16_t pwm;
}PWMX_LOW_HandleDef;

//定义一个结构体存放高级PWM参数
/* 
 // 默认PA12 - PWM4--->映射PA6
 // 默认PA13 - PWM5--->映射PA7
 // 默认PB0 - PWM6**************不可映射
 // 默认PB4 - PWM7--->映射PB1
 // 默认PB6 - PWM8--->映射PB2
 // 默认PB7 - PWM9--->映射PB3
 // 默认PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 此脚复用为外部复位脚，需要关闭此功能才能开启PWM功能
print:管脚映射使能
function:有效电平
timfre:基准时钟 = timfre/系统时钟主频62400000  (单位s)
period:周期（us）
pwm:占空比
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t timfre;
    uint16_t period;
    uint16_t pwm;
}PWMX_High_HandleDef;

//四个定时器的引脚pwm
extern PWM_Tim_HandleDef wapwmt0;
extern PWM_Tim_HandleDef wapwmt1;
extern PWM_Tim_HandleDef wapwmt2;
extern PWM_Tim_HandleDef wapwmt3;

//定义8个低级PWM
extern PWMX_LOW_HandleDef wapwmlow4;
extern PWMX_LOW_HandleDef wapwmlow5;
extern PWMX_LOW_HandleDef wapwmlow6;
extern PWMX_LOW_HandleDef wapwmlow7;
extern PWMX_LOW_HandleDef wapwmlow8;
extern PWMX_LOW_HandleDef wapwmlow9;
extern PWMX_LOW_HandleDef wapwmlow10;
extern PWMX_LOW_HandleDef wapwmlow11;

//定义6个高级PWM
extern PWMX_High_HandleDef wapwmhigh4;
extern PWMX_High_HandleDef wapwmhigh5;
extern PWMX_High_HandleDef wapwmhigh6;
extern PWMX_High_HandleDef wapwmhigh7;
extern PWMX_High_HandleDef wapwmhigh8;
extern PWMX_High_HandleDef wapwmhigh9;

/*
    定时器引脚的PWM：
    pwmt：地址
    print:管脚映射使能
    function:有效电平
    times:有效输出重复次数
    period:周期（us）
    pwm:占空比
*/
void WA_PWM_TIM_Init(PWM_Tim_HandleDef *pwmt,uint8_t print,uint8_t function,uint8_t times,uint16_t period,uint16_t pwm);

/*
    低级PWM：
    pwm：地址
    print:管脚映射使能
    ***************注意！！！这里的管脚映射一旦打开，便是所有的PWM通道全部
    ***************都会映射，所以，我们初始化的LOW_PWM的这个参数要保持一致
    function:有效电平
    timfre:基准时钟 = timfre/系统时钟主频62400000  (单位s)
    period:周期（us）******这个参数也要保持一致，因为这几个PWM公用一个时钟线
    **************周期的参数只能填
                                PWMX_Cycle_256 = 0, // 256 个PWMX周期
                                PWMX_Cycle_255,     // 255 个PWMX周期
                                PWMX_Cycle_128,     // 128 个PWMX周期
                                PWMX_Cycle_127,     // 127 个PWMX周期
                                PWMX_Cycle_64,      // 64 个PWMX周期
                                PWMX_Cycle_63,      // 63 个PWMX周期

    pwm:占空比
*/
void WA_PWM_LOW_Init(PWMX_LOW_HandleDef *pwmlow,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm);

/*
    高级PWM：
    pwm：地址
    print:管脚映射使能
    ***************注意！！！这里的管脚映射一旦打开，便是所有的PWM通道全部
    ***************都会映射，所以，我们初始化的LOW_PWM的这个参数要保持一致
    function:有效电平
    timfre:基准时钟 = timfre/系统时钟主频62400000  (单位s)
    period:周期（us）******这个参数也要保持一致，因为这几个PWM公用一个时钟线
    pwm:占空比
*/
void WA_PWM_High_Init(PWMX_High_HandleDef *pwmhigh,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm);

/*
    实时更新PWM占空比函数
*/
void PWM_ALL_UpData(void *pwmall,uint16_t percent);

#endif 
