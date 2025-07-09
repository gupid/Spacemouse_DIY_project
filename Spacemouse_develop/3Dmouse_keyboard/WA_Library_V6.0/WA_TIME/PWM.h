/* �ٷ��ײ�ܽ�ӳ�������⣺TIM0��Ĭ��������PA9������ӳ�䲻��PB23,��ΪPB23Ĭ�������ظ�λ����*/
/* �ٷ��ײ�ܽ�ӳ�������⣺TIM3��Ĭ��������PB22������ӳ�䵽PA2�����Ǹ��ļ���ע���Ǵ��*/
#ifndef __PWM_H
#define __PWM_H

#include "main.h"

#define  g_1us  (FREQ_SYS/1000000)


/*  RB_PIN_TMR3   -  TMR3:  PA2 ->  PB22
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */
//����һ���ṹ���Ŷ�ʱ����PWM����
/* 
print:�ܽ�ӳ��ʹ��
function:��Ч��ƽ
times:��Ч����ظ�����
period:���ڣ�us��
pwm:ռ�ձ�
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t times;
    uint16_t period;
    uint16_t pwm;
}PWM_Tim_HandleDef;

//����һ���ṹ���ŵͼ�PWM����
/* 
 // Ĭ��PA12 - PWM4--->ӳ��PA6
 // Ĭ��PA13 - PWM5--->ӳ��PA7
 // Ĭ��PB0 - PWM6**************����ӳ��
 // Ĭ��PB4 - PWM7--->ӳ��PB1
 // Ĭ��PB6 - PWM8--->ӳ��PB2
 // Ĭ��PB7 - PWM9--->ӳ��PB3
 // Ĭ��PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 �˽Ÿ���Ϊ�ⲿ��λ�ţ���Ҫ�رմ˹��ܲ��ܿ���PWM����
print:�ܽ�ӳ��ʹ��
function:��Ч��ƽ
timfre:��׼ʱ�� = timfre/ϵͳʱ����Ƶ62400000  (��λs)
period:���ڣ�us��
pwm:ռ�ձ�
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t timfre;
    uint16_t period;
    uint16_t pwm;
}PWMX_LOW_HandleDef;

//����һ���ṹ���Ÿ߼�PWM����
/* 
 // Ĭ��PA12 - PWM4--->ӳ��PA6
 // Ĭ��PA13 - PWM5--->ӳ��PA7
 // Ĭ��PB0 - PWM6**************����ӳ��
 // Ĭ��PB4 - PWM7--->ӳ��PB1
 // Ĭ��PB6 - PWM8--->ӳ��PB2
 // Ĭ��PB7 - PWM9--->ӳ��PB3
 // Ĭ��PB14 - PWM10
 //  GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeOut_PP_5mA); // PB23 - PWM11 �˽Ÿ���Ϊ�ⲿ��λ�ţ���Ҫ�رմ˹��ܲ��ܿ���PWM����
print:�ܽ�ӳ��ʹ��
function:��Ч��ƽ
timfre:��׼ʱ�� = timfre/ϵͳʱ����Ƶ62400000  (��λs)
period:���ڣ�us��
pwm:ռ�ձ�
*/
typedef struct{
    uint8_t print;
    uint8_t function;
    uint8_t timfre;
    uint16_t period;
    uint16_t pwm;
}PWMX_High_HandleDef;

//�ĸ���ʱ��������pwm
extern PWM_Tim_HandleDef wapwmt0;
extern PWM_Tim_HandleDef wapwmt1;
extern PWM_Tim_HandleDef wapwmt2;
extern PWM_Tim_HandleDef wapwmt3;

//����8���ͼ�PWM
extern PWMX_LOW_HandleDef wapwmlow4;
extern PWMX_LOW_HandleDef wapwmlow5;
extern PWMX_LOW_HandleDef wapwmlow6;
extern PWMX_LOW_HandleDef wapwmlow7;
extern PWMX_LOW_HandleDef wapwmlow8;
extern PWMX_LOW_HandleDef wapwmlow9;
extern PWMX_LOW_HandleDef wapwmlow10;
extern PWMX_LOW_HandleDef wapwmlow11;

//����6���߼�PWM
extern PWMX_High_HandleDef wapwmhigh4;
extern PWMX_High_HandleDef wapwmhigh5;
extern PWMX_High_HandleDef wapwmhigh6;
extern PWMX_High_HandleDef wapwmhigh7;
extern PWMX_High_HandleDef wapwmhigh8;
extern PWMX_High_HandleDef wapwmhigh9;

/*
    ��ʱ�����ŵ�PWM��
    pwmt����ַ
    print:�ܽ�ӳ��ʹ��
    function:��Ч��ƽ
    times:��Ч����ظ�����
    period:���ڣ�us��
    pwm:ռ�ձ�
*/
void WA_PWM_TIM_Init(PWM_Tim_HandleDef *pwmt,uint8_t print,uint8_t function,uint8_t times,uint16_t period,uint16_t pwm);

/*
    �ͼ�PWM��
    pwm����ַ
    print:�ܽ�ӳ��ʹ��
    ***************ע�⣡��������Ĺܽ�ӳ��һ���򿪣��������е�PWMͨ��ȫ��
    ***************����ӳ�䣬���ԣ����ǳ�ʼ����LOW_PWM���������Ҫ����һ��
    function:��Ч��ƽ
    timfre:��׼ʱ�� = timfre/ϵͳʱ����Ƶ62400000  (��λs)
    period:���ڣ�us��******�������ҲҪ����һ�£���Ϊ�⼸��PWM����һ��ʱ����
    **************���ڵĲ���ֻ����
                                PWMX_Cycle_256 = 0, // 256 ��PWMX����
                                PWMX_Cycle_255,     // 255 ��PWMX����
                                PWMX_Cycle_128,     // 128 ��PWMX����
                                PWMX_Cycle_127,     // 127 ��PWMX����
                                PWMX_Cycle_64,      // 64 ��PWMX����
                                PWMX_Cycle_63,      // 63 ��PWMX����

    pwm:ռ�ձ�
*/
void WA_PWM_LOW_Init(PWMX_LOW_HandleDef *pwmlow,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm);

/*
    �߼�PWM��
    pwm����ַ
    print:�ܽ�ӳ��ʹ��
    ***************ע�⣡��������Ĺܽ�ӳ��һ���򿪣��������е�PWMͨ��ȫ��
    ***************����ӳ�䣬���ԣ����ǳ�ʼ����LOW_PWM���������Ҫ����һ��
    function:��Ч��ƽ
    timfre:��׼ʱ�� = timfre/ϵͳʱ����Ƶ62400000  (��λs)
    period:���ڣ�us��******�������ҲҪ����һ�£���Ϊ�⼸��PWM����һ��ʱ����
    pwm:ռ�ձ�
*/
void WA_PWM_High_Init(PWMX_High_HandleDef *pwmhigh,uint8_t print,uint8_t function,uint8_t timfre,uint16_t period,uint16_t pwm);

/*
    ʵʱ����PWMռ�ձȺ���
*/
void PWM_ALL_UpData(void *pwmall,uint16_t percent);

#endif 
