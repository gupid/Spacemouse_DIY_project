#ifndef __UART_H
#define __UART_H



// #include "CH58x_common.h"

// #include "stdio.h"
// #include "string.h"
#include "main.h"

// 定义一个函数指针指向相应的UART的GPIO配置函数
typedef void(*GPIO_UART_Init) (uint32_t, GPIOModeTypeDef);

// 定义一个评估板板载UART的结构体存放UART的波特率和UART参数配置函数
typedef struct Uart_HandleDef{
    uint32_t baudrate;              //波特率设置
    GPIO_UART_Init GpioUartInit;    //Uart的GPIO配置函数

    uint8_t UartByte;               //存放中断的触发字节 UARTByteTRIGTypeDef
    uint8_t enable;                 //串口中断配置使能位
    uint8_t RB_IER;                 //串口中断配置收发位
    uint8_t UartIRQn;               //串口中断配置
}Uart_HandleDef;

extern Uart_HandleDef wauart0;
extern Uart_HandleDef wauart1;
extern Uart_HandleDef wauart2;
extern Uart_HandleDef wauart3;


//串口初始化函数调用四个串口资源初始化
//wauart:结构体地址;
//botelu：波特率;
void WA_UART_Init(Uart_HandleDef *wauart,uint32_t botelu);

//串口中断初始化
void WA_UART_IT_Init(Uart_HandleDef *wauart,uint32_t botelu);


//回调函数
void WA_UART_RxCallback(Uart_HandleDef *wauart);
void WA_UART_TxCallback(Uart_HandleDef *wauart);
void WA_UART_ErrorCallback(Uart_HandleDef *wauart);

//由于MRS环境的重映射
/*其中的 DEBUG 宏是在MRS编译器的Properties里的 C compiler下的Propercessor*/
#ifdef  DEBUG
#define WA_printf(X...) printf(X)
#else
#define PRINT(X...)
#endif

#endif
