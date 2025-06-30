/* �ٷ��ײ�ܽ�ӳ�������⣺TIM0��Ĭ��������PA9������ӳ�䲻��PB23����ΪPB23Ĭ�������ظ�λ����*/
/* �ٷ��ײ�ܽ�ӳ�������⣺TIM3��Ĭ��������PB22������ӳ�䵽PA2�����Ǹ��ļ���ע���Ǵ��*/
#include "TIM.h"

/*  RB_PIN_TMR3   -  TMR3:  PA2 ->  PB22
 *  RB_PIN_TMR2   -  TMR2:  PA11 ->  PB11
 *  RB_PIN_TMR1   -  TMR1:  PA10 ->  PB10
 *  RB_PIN_TMR0   -  TMR0:  PA9 ->  PB23 */


//ֻ���ĸ���ʱ����Դ��ֱ��ȫ������
TIM_HandleDef watime0;
TIM_HandleDef watime1;
TIM_HandleDef watime2;
TIM_HandleDef watime3;
//�����Ӻ�����
void WA_TIM_IT_MspInit(TIM_HandleDef *watime);


//��ʼ����ʱ��||������
/* 
watime:��ַ
mode:ģʽ   WA_TIMER    or  WA_COUNT
yingcshe:�ܽ�ӳ��ʹ��
count_way��������ʽ
count������ֵ
*/
void WA_TIM_IT_Init(TIM_HandleDef *watime,uint16_t mode,uint8_t yingcshe,uint32_t fre,uint8_t count_way,uint32_t count)
{
    watime->Frequency = fre;
    watime->mode = mode;
    watime->count = count;
    watime->count_way = count_way;
    watime->print = yingcshe;
    //�����Ӻ�����
    WA_TIM_IT_MspInit(watime);
}

//�����Ӻ�����
void WA_TIM_IT_MspInit(TIM_HandleDef *watime)
{
    if(watime == &watime0)
    {
        if(watime->mode == WA_TIMER)
        {
            TMR0_TimerInit(FREQ_SYS  / watime->Frequency);         // ���ö�ʱʱ��
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);                                  // �����ж�
            PFIC_EnableIRQ(TMR0_IRQn);
        }

        if(watime->mode == WA_COUNT)
        {
            if(watime->print == Pin_Enable)
            {
                GPIOB_ModeCfg(GPIO_Pin_23, GPIO_ModeIN_PD);
                GPIOPinRemap(ENABLE, RB_PIN_TMR0);
            }
            else 
            {
                GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeIN_PD);
            }
            TMR0_EXTSingleCounterInit(watime->count_way);
            TMR0_CountOverflowCfg(watime->count); 
            /* ������������жϣ�����1000�����ڽ����ж� */
            TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR0_IRQn);
            TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
        }
        
    }

    if(watime == &watime1)
    {
        if(watime->mode == WA_TIMER)
        {
            TMR1_TimerInit(FREQ_SYS / watime->Frequency);         // ���ö�ʱʱ�� 100ms
            TMR1_ITCfg(ENABLE, TMR0_3_IT_CYC_END);                                  // �����ж�
            PFIC_EnableIRQ(TMR1_IRQn);
        }

        if(watime->mode == WA_COUNT)
        {
            if(watime->print == Pin_Enable)
            {
                GPIOB_ModeCfg(GPIO_Pin_10, GPIO_ModeIN_PD);
                GPIOPinRemap(ENABLE, RB_PIN_TMR1);
            }
            else 
            {
                GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeIN_PD);
            }
            TMR1_EXTSingleCounterInit(watime->count_way);
            TMR1_CountOverflowCfg(watime->count); 
            /* ������������жϣ�����1000�����ڽ����ж� */
            TMR1_ClearITFlag(TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR1_IRQn);
            TMR1_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
        }
     
    }

    if(watime == &watime2)
    {
        if(watime->mode == WA_TIMER)
        {
            TMR2_TimerInit(FREQ_SYS / watime->Frequency);         // ���ö�ʱʱ�� 100ms
            TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);                                  // �����ж�
            PFIC_EnableIRQ(TMR2_IRQn);
        }

        if(watime->mode == WA_COUNT)
        {
            if(watime->print == Pin_Enable)
            {
                GPIOB_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PD);
                GPIOPinRemap(ENABLE, RB_PIN_TMR2);
            }
            else 
            {
                GPIOA_ModeCfg(GPIO_Pin_11, GPIO_ModeIN_PD);
            }
            TMR2_EXTSingleCounterInit(watime->count_way);
            TMR2_CountOverflowCfg(watime->count); 
            /* ������������жϣ�����1000�����ڽ����ж� */
            TMR2_ClearITFlag(TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR2_IRQn);
            TMR2_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
        }
    }

    if(watime == &watime3)
    {
        if(watime->mode == WA_TIMER)
        {
            TMR3_TimerInit(FREQ_SYS / watime->Frequency);         // ���ö�ʱʱ�� 100ms
            TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END);                                  // �����ж�
            PFIC_EnableIRQ(TMR3_IRQn);
        }

        if(watime->mode == WA_COUNT)
        {
            if(watime->print == Pin_Enable)
            {
                GPIOA_ModeCfg(GPIO_Pin_2, GPIO_ModeIN_PD);
                GPIOPinRemap(ENABLE, RB_PIN_TMR3);
            }
            else 
            {
                GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeIN_PD);
            }
            TMR3_EXTSingleCounterInit(watime->count_way);
            TMR3_CountOverflowCfg(watime->count); 
            /* ������������жϣ�����1000�����ڽ����ж� */
            TMR3_ClearITFlag(TMR0_3_IT_CYC_END);
            PFIC_EnableIRQ(TMR3_IRQn);
            TMR3_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
        }
        
    }
}


/* ���������жϵĻص����� */
__weak_symbol void WA_TIM_PeriodCallBack(TIM_HandleDef *watime)
{
    /* ��ʼ��ĳ��� */
}

/* ��������жϵĻص����� */
__weak_symbol void WA_TIM_CountCallBack(TIM_HandleDef *watime)
{
    /* ��ʼ��ĳ��� */
}

