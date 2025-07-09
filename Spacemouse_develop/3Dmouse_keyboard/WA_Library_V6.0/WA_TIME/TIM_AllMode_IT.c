#include "TIM_AllMode_IT.h"

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void) // TMR0 ��ʱ�ж�
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // ���ڽ�����־����׽-��ʱ����ʱ-���ڽ�����PWM-���ڽ���
        if(watime0.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime0);
        if(watime0.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime0);

    }

    if(TMR0_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR0_ClearITFlag(TMR0_3_IT_DATA_ACT); // // ������Ч��־����׽-�����ݣ�PWM-��Ч��ƽ����
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR0_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO ʹ�ù��룺��׽- FIFO>=4�� PWM- FIFO<4
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR0_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // ʹ�õ���DMA����+�жϣ�ע����ɺ�رմ��ж�ʹ�ܣ������һֱ�ϱ��жϡ�
        TMR0_ClearITFlag(TMR0_3_IT_DMA_END); // DMA ������֧��TMR0-TMR3
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR0_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO �������׽- FIFO���� PWM- FIFO��
        
    }
}


/*********************************************************************
 * @fn      TMR1_IRQHandler
 *
 * @brief   TMR1�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void) // TMR1 ��ʱ�ж�
{
    if(TMR1_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR1_ClearITFlag(TMR0_3_IT_CYC_END); // ���ڽ�����־����׽-��ʱ����ʱ-���ڽ�����PWM-���ڽ���
        if(watime1.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime1);
        if(watime1.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime1);
    }

    if(TMR1_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR1_ClearITFlag(TMR0_3_IT_DATA_ACT); // // ������Ч��־����׽-�����ݣ�PWM-��Ч��ƽ����
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR1_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO ʹ�ù��룺��׽- FIFO>=4�� PWM- FIFO<4
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR1_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // ʹ�õ���DMA����+�жϣ�ע����ɺ�رմ��ж�ʹ�ܣ������һֱ�ϱ��жϡ�
        TMR1_ClearITFlag(TMR0_3_IT_DMA_END); // DMA ������֧��TMR0-TMR3
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR1_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO �������׽- FIFO���� PWM- FIFO��
        
    }
}

/*********************************************************************
 * @fn      TMR2_IRQHandler
 *
 * @brief   TMR2�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void) // TMR2 ��ʱ�ж�
{
    if(TMR2_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END); // ���ڽ�����־����׽-��ʱ����ʱ-���ڽ�����PWM-���ڽ���
        if(watime2.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime2);
        if(watime2.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime2);
    }

    if(TMR2_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR2_ClearITFlag(TMR0_3_IT_DATA_ACT); // // ������Ч��־����׽-�����ݣ�PWM-��Ч��ƽ����
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR2_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO ʹ�ù��룺��׽- FIFO>=4�� PWM- FIFO<4
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR2_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // ʹ�õ���DMA����+�жϣ�ע����ɺ�رմ��ж�ʹ�ܣ������һֱ�ϱ��жϡ�
        TMR2_ClearITFlag(TMR0_3_IT_DMA_END); // DMA ������֧��TMR0-TMR3
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR2_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO �������׽- FIFO���� PWM- FIFO��
        
    }
}

/*********************************************************************
 * @fn      TMR3_IRQHandler
 *
 * @brief   TMR3�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR3_IRQHandler(void) // TMR3 ��ʱ�ж�
{
    if(TMR3_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR3_ClearITFlag(TMR0_3_IT_CYC_END); // ���ڽ�����־����׽-��ʱ����ʱ-���ڽ�����PWM-���ڽ���
        if(watime3.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime3);
        if(watime3.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime3);
    }

    if(TMR3_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR3_ClearITFlag(TMR0_3_IT_DATA_ACT); // // ������Ч��־����׽-�����ݣ�PWM-��Ч��ƽ����
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR3_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO ʹ�ù��룺��׽- FIFO>=4�� PWM- FIFO<4
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR3_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // ʹ�õ���DMA����+�жϣ�ע����ɺ�رմ��ж�ʹ�ܣ������һֱ�ϱ��жϡ�
        TMR3_ClearITFlag(TMR0_3_IT_DMA_END); // DMA ������֧��TMR0-TMR3
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR3_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO �������׽- FIFO���� PWM- FIFO��
        
    }
}
