#include "TIM_AllMode_IT.h"

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler(void) // TMR0 定时中断
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // 周期结束标志：捕捉-超时，定时-周期结束，PWM-周期结束
        if(watime0.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime0);
        if(watime0.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime0);

    }

    if(TMR0_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR0_ClearITFlag(TMR0_3_IT_DATA_ACT); // // 数据有效标志：捕捉-新数据，PWM-有效电平结束
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR0_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO 使用过半：捕捉- FIFO>=4， PWM- FIFO<4
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR0_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR0_ClearITFlag(TMR0_3_IT_DMA_END); // DMA 结束，支持TMR0-TMR3
        
    }

    if(TMR0_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR0_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO 溢出：捕捉- FIFO满， PWM- FIFO空
        
    }
}


/*********************************************************************
 * @fn      TMR1_IRQHandler
 *
 * @brief   TMR1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler(void) // TMR1 定时中断
{
    if(TMR1_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR1_ClearITFlag(TMR0_3_IT_CYC_END); // 周期结束标志：捕捉-超时，定时-周期结束，PWM-周期结束
        if(watime1.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime1);
        if(watime1.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime1);
    }

    if(TMR1_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR1_ClearITFlag(TMR0_3_IT_DATA_ACT); // // 数据有效标志：捕捉-新数据，PWM-有效电平结束
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR1_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO 使用过半：捕捉- FIFO>=4， PWM- FIFO<4
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR1_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR1_ClearITFlag(TMR0_3_IT_DMA_END); // DMA 结束，支持TMR0-TMR3
        
    }

    if(TMR1_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR1_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO 溢出：捕捉- FIFO满， PWM- FIFO空
        
    }
}

/*********************************************************************
 * @fn      TMR2_IRQHandler
 *
 * @brief   TMR2中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void) // TMR2 定时中断
{
    if(TMR2_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR2_ClearITFlag(TMR0_3_IT_CYC_END); // 周期结束标志：捕捉-超时，定时-周期结束，PWM-周期结束
        if(watime2.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime2);
        if(watime2.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime2);
    }

    if(TMR2_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR2_ClearITFlag(TMR0_3_IT_DATA_ACT); // // 数据有效标志：捕捉-新数据，PWM-有效电平结束
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR2_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO 使用过半：捕捉- FIFO>=4， PWM- FIFO<4
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR2_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR2_ClearITFlag(TMR0_3_IT_DMA_END); // DMA 结束，支持TMR0-TMR3
        
    }

    if(TMR2_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR2_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO 溢出：捕捉- FIFO满， PWM- FIFO空
        
    }
}

/*********************************************************************
 * @fn      TMR3_IRQHandler
 *
 * @brief   TMR3中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void TMR3_IRQHandler(void) // TMR3 定时中断
{
    if(TMR3_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR3_ClearITFlag(TMR0_3_IT_CYC_END); // 周期结束标志：捕捉-超时，定时-周期结束，PWM-周期结束
        if(watime3.mode == WA_TIMER)
            WA_TIM_PeriodCallBack(&watime3);
        if(watime3.mode == WA_COUNT)
            WA_TIM_CountCallBack(&watime3);
    }

    if(TMR3_GetITFlag(TMR0_3_IT_DATA_ACT))
    {
        TMR3_ClearITFlag(TMR0_3_IT_DATA_ACT); // // 数据有效标志：捕捉-新数据，PWM-有效电平结束
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_FIFO_HF))
    {
        TMR3_ClearITFlag(TMR0_3_IT_FIFO_HF); // FIFO 使用过半：捕捉- FIFO>=4， PWM- FIFO<4
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_DMA_END))
    {
        TMR3_ITCfg(DISABLE, TMR0_3_IT_DMA_END); // 使用单次DMA功能+中断，注意完成后关闭此中断使能，否则会一直上报中断。
        TMR3_ClearITFlag(TMR0_3_IT_DMA_END); // DMA 结束，支持TMR0-TMR3
        
    }

    if(TMR3_GetITFlag(TMR0_3_IT_FIFO_OV))
    {
        TMR3_ClearITFlag(TMR0_3_IT_FIFO_OV); // FIFO 溢出：捕捉- FIFO满， PWM- FIFO空
        
    }
}
