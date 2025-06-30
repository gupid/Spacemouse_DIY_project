#ifndef __SPI_HARDWARE_H
#define __SPI_HARDWARE_H

#include "main.h"

#define Master 1
#define Slave  2

/*
    由于考虑到CH58X和CH57X等系列芯片的兼容性：
    spi0作为这系列芯片都有的硬件SPI，那么我们
    的硬件SPI都只用SPI0.
    SPI0的管脚映射：
    RB_PIN_SPI0   -  SPI0:  PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
*/
void WA_SPI0_Hardware_Init(uint8_t mode,uint8_t pinenable);

void SPI0_CSS_H();
void SPI0_CSS_L();


#endif
