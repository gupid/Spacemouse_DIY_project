/********************************** (C) COPYRIGHT *******************************
 * File Name          : CH58x_usbdev.c
 * Author             : WCH
 * Version            : V1.2
 * Date               : 2021/11/17
 * Description        : source file(ch585/ch584)
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"

uint8_t *pEP0_RAM_Addr;
uint8_t *pEP1_RAM_Addr;
uint8_t *pEP2_RAM_Addr;
uint8_t *pEP3_RAM_Addr;

/*********************************************************************
 * @fn      USB_DeviceInit
 *
 * @brief   USB设备功能初始化，4个端点，8个通道。
 *
 * @param   none
 *
 * @return  none
 */
void USB_DeviceInit(void)
{
    R8_USB_CTRL = 0x00; // 先设定模式,取消 RB_UC_CLR_ALL

    R8_UEP4_1_MOD = RB_UEP4_RX_EN | RB_UEP4_TX_EN | RB_UEP1_RX_EN | RB_UEP1_TX_EN;
    R8_UEP2_3_MOD = RB_UEP2_RX_EN | RB_UEP2_TX_EN | RB_UEP3_RX_EN | RB_UEP3_TX_EN;

    R32_UEP0_DMA = (uint32_t)pEP0_RAM_Addr;
    R32_UEP1_DMA = (uint32_t)pEP1_RAM_Addr;
    R32_UEP2_DMA = (uint32_t)pEP2_RAM_Addr;
    R32_UEP3_DMA = (uint32_t)pEP3_RAM_Addr;
    // 注意：端点4没有独立的DMA地址寄存器，其地址基于EP0偏移，故无需设置R32_UEP4_DMA

    // 端点0: 控制传输 (手动TOG)
    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    // 端点1 (键盘 - 中断传输): 开启自动TOG
    R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    // 端点2 (SpaceMouse - 中断传输): 开启自动TOG
    R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    // 端点3 (CDC数据 - 批量传输): 关闭自动TOG
    R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    // 端点4 (CDC通知 - 中断传输): 开启自动TOG
    R8_UEP4_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;

    R8_USB_DEV_AD = 0x00;
    R8_USB_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN;
    R16_PIN_CONFIG |= RB_PIN_USB_EN | RB_UDP_PU_EN;
    R8_USB_INT_FG = 0xFF;
    R8_UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;
    R8_USB_INT_EN = RB_UIE_SUSPEND | RB_UIE_BUS_RST | RB_UIE_TRANSFER;
}
/*********************************************************************
 * @fn      DevEP1_IN_Deal
 *
 * @brief   端点1数据上传
 *
 * @param   l   - 上传数据长度(<64B)
 *
 * @return  none
 */
void DevEP1_IN_Deal(uint8_t l)
{
    R8_UEP1_T_LEN = l;
    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

/*********************************************************************
 * @fn      DevEP2_IN_Deal
 *
 * @brief   端点2数据上传
 *
 * @param   l   - 上传数据长度(<64B)
 *
 * @return  none
 */
void DevEP2_IN_Deal(uint8_t l)
{
    R8_UEP2_T_LEN = l;
    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

/*********************************************************************
 * @fn      DevEP3_IN_Deal
 *
 * @brief   端点3数据上传
 *
 * @param   l   - 上传数据长度(<64B)
 *
 * @return  none
 */
void DevEP3_IN_Deal(uint8_t l)
{
    R8_UEP3_T_LEN = l;
    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

/*********************************************************************
 * @fn      DevEP4_IN_Deal
 *
 * @brief   端点4数据上传
 *
 * @param   l   - 上传数据长度(<64B)
 *
 * @return  none
 */
void DevEP4_IN_Deal(uint8_t l)
{
    R8_UEP4_T_LEN = l;
    R8_UEP4_CTRL = (R8_UEP4_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}
