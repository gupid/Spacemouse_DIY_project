/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : ITKeiller
 * Version            : V1.0
 * Date               : 2025/04/30
 * Description        : ����
 *********************************************************************************
    CH585����Դ�٣�ͨ��Э�鶼ֱ��ÿ���ļ���������
    ���ԣ����ļ����ۺϵ���API����ʼ�����Լ����ڵ�
    ��ӳ�书�ܡ�
 *******************************************************************************/

#include "UART.h"

Uart_HandleDef wauart0;
Uart_HandleDef wauart1;
Uart_HandleDef wauart2;
Uart_HandleDef wauart3;

//��������
void WA_UART_MspInit(Uart_HandleDef *wauart);
void WA_Uart_DefInit(Uart_HandleDef *wauart);
void WA_UART_IT_MspInit(Uart_HandleDef *wauart);


//���ڳ�ʼ�����������ĸ�������Դ��ʼ��
//wauart:�ṹ���ַ;
//botelu��������;
void WA_UART_Init(Uart_HandleDef *wauart,uint32_t botelu)
{
    /*
        �����ṹ��-------�������嵥��
        *************************
        ����CH585��һϵ�е�ͨ����Դ�پ�4������
        ���Ըɴ�ȫ�������嵥���к��ˣ�����ѡȡ
        ��Ӧ�Ľṹ��ͺá�
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

    /*"���Ӻ���"*/
    WA_UART_MspInit(wauart);
}

//�����жϳ�ʼ��
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
    //�����Ӻ��������ԡ������嵥�����д���
    WA_UART_IT_MspInit(wauart);
}

//�����Ӻ��������ԡ������嵥�����д���
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

//�����Ӻ��������ԡ������嵥�����д���
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

//��UART0_DefInit(),UART1_DefInit�ȵ����±�д
//�ŵ�:�����Զ��岨����
void WA_Uart_DefInit(Uart_HandleDef *wauart)
{
    if(wauart == &wauart0)
    {
        UART0_BaudRateCfg(wauart->baudrate);
        R8_UART0_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�򿪣�������4�ֽ�
        R8_UART0_LCR = RB_LCR_WORD_SZ;
        R8_UART0_IER = RB_IER_TXD_EN;
        R8_UART0_DIV = 1;
    }

    if(wauart == &wauart1)
    {
        UART1_BaudRateCfg(wauart->baudrate);
        R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�򿪣�������4�ֽ�
        R8_UART1_LCR = RB_LCR_WORD_SZ;
        R8_UART1_IER = RB_IER_TXD_EN;
        R8_UART1_DIV = 1;
    }

    if(wauart == &wauart2)
    {
        UART2_BaudRateCfg(wauart->baudrate);
        R8_UART2_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�򿪣�������4�ֽ�
        R8_UART2_LCR = RB_LCR_WORD_SZ;
        R8_UART2_IER = RB_IER_TXD_EN;
        R8_UART2_DIV = 1;
    }

    if(wauart == &wauart3)
    {
        UART3_BaudRateCfg(wauart->baudrate);
        R8_UART3_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN; // FIFO�򿪣�������4�ֽ�
        R8_UART3_LCR = RB_LCR_WORD_SZ;
        R8_UART3_IER = RB_IER_TXD_EN;
        R8_UART3_DIV = 1;
    }
}

//���ջص������������ڲ�ͬ�ļ����ô����ж�����
__weak_symbol void WA_UART_RxCallback(Uart_HandleDef *wauart)
{
    /*���ջص������������ڲ�ͬ�ļ����ô����ж�����*/
}

//���ͻص������������ڲ�ͬ�ļ����ô����ж�����
__weak_symbol void WA_UART_TxCallback(Uart_HandleDef *wauart)
{
    /*���ջص������������ڲ�ͬ�ļ����ô����ж�����*/
}

//��·����ص������������ڲ�ͬ�ļ����ô����ж�����
__weak_symbol void WA_UART_ErrorCallback(Uart_HandleDef *wauart)
{
    /*���ջص������������ڲ�ͬ�ļ����ô����ж�����*/
}
