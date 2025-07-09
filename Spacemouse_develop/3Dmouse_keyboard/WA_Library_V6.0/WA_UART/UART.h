#ifndef __UART_H
#define __UART_H



// #include "CH58x_common.h"

// #include "stdio.h"
// #include "string.h"
#include "main.h"

// ����һ������ָ��ָ����Ӧ��UART��GPIO���ú���
typedef void(*GPIO_UART_Init) (uint32_t, GPIOModeTypeDef);

// ����һ�����������UART�Ľṹ����UART�Ĳ����ʺ�UART�������ú���
typedef struct Uart_HandleDef{
    uint32_t baudrate;              //����������
    GPIO_UART_Init GpioUartInit;    //Uart��GPIO���ú���

    uint8_t UartByte;               //����жϵĴ����ֽ� UARTByteTRIGTypeDef
    uint8_t enable;                 //�����ж�����ʹ��λ
    uint8_t RB_IER;                 //�����ж������շ�λ
    uint8_t UartIRQn;               //�����ж�����
}Uart_HandleDef;

extern Uart_HandleDef wauart0;
extern Uart_HandleDef wauart1;
extern Uart_HandleDef wauart2;
extern Uart_HandleDef wauart3;


//���ڳ�ʼ�����������ĸ�������Դ��ʼ��
//wauart:�ṹ���ַ;
//botelu��������;
void WA_UART_Init(Uart_HandleDef *wauart,uint32_t botelu);

//�����жϳ�ʼ��
void WA_UART_IT_Init(Uart_HandleDef *wauart,uint32_t botelu);


//�ص�����
void WA_UART_RxCallback(Uart_HandleDef *wauart);
void WA_UART_TxCallback(Uart_HandleDef *wauart);
void WA_UART_ErrorCallback(Uart_HandleDef *wauart);

//����MRS��������ӳ��
/*���е� DEBUG ������MRS��������Properties��� C compiler�µ�Propercessor*/
#ifdef  DEBUG
#define WA_printf(X...) printf(X)
#else
#define PRINT(X...)
#endif

#endif
