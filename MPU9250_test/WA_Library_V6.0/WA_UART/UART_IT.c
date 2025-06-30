#include "UART_IT.h"

/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART0_GetLinSTA();
            WA_UART_ErrorCallback(&wauart0);
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点,要跟接收超时一起用才能不丢包
            WA_UART_RxCallback(&wauart0);
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            WA_UART_RxCallback(&wauart0);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            WA_UART_TxCallback(&wauart0);
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}


/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART1_GetLinSTA();
            WA_UART_ErrorCallback(&wauart1);
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点,要跟接收超时一起用才能不丢包
            WA_UART_RxCallback(&wauart1);
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            WA_UART_RxCallback(&wauart1);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            WA_UART_TxCallback(&wauart1);
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART2_IRQHandler
 *
 * @brief   UART2中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART2_IRQHandler(void)
{
    switch(UART2_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART2_GetLinSTA();
            WA_UART_ErrorCallback(&wauart2);
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点,要跟接收超时一起用才能不丢包
            WA_UART_RxCallback(&wauart2);
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            WA_UART_RxCallback(&wauart2);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            WA_UART_TxCallback(&wauart2);
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   UART3中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART3_IRQHandler(void)
{
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT: // 线路状态错误
        {
            UART3_GetLinSTA();
            WA_UART_ErrorCallback(&wauart3);
            break;
        }

        case UART_II_RECV_RDY: // 数据达到设置触发点,要跟接收超时一起用才能不丢包
            WA_UART_RxCallback(&wauart3);
            break;

        case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
            WA_UART_RxCallback(&wauart3);
            break;

        case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
            WA_UART_TxCallback(&wauart3);
            break;

        case UART_II_MODEM_CHG: // 只支持串口0
            break;

        default:
            break;
    }
}


