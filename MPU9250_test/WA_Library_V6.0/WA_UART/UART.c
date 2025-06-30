/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : ITKeiller
 * Version            : V1.0
 * Date               : 2025/04/30
 * Description        : 串口
 *********************************************************************************
    CH585的资源少，通信协议都直接每个文件都配置有
    所以，本文件是综合调用API作初始化，以及串口的
    重映射功能。
 *******************************************************************************/

#include "UART.h"

Uart_HandleDef wauart0;
Uart_HandleDef wauart1;
Uart_HandleDef wauart2;
Uart_HandleDef wauart3;

//函数声明
void WA_UART_MspInit(Uart_HandleDef *wauart);
void WA_Uart_DefInit(Uart_HandleDef *wauart);
void WA_UART_IT_MspInit(Uart_HandleDef *wauart);


//串口初始化函数调用四个串口资源初始化
//wauart:结构体地址;
//botelu：波特率;
void WA_UART_Init(Uart_HandleDef *wauart,uint32_t botelu)
{
    /*
        制作结构体-------“配置清单”
        *************************
        由于CH585这一系列的通信资源少就4个串口
        所以干脆全部配置清单都列好了，后续选取
        对应的结构体就好。
        *************************
    */
    if(wauart == &wauart0)
    {
        wauart->baudrate = botelu;
        wauart->GpioUartInit = GPIOB_ModeCfg;
    }

    if(wauart == &wauart1)
    {
        wauart->baudrate = botelu;
        wauart->GpioUartInit = GPIOA_ModeCfg;
    }

    if(wauart == &wauart2)
    {
        wauart->baudrate = botelu;
        wauart->GpioUartInit = GPIOA_ModeCfg;
    }

    if(wauart == &wauart3)
    {
        wauart->baudrate = botelu;
        wauart->GpioUartInit = GPIOA_ModeCfg;
    }

    /*"厨子函数"*/
    WA_UART_MspInit(wauart);
}

//串口中断初始化
void WA_UART_IT_Init(Uart_HandleDef *wauart,uint32_t botelu)
{
    WA_UART_Init(wauart,botelu);

    if(wauart == &wauart0)
    {
        wauart->UartByte = UART_7BYTE_TRIG;
        wauart->enable = ENABLE;
        wauart->RB_IER = RB_IER_RECV_RDY | RB_IER_LINE_STAT | RB_IER_THR_EMPTY;
        wauart->UartIRQn = UART0_IRQn;       
    }

    if(wauart == &wauart1)
    {
        wauart->UartByte = UART_7BYTE_TRIG;
        wauart->enable = ENABLE;
        wauart->RB_IER = RB_IER_RECV_RDY | RB_IER_LINE_STAT | RB_IER_THR_EMPTY;
        wauart->UartIRQn = UART1_IRQn;
    }

    if(wauart == &wauart2)
    {
        wauart->UartByte = UART_7BYTE_TRIG;
        wauart->enable = ENABLE;
        wauart->RB_IER = RB_IER_RECV_RDY | RB_IER_LINE_STAT | RB_IER_THR_EMPTY;
        wauart->UartIRQn = UART2_IRQn;
    }

    if(wauart == &wauart3)
    {
        wauart->UartByte = UART_7BYTE_TRIG;
        wauart->enable = ENABLE;
        wauart->RB_IER = RB_IER_RECV_RDY | RB_IER_LINE_STAT | RB_IER_THR_EMPTY;
        wauart->UartIRQn = UART3_IRQn;
    }
    //“厨子函数”：对“配置清单”进行处理
    WA_UART_IT_MspInit(wauart);
}

//“厨子函数”：对“配置清单”进行处理
void WA_UART_IT_MspInit(Uart_HandleDef *wauart)
{

    if(wauart == &wauart0)
    {
        UART0_ByteTrigCfg(wauart->UartByte);
        UART0_INTCfg(wauart->enable, wauart->RB_IER);
        PFIC_EnableIRQ(wauart->UartIRQn);
    }

     if(wauart == &wauart1)
    {
        UART1_ByteTrigCfg(wauart->UartByte);
        UART1_INTCfg(wauart->enable, wauart->RB_IER);
        PFIC_EnableIRQ(wauart->UartIRQn);
    }

     if(wauart == &wauart2)
    {
        UART2_ByteTrigCfg(wauart->UartByte);
        UART2_INTCfg(wauart->enable, wauart->RB_IER);
        PFIC_EnableIRQ(wauart->UartIRQn);
    }

     if(wauart == &wauart3)
    {
        UART3_ByteTrigCfg(wauart->UartByte);
        UART3_INTCfg(wauart->enable, wauart->RB_IER);
        PFIC_EnableIRQ(wauart->UartIRQn);
    }
}

