/* �ٷ��ײ�ܽ�ӳ�������⣺TIM0��Ĭ��������PA9������ӳ�䲻��PB23����ΪPB23Ĭ�������ظ�λ����*/
/* �ٷ��ײ�ܽ�ӳ�������⣺TIM3��Ĭ��������PB22������ӳ�䵽PA2�����Ǹ��ļ���ע���Ǵ��*/
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

//������ӳ��ʹ��λ
#define Pin_Enable 1
#define Pin_Disable 0
#define null 2

//����һ���ṹ���Ŷ�ʱ������
/* 
mode:��ʱ��ģʽ || ������ģʽ
Frequency��Ƶ��Hz����ʱ��ģʽ��
print:�ܽ�ӳ��ʹ��
count_way:������ʽ
count����������ֵ
*/
typedef struct{
    uint16_t mode;
    uint32_t Frequency;

    uint8_t print;
    uint8_t count_way;
    uint32_t count;
}TIM_HandleDef;

//ֻ���ĸ���ʱ����Դ��ֱ��ȫ������
extern TIM_HandleDef watime0;
extern TIM_HandleDef watime1;
extern TIM_HandleDef watime2;
extern TIM_HandleDef watime3;

//��ʼ����ʱ��||������
/* 
watime:��ַ
mode:ģʽ   WA_TIMER    or  WA_COUNT
count_way��������ʽ
count������ֵ
*/
void WA_TIM_IT_Init(TIM_HandleDef *watime,uint16_t mode,uint8_t yingcshe,uint32_t fre,uint8_t count_way,uint32_t count);

/* ���������жϵĻص����� */
void WA_TIM_PeriodCallBack(TIM_HandleDef *watime);
/* ��������жϵĻص����� */
void WA_TIM_CountCallBack(TIM_HandleDef *watime);

#endif
