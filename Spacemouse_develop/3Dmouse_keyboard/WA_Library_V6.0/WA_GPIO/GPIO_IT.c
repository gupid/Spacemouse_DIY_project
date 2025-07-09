#include "GPIO_IT.h"
#include "UART.h"
/* GPIOA外部中断服务函数 */
__INTERRUPT
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    WA_GPIOA_IQRHandler();
}


/* GPIOB外部中断服务函数 */
__INTERRUPT
__HIGH_CODE
void GPIOB_IRQHandler(void)
{
    WA_GPIOB_IQRHandler();   
}

//中断服务分配函数
void WA_GPIOA_IQRHandler(void)
{
    if(GPIOA_ReadITFlagBit(GPIO_Pin_0))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_0);
        WA_GPIOA_EventCallBack(GPIO_Pin_0);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_1))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_1);
        WA_GPIOA_EventCallBack(GPIO_Pin_1);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_2))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_2);
        WA_GPIOA_EventCallBack(GPIO_Pin_2);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_3))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_3);
        WA_GPIOA_EventCallBack(GPIO_Pin_3);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_4))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_4);
        WA_GPIOA_EventCallBack(GPIO_Pin_4);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_5))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_5);
        WA_GPIOA_EventCallBack(GPIO_Pin_5);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_6))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_6);
        WA_GPIOA_EventCallBack(GPIO_Pin_6);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_7))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_7);
        WA_GPIOA_EventCallBack(GPIO_Pin_7);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_8))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_8);
        WA_GPIOA_EventCallBack(GPIO_Pin_8);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_9))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_9);
        WA_GPIOA_EventCallBack(GPIO_Pin_9);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_10))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_10);
        WA_GPIOA_EventCallBack(GPIO_Pin_10);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_11))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_11);
        WA_GPIOA_EventCallBack(GPIO_Pin_11);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_12))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_12);
        WA_GPIOA_EventCallBack(GPIO_Pin_12);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_13))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_13);
        WA_GPIOA_EventCallBack(GPIO_Pin_13);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_14))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_14);
        WA_GPIOA_EventCallBack(GPIO_Pin_14);
    }

    if(GPIOA_ReadITFlagBit(GPIO_Pin_15))
    {
        GPIOA_ClearITFlagBit(GPIO_Pin_15);
        WA_GPIOA_EventCallBack(GPIO_Pin_15);
    }
}

//中断服务分配函数
void WA_GPIOB_IQRHandler(void)
{
    if(GPIOB_ReadITFlagBit(GPIO_Pin_0))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_0);
        WA_GPIOB_EventCallBack(GPIO_Pin_0);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_1))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_1);
        WA_GPIOB_EventCallBack(GPIO_Pin_1);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_2))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_2);
        WA_GPIOB_EventCallBack(GPIO_Pin_2);
    }

    if(GPIOB_ClearITFlagBit(GPIO_Pin_3))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_3);
        WA_GPIOB_EventCallBack(GPIO_Pin_3);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_4))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_4);
        WA_GPIOB_EventCallBack(GPIO_Pin_4);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_5))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_5);
        WA_GPIOB_EventCallBack(GPIO_Pin_5);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_6))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_6);
        WA_GPIOB_EventCallBack(GPIO_Pin_6);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_7))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_7);
        WA_GPIOB_EventCallBack(GPIO_Pin_7);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_8))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_8);
        WA_GPIOB_EventCallBack(GPIO_Pin_8);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_9))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_9);
        WA_GPIOB_EventCallBack(GPIO_Pin_9);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_10))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_10);
        WA_GPIOB_EventCallBack(GPIO_Pin_10);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_11))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_11);
        WA_GPIOB_EventCallBack(GPIO_Pin_11);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_12))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_12);
        WA_GPIOB_EventCallBack(GPIO_Pin_12);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_13))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_13);
        WA_GPIOB_EventCallBack(GPIO_Pin_13);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_14))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_14);
        WA_GPIOB_EventCallBack(GPIO_Pin_14);
    }

    if(GPIOB_ReadITFlagBit(GPIO_Pin_15))
    {
        GPIOB_ClearITFlagBit(GPIO_Pin_15);
        WA_GPIOB_EventCallBack(GPIO_Pin_15);
    }
}