//“厨子函数”：对“配置清单”进行处理
void WA_UART_MspInit(Uart_HandleDef *wauart)
{
    if(wauart == &wauart0)
    {
        GPIOB_SetBits(GPIO_Pin_7);                           //PB7---TXD0
        wauart->GpioUartInit(GPIO_Pin_4,GPIO_ModeIN_PU);     //PB4---RXD0
        wauart->GpioUartInit(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);//PB7---TXD0
        WA_Uart_DefInit(wauart);
    }

    if(wauart == &wauart1)
    {
        GPIOA_SetBits(GPIO_Pin_9);                           //PA9---TXD1
        wauart->GpioUartInit(GPIO_Pin_4,GPIO_ModeIN_PU);     //PA8---RXD1
        wauart->GpioUartInit(GPIO_Pin_9,GPIO_ModeOut_PP_5mA);//PA9---TXD1
        WA_Uart_DefInit(wauart);
    }

    if(wauart == &wauart2)
    {
        GPIOA_SetBits(GPIO_Pin_7);                           //PA7---TXD2
        wauart->GpioUartInit(GPIO_Pin_6,GPIO_ModeIN_PU);     //PA6---RXD2
        wauart->GpioUartInit(GPIO_Pin_7,GPIO_ModeOut_PP_5mA);//PA7---TXD2
        WA_Uart_DefInit(wauart);
    }

    if(wauart == &wauart3)
    {
        GPIOA_SetBits(GPIO_Pin_5);                           //PA5---TXD3
        wauart->GpioUartInit(GPIO_Pin_4,GPIO_ModeIN_PU);     //PA4---RXD3
        wauart->GpioUartInit(GPIO_Pin_5,GPIO_ModeOut_PP_5mA);//PA5---TXD3
        WA_Uart_DefInit(wauart);
    }

}

//对UART0_DefInit(),UART1_DefInit等的重新编写
//优点:方便自定义波特率
void WA_Uart_DefInit(Uart_HandleDef *wauart)
{
    if(wauart == &wauart0)
    {
        UART0_BaudRateCfg(wauart->baudrate);
        R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        R8_UART0_LCR = RB_LCR_WORD_SZ;
        R8_UART0_IER = RB_IER_TXD_EN;
        R8_UART0_DIV = 1;
    }

    if(wauart == &wauart1)
    {
        UART1_BaudRateCfg(wauart->baudrate);
        R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        R8_UART1_LCR = RB_LCR_WORD_SZ;
        R8_UART1_IER = RB_IER_TXD_EN;
        R8_UART1_DIV = 1;
    }

    if(wauart == &wauart2)
    {
        UART2_BaudRateCfg(wauart->baudrate);
        R8_UART2_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        R8_UART2_LCR = RB_LCR_WORD_SZ;
        R8_UART2_IER = RB_IER_TXD_EN;
        R8_UART2_DIV = 1;
    }

    if(wauart == &wauart3)
    {
        UART3_BaudRateCfg(wauart->baudrate);
        R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO打开，触发点4字节
        R8_UART3_LCR = RB_LCR_WORD_SZ;
        R8_UART3_IER = RB_IER_TXD_EN;
        R8_UART3_DIV = 1;
    }
}

//接收回调函数：方便在不同文件调用处理中断数据
__weak_symbol void WA_UART_RxCallback(Uart_HandleDef *wauart)
{
    /*接收回调函数：方便在不同文件调用处理中断数据*/
}

//发送回调函数：方便在不同文件调用处理中断数据
__weak_symbol void WA_UART_TxCallback(Uart_HandleDef *wauart)
{
    /*接收回调函数：方便在不同文件调用处理中断数据*/
}

//线路错误回调函数：方便在不同文件调用处理中断数据
__weak_symbol void WA_UART_ErrorCallback(Uart_HandleDef *wauart)
{
    /*接收回调函数：方便在不同文件调用处理中断数据*/
}
