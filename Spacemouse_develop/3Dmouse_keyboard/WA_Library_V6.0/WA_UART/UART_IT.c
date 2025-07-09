#include "UART_IT.h"

/*********************************************************************
 * @fn      UART0_IRQHandler
 *
 * @brief   UART0�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT: // ��·״̬����
        {
            UART0_GetLinSTA();
            WA_UART_ErrorCallback(&wauart0);
            break;
        }

        case UART_II_RECV_RDY: // ���ݴﵽ���ô�����,Ҫ�����ճ�ʱһ���ò��ܲ�����
            WA_UART_RxCallback(&wauart0);
            break;

        case UART_II_RECV_TOUT: // ���ճ�ʱ����ʱһ֡���ݽ������
            WA_UART_RxCallback(&wauart0);
            break;

        case UART_II_THR_EMPTY: // ���ͻ������գ��ɼ�������
            WA_UART_TxCallback(&wauart0);
            break;

        case UART_II_MODEM_CHG: // ֻ֧�ִ���0
            break;

        default:
            break;
    }
}


/*********************************************************************
 * @fn      UART1_IRQHandler
 *
 * @brief   UART1�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART1_IRQHandler(void)
{
    switch(UART1_GetITFlag())
    {
        case UART_II_LINE_STAT: // ��·״̬����
        {
            UART1_GetLinSTA();
            WA_UART_ErrorCallback(&wauart1);
            break;
        }

        case UART_II_RECV_RDY: // ���ݴﵽ���ô�����,Ҫ�����ճ�ʱһ���ò��ܲ�����
            WA_UART_RxCallback(&wauart1);
            break;

        case UART_II_RECV_TOUT: // ���ճ�ʱ����ʱһ֡���ݽ������
            WA_UART_RxCallback(&wauart1);
            break;

        case UART_II_THR_EMPTY: // ���ͻ������գ��ɼ�������
            WA_UART_TxCallback(&wauart1);
            break;

        case UART_II_MODEM_CHG: // ֻ֧�ִ���0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART2_IRQHandler
 *
 * @brief   UART2�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART2_IRQHandler(void)
{
    switch(UART2_GetITFlag())
    {
        case UART_II_LINE_STAT: // ��·״̬����
        {
            UART2_GetLinSTA();
            WA_UART_ErrorCallback(&wauart2);
            break;
        }

        case UART_II_RECV_RDY: // ���ݴﵽ���ô�����,Ҫ�����ճ�ʱһ���ò��ܲ�����
            WA_UART_RxCallback(&wauart2);
            break;

        case UART_II_RECV_TOUT: // ���ճ�ʱ����ʱһ֡���ݽ������
            WA_UART_RxCallback(&wauart2);
            break;

        case UART_II_THR_EMPTY: // ���ͻ������գ��ɼ�������
            WA_UART_TxCallback(&wauart2);
            break;

        case UART_II_MODEM_CHG: // ֻ֧�ִ���0
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      UART3_IRQHandler
 *
 * @brief   UART3�жϺ���
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void UART3_IRQHandler(void)
{
    switch(UART3_GetITFlag())
    {
        case UART_II_LINE_STAT: // ��·״̬����
        {
            UART3_GetLinSTA();
            WA_UART_ErrorCallback(&wauart3);
            break;
        }

        case UART_II_RECV_RDY: // ���ݴﵽ���ô�����,Ҫ�����ճ�ʱһ���ò��ܲ�����
            WA_UART_RxCallback(&wauart3);
            break;

        case UART_II_RECV_TOUT: // ���ճ�ʱ����ʱһ֡���ݽ������
            WA_UART_RxCallback(&wauart3);
            break;

        case UART_II_THR_EMPTY: // ���ͻ������գ��ɼ�������
            WA_UART_TxCallback(&wauart3);
            break;

        case UART_II_MODEM_CHG: // ֻ֧�ִ���0
            break;

        default:
            break;
    }
}


