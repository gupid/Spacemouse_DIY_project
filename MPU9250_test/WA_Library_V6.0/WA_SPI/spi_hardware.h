#ifndef __SPI_HARDWARE_H
#define __SPI_HARDWARE_H

#include "main.h"

#define Master 1
#define Slave  2

/*
    ���ڿ��ǵ�CH58X��CH57X��ϵ��оƬ�ļ����ԣ�
    spi0��Ϊ��ϵ��оƬ���е�Ӳ��SPI����ô����
    ��Ӳ��SPI��ֻ��SPI0.
    SPI0�Ĺܽ�ӳ�䣺
    RB_PIN_SPI0   -  SPI0:  PA12/PA13/PA14/PA15 -> PB12/PB13/PB14/PB15
*/
void WA_SPI0_Hardware_Init(uint8_t mode,uint8_t pinenable);

void SPI0_CSS_H();
void SPI0_CSS_L();


#endif
