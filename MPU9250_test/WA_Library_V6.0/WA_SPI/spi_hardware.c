#include "spi_hardware.h"

uint8_t flag;

/*
    由于考虑到CH58X和CH57X等系列芯片的兼容性：
    spi0作为这系列芯片都有的硬件SPI，那么我们
    的硬件SPI都只用SPI0.
    SPI0的管脚映射：
    RB_PIN_SPI0   -  SPI0:  PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
*/
void WA_SPI0_Hardware_Init(uint8_t mode,uint8_t pinenable)
{
    if(pinenable == Pin_Enable)
    {   
        flag = 1;
        GPIOPinRemap(ENABLE,RB_PIN_SPI0);
            if(mode == Master)
        {
            GPIOB_SetBits(GPIO_Pin_12);
            GPIOB_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
            SPI0_MasterDefInit();
        }
            if(mode == Slave)
        {
            GPIOB_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_ModeIN_PU);
            SPI0_SlaveInit();
        }
    }
    else 
    {
            flag = 0;
            if(mode == Master)
        {
            GPIOA_SetBits(GPIO_Pin_12);
            GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeOut_PP_5mA);
            SPI0_MasterDefInit();
        }
            if(mode == Slave)
        {
            GPIOA_ModeCfg(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_ModeIN_PU);
            SPI0_SlaveInit();
        }
    }  
}



void SPI0_CSS_H()
{
    if(flag == 0)   //默认引脚
        GPIOA_SetBits(GPIO_Pin_12);
    if(flag == 1)
        GPIOB_SetBits(GPIO_Pin_12);
}

void SPI0_CSS_L()
{
    if(flag == 0)   //默认引脚
        GPIOA_ResetBits(GPIO_Pin_12);
    if(flag == 1)
        GPIOB_ResetBits(GPIO_Pin_12);
}




