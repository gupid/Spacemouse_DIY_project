#include "usb_hid_composite.h"


// 定义CDC类请求码
#define CDC_SET_LINE_CODING         0x20
#define CDC_GET_LINE_CODING         0x21
#define CDC_SET_CONTROL_LINE_STATE  0x22

// CDC串口线路状态变量 (7字节: BPS, StopBits, Parity, DataBits)
uint8_t CDC_LineCoding[] = {0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08}; // 115200bps, 1 stop, no parity, 8 data


#define DevEP0SIZE    0x40
#define CDC_RX_BUF_SIZE 64
uint8_t cdc_rx_buffer[CDC_RX_BUF_SIZE];
uint8_t cdc_rx_len = 0;

void cdc_send_data(const uint8_t* data, uint8_t len); // 提前声明数据发送函数

/*HID类报表描述符*/
const uint8_t KeyRepDesc[] = {0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25,
                              0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x03,
                              0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x03, 0x91, 0x02, 0x95, 0x05, 0x75, 0x01, 0x91,
                              0x01, 0x95, 0x06, 0x75, 0x08, 0x26, 0xff, 0x00, 0x05, 0x07, 0x19, 0x00, 0x29, 0x91, 0x81,
                              0x00, 0xC0};
// const uint8_t MouseRepDesc[] = {0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29,
//                                 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x75, 0x05, 0x95, 0x01,
//                                 0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75,
//                                 0x08, 0x95, 0x03, 0x81, 0x06, 0xC0, 0xC0};
const uint8_t SpaceMouseRepDesc[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x08,       // Usage (Multi-Axis)
    0xA1, 0x01,       // Collection (Application)
    
    // --- Report 1: Translation
    0xA1, 0x00,       // Collection (Physical)
    0x85, 0x01,       //   Report ID (1)
    0x16, 0xA2, 0xFE, //   Logical Minimum (-350)
    0x26, 0x5E, 0x01, //   Logical Maximum (350)
    0x36, 0x88, 0xFA, //   Physical Minimum (-1400) // [!!] 新增
    0x46, 0x78, 0x05, //   Physical Maximum (1400)  // [!!] 新增
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x32,       //   Usage (Z)
    0x75, 0x10,       //   Report Size (16)
    0x95, 0x03,       //   Report Count (3)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xC0,             // End Collection
    
    // --- Report 2: Rotation
    0xA1, 0x00,       // Collection (Physical)
    0x85, 0x02,       //   Report ID (2)
    0x16, 0xA2, 0xFE, //   Logical Minimum (-350)
    0x26, 0x5E, 0x01, //   Logical Maximum (350)
    0x36, 0x88, 0xFA, //   Physical Minimum (-1400) // [!!] 新增
    0x46, 0x78, 0x05, //   Physical Maximum (1400)  // [!!] 新增
    0x09, 0x33,       //   Usage (RX)
    0x09, 0x34,       //   Usage (RY)
    0x09, 0x35,       //   Usage (RZ)
    0x75, 0x10,       //   Report Size (16)
    0x95, 0x03,       //   Report Count (3)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xC0,             // End Collection
    
    // --- Report 3: Buttons
    0xA1, 0x00,          // Collection (Physical)
    0x85, 0x03,          //  Report ID (3)
    0x15, 0x00,          //   Logical Minimum (0)
    0x25, 0x01,          //   Logical Maximum (1)
    0x75, 0x01,          //   Report Size (1)
    0x95, 0x20,          //   Report Count (32) -> 32个按键
    0x05, 0x09,          //   Usage Page (Button)
    0x19, 0x01,          //   Usage Minimum (Button 1)
    0x29, 0x20,          //   Usage Maximum (Button 32)
    0x81, 0x02,          //   Input (Data,Var,Abs)
    0xC0,                // End Collection
    
    // --- Report 4: LEDs (Host -> Device)
    0xA1, 0x02,          //   Collection (Logical)
    0x85, 0x04,          //     Report ID (4)
    0x05, 0x08,          //     Usage Page (LEDs)
    0x09, 0x4B,          //     Usage (Generic Indicator)
    0x15, 0x00,          //     Logical Minimum (0)
    0x25, 0x01,          //     Logical Maximum (1)
    0x95, 0x01,          //     Report Count (1)
    0x75, 0x01,          //     Report Size (1)
    0x91, 0x02,          //     Output (Data,Var,Abs) // [!!] 属性与参考项目保持一致
    0x95, 0x01,          //     Report Count (1)
    0x75, 0x07,          //     Report Size (7)
    0x91, 0x03,          //     Output (Const,Var,Abs) // [!!] 属性与参考项目保持一致
    0xC0,                //   End Collection
    
    0xC0              // End Collection (Application)
};

// 设备描述符
const uint8_t MyDevDescr[] = {0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, DevEP0SIZE, 0x6F, 0x25, 0x32, 0xC6, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x01};
// 配置描述符
const uint8_t MyCfgDescr[] = {
    /******************** 配置描述符 ********************/
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    // [!! 已修正 !!] wTotalLength: 总长度 = 原HID(66) + 新CDC(66) = 132 (0x84)
    0x84, 0x00,
    // [!! 已修正 !!] bNumInterfaces: 接口总数 = 键盘(0) + SpaceMouse(1) + CDC通信(2) + CDC数据(3) = 4个
    0x04,
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration
    0xA0,        // bmAttributes (Bus-powered, Remote Wakeup)
    0x32,        // bMaxPower (100mA)

    /******************** 接口 0: 键盘 (保持不变) ********************/
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(KeyRepDesc), 0x00,
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A,

    /******************** 接口 1: SpaceMouse (保持不变) ********************/
    0x09, 0x04, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, sizeof(SpaceMouseRepDesc), 0x00,
    0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0x08,
    0x07, 0x05, 0x02, 0x03, 0x08, 0x00, 0x08,

    /******************** [!! 新增 !!] CDC 虚拟串口功能 ********************/
    /* (1) 接口关联描述符 (IAD) */
    0x08, 0x0B, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00,
    /* (2) CDC 通信接口 (Interface 2) */
    0x09, 0x04, 0x02, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    /* CDC 类特定描述符 */
    0x05, 0x24, 0x00, 0x10, 0x01, // Header
    0x05, 0x24, 0x01, 0x00, 0x03, // Call Management
    0x04, 0x24, 0x02, 0x02,       // ACM
    0x05, 0x24, 0x06, 0x02, 0x03, // Union
    /* CDC 通知端点 (Endpoint 4 IN) */
    0x07, 0x05, 0x84, 0x03, 0x08, 0x00, 0xFF,
    /* (3) CDC 数据接口 (Interface 3) */
    0x09, 0x04, 0x03, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
    /* CDC 数据端点 (Endpoint 3 OUT) */
    0x07, 0x05, 0x03, 0x02, 0x40, 0x00, 0x00,
    /* CDC 数据端点 (Endpoint 3 IN) */
    0x07, 0x05, 0x83, 0x02, 0x40, 0x00, 0x00
};

/* USB速度匹配描述符 */
const uint8_t My_QueDescr[] = {0x0A, 0x06, 0x00, 0x02, 0xFF, 0x00, 0xFF, 0x40, 0x01, 0x00};

/* USB全速模式,其他速度配置描述符 */
uint8_t USB_FS_OSC_DESC[sizeof(MyCfgDescr)] = {
    0x09, 0x07, /* 其他部分通过程序复制 */
};

// 语言描述符
const uint8_t MyLangDescr[] = {0x04, 0x03, 0x09, 0x04};
// 厂家信息
const uint8_t MyManuInfo[] = {0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};
// 产品信息
const uint8_t MyProdInfo[] = {0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '9', 0, 'x', 0};

/**********************************************************/
uint8_t        DevConfig, Ready;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;
uint8_t        Report_Value = 0x00;
uint8_t        Idle_Value = 0x00;
uint8_t        USB_SleepStatus = 0x00; /* USB睡眠状态 */

/******** 用户自定义分配端点RAM ****************************************/
__attribute__((aligned(4))) uint8_t EP0_Databuf[64 + 64 + 64]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4))) uint8_t EP1_Databuf[64 + 64];      //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4))) uint8_t EP2_Databuf[64 + 64];      //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4))) uint8_t EP3_Databuf[64 + 64];      //ep3_out(64)+ep3_in(64)
/*********************************************************************
 * @fn      USB_DevTransProcess
 *
 * @brief   USB 传输处理函数
 *
 * @return  none
 */
void USB_DevTransProcess(void)
{
    uint8_t len, chtype;
    uint8_t intflag, errflag = 0;

    intflag = R8_USB_INT_FG;
    if(intflag & RB_UIF_TRANSFER)
    {
        if((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) // 非空闲
        {
            switch(R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
            // 分析操作令牌和端点号
            {
                case UIS_TOKEN_IN: // 端点0 IN
                {
                    switch(SetupReqCode)
                    {
                        case USB_GET_DESCRIPTOR:
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen;
                            memcpy(pEP0_DataBuf, pDescr, len);
                            SetupReqLen -= len;
                            pDescr += len;
                            R8_UEP0_T_LEN = len;
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG;
                            break;
                        case USB_SET_ADDRESS:
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;
                        default:
                            R8_UEP0_T_LEN = 0; // 状态阶段完成
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            break;
                    }
                }
                break;

                case UIS_TOKEN_OUT: // 端点0 OUT
                {
                    len = R8_USB_RX_LEN;
                    if(SetupReqCode == 0x09) // SET_REPORT, 用于键盘LED
                    {
                        // LED灯状态处理代码
                    }
                    if (SetupReqCode == CDC_SET_LINE_CODING) // CDC设置线路状态
                    {
                        if(len >= sizeof(CDC_LineCoding))
                        {
                            memcpy(CDC_LineCoding, pEP0_DataBuf, sizeof(CDC_LineCoding));
                        }
                    }
                }
                break;

                // --- 端点1 (键盘) ---
                case UIS_TOKEN_OUT | 1:
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK) { R8_UEP1_CTRL ^= RB_UEP_R_TOG; DevEP1_OUT_Deal(R8_USB_RX_LEN); }
                    break;
                case UIS_TOKEN_IN  | 1:
                    // 手动翻转TOG位
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                // --- 端点2 (SpaceMouse) ---
                case UIS_TOKEN_OUT | 2:
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK) { R8_UEP2_CTRL ^= RB_UEP_R_TOG; DevEP2_OUT_Deal(R8_USB_RX_LEN); }
                    break;
                case UIS_TOKEN_IN  | 2:
                    // 手动翻转TOG位
                    R8_UEP2_CTRL ^= RB_UEP_T_TOG;
                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;


                // --- 端点3 (CDC 数据) ---
                case UIS_TOKEN_OUT | 3:
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    {
                        // Bulk端点未开启AUTO_TOG，需手动翻转R_TOG
                        R8_UEP3_CTRL ^= RB_UEP_R_TOG;
                        DevEP3_OUT_Deal(R8_USB_RX_LEN);
                    }
                    break;
                case UIS_TOKEN_IN | 3:
                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                // --- 端点4 (CDC 通知) ---
                case UIS_TOKEN_OUT | 4:
                    if(R8_USB_INT_ST & RB_UIS_TOG_OK)
                    {
                        // 硬件已开启AUTO_TOG，软件无需翻转R_TOG
                        DevEP4_OUT_Deal(R8_USB_RX_LEN);
                    }
                    break;
                case UIS_TOKEN_IN | 4:
                    R8_UEP4_CTRL = (R8_UEP4_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                    break;

                default:
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
        if(R8_USB_INT_ST & RB_UIS_SETUP_ACT) // Setup包处理
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            SetupReqLen = pSetupReqPak->wLength;
            SetupReqCode = pSetupReqPak->bRequest;
            chtype = pSetupReqPak->bRequestType;

            len = 0;
            errflag = 0;
            if((pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD)
            {
                /* 非标准请求 */
                /* 其它请求,如类请求，产商请求等 */
                if(pSetupReqPak->bRequestType & 0x40)
                {
                    /* 厂商请求 */
                }
                else if(pSetupReqPak->bRequestType & 0x20)
                {
                    switch(SetupReqCode)
                    {
                        case CDC_GET_LINE_CODING:
                            pDescr = CDC_LineCoding;
                            len = sizeof(CDC_LineCoding);
                            break;
                        case CDC_SET_LINE_CODING:
                            // 接收数据阶段在 UIS_TOKEN_OUT 中处理
                            break;
                        case CDC_SET_CONTROL_LINE_STATE:
                            // 主机通过这个请求来打开/关闭串口，我们只需ACK即可
                            break;
                        case DEF_USB_SET_IDLE: /* 0x0A: SET_IDLE */
                            Idle_Value = EP0_Databuf[3];
                            break; //这个一定要有

                        case DEF_USB_SET_REPORT: /* 0x09: SET_REPORT */
                            break;

                        case DEF_USB_SET_PROTOCOL: /* 0x0B: SET_PROTOCOL */
                            Report_Value = EP0_Databuf[2];
                            break;

                        case DEF_USB_GET_IDLE: /* 0x02: GET_IDLE */
                            EP0_Databuf[0] = Idle_Value;
                            len = 1;
                            break;

                        case DEF_USB_GET_PROTOCOL: /* 0x03: GET_PROTOCOL */
                            EP0_Databuf[0] = Report_Value;
                            len = 1;
                            break;

                        default:
                            errflag = 0xFF;
                    }
                }
            }
            else /* 标准请求 */
            {
                switch(SetupReqCode)
                {
                    case USB_GET_DESCRIPTOR:
                    {
                        switch(((pSetupReqPak->wValue) >> 8))
                        {
                            case USB_DESCR_TYP_DEVICE:
                            {
                                pDescr = MyDevDescr;
                                len = MyDevDescr[0];
                            }
                            break;

                            case USB_DESCR_TYP_CONFIG:
                            {
                                pDescr = MyCfgDescr;
                                len = MyCfgDescr[2] | (MyCfgDescr[3] << 8);
                            }
                            break;

                           case USB_DESCR_TYP_HID:
                                switch((pSetupReqPak->wIndex) & 0xff)
                                {
                                    case 0: // 接口0: 键盘
                                        pDescr = (uint8_t *)(&MyCfgDescr[18]); // 键盘HID描述符的偏移
                                        len = 9;
                                        break;
                                    case 1: // 接口1: SpaceMouse
                                        pDescr = (uint8_t *)(&MyCfgDescr[43]); // SpaceMouse HID描述符的偏移 (9+25+9)
                                        len = 9;
                                        break;
                                    default:
                                        errflag = 0xff;
                                        break;
                                }
                                break;

                           case USB_DESCR_TYP_REPORT:
                            {
                                if(((pSetupReqPak->wIndex) & 0xff) == 0) // 接口0 -> 键盘
                                {
                                    pDescr = KeyRepDesc;
                                    len = sizeof(KeyRepDesc);
                                }
                                else if(((pSetupReqPak->wIndex) & 0xff) == 1) // 接口1 -> SpaceMouse
                                {
                                    pDescr = SpaceMouseRepDesc;
                                    len = sizeof(SpaceMouseRepDesc);
                                }
                                else
                                {
                                     errflag = 0xff; // 不支持的接口
                                }
                                Ready = 1; // 设备就绪
                            }
                            break;

                            case USB_DESCR_TYP_STRING:
                            {
                                switch((pSetupReqPak->wValue) & 0xff)
                                {
                                    case 1:
                                        pDescr = MyManuInfo;
                                        len = MyManuInfo[0];
                                        break;
                                    case 2:
                                        pDescr = MyProdInfo;
                                        len = MyProdInfo[0];
                                        break;
                                    case 0:
                                        pDescr = MyLangDescr;
                                        len = MyLangDescr[0];
                                        break;
                                    default:
                                        errflag = 0xFF; // 不支持的字符串描述符
                                        break;
                                }
                            }
                            break;

                            case 0x06:
                                pDescr = (uint8_t *)(&My_QueDescr[0]);
                                len = sizeof(My_QueDescr);
                                break;

                            case 0x07:
                                memcpy(&USB_FS_OSC_DESC[2], &MyCfgDescr[2], sizeof(MyCfgDescr) - 2);
                                pDescr = (uint8_t *)(&USB_FS_OSC_DESC[0]);
                                len = sizeof(USB_FS_OSC_DESC);
                                break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if(SetupReqLen > len)
                            SetupReqLen = len; //实际需上传总长度
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                        memcpy(pEP0_DataBuf, pDescr, len);
                        pDescr += len;
                    }
                    break;

                    case USB_SET_ADDRESS:
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;
                        break;

                    case USB_GET_CONFIGURATION:
                        pEP0_DataBuf[0] = DevConfig;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;
                        break;

                    case USB_SET_CONFIGURATION:
                        DevConfig = (pSetupReqPak->wValue) & 0xff;
                        break;

                    case USB_CLEAR_FEATURE:
                    {
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // 端点
                        {
                            switch((pSetupReqPak->wIndex) & 0xff)
                            {
                                case 0x83:
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x03:
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x82:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x02:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x81:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // 不支持的端点
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                        {
                            if(pSetupReqPak->wValue == 1)
                            {
                                USB_SleepStatus &= ~0x01;
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                    break;

                    case USB_SET_FEATURE:
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                        {
                            /* 端点 */
                            switch(pSetupReqPak->wIndex)
                            {
                                case 0x83:
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x03:
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x82:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x02:
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x81:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x01:
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    /* 不支持的端点 */
                                    errflag = 0xFF; // 不支持的端点
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                        {
                            if(pSetupReqPak->wValue == 1)
                            {
                                /* 设置睡眠 */
                                USB_SleepStatus |= 0x01;
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                        break;

                    case USB_GET_INTERFACE:
                        pEP0_DataBuf[0] = 0x00;
                        if(SetupReqLen > 1)
                            SetupReqLen = 1;
                        break;

                    case USB_SET_INTERFACE:
                        break;

                    case USB_GET_STATUS:
                        if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                        {
                            /* 端点 */
                            pEP0_DataBuf[0] = 0x00;
                            switch(pSetupReqPak->wIndex)
                            {
                                case 0x83:
                                    if((R8_UEP3_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;

                                case 0x03:
                                    if((R8_UEP3_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;

                                case 0x82:
                                    if((R8_UEP2_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;

                                case 0x02:
                                    if((R8_UEP2_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;

                                case 0x81:
                                    if((R8_UEP1_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;

                                case 0x01:
                                    if((R8_UEP1_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL)
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                            }
                        }
                        else if((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE)
                        {
                            pEP0_DataBuf[0] = 0x00;
                            if(USB_SleepStatus)
                            {
                                pEP0_DataBuf[0] = 0x02;
                            }
                            else
                            {
                                pEP0_DataBuf[0] = 0x00;
                            }
                        }
                        pEP0_DataBuf[1] = 0;
                        if(SetupReqLen >= 2)
                        {
                            SetupReqLen = 2;
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }
            if(errflag == 0xff) // 错误或不支持
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
            }
            else
            {
                if(chtype & 0x80) // 上传
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0; // 下传
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // 默认数据包是DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    }
    else if(intflag & RB_UIF_BUS_RST)
    {
        R8_USB_DEV_AD = 0;
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    }
    else if(intflag & RB_UIF_SUSPEND)
    {
        if(R8_USB_MIS_ST & RB_UMS_SUSPEND)
        {
            ;
        } // 挂起
        else
        {
            ;
        } // 唤醒
        R8_USB_INT_FG = RB_UIF_SUSPEND;
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}

void usb_hid_composite_init(void)
{
    // 设置端点RAM地址
    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    // 调用库函数初始化USB设备
    USB_DeviceInit();
    //第二高优先级，第一高为键盘输入
    PFIC_SetPriority(USB_IRQn,0);
    // 使能USB中断
    PFIC_EnableIRQ(USB_IRQn);
}

void usb_hid_report_keyboard(uint8_t modifier, uint8_t key_code)
{
    uint8_t HIDKey[8] = {0}; // 报告清零

    HIDKey[0] = modifier;   // 第0字节：设置修饰键
    HIDKey[2] = key_code;   // 第2字节：设置普通键

    // 等待端点1就绪
    // (如果您的系统是阻塞式的，可以保留这个循环)
    while((R8_UEP1_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);

    // 将构建好的8字节报告复制到端点缓冲区
    memcpy(pEP1_IN_DataBuf, HIDKey, sizeof(HIDKey));
    
    // 启动USB传输
    DevEP1_IN_Deal(sizeof(HIDKey));
}

void usb_hid_send_report(const uint8_t* report)
{
    // 等待端点1就绪
    // (如果您的系统是阻塞式的，可以保留这个循环)
    while((R8_UEP1_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);
    // 将构建好的8字节报告复制到端点缓冲区
    memcpy(pEP1_IN_DataBuf,report,8);
    
    // 启动USB传输
    DevEP1_IN_Deal(8);
}

void usb_hid_composite_wakeup(void)
{
    // 实现与原 DevWakeup 相同
    R16_PIN_CONFIG &= ~(RB_UDP_PU_EN);
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(2);
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;
    R16_PIN_CONFIG |= RB_UDP_PU_EN;
}

/**
 * @brief 发送6轴数据（平移和旋转）
 * @note  此版本根据当前模式（平移或旋转），发送一个有效的数据报告，
 * 并紧接着发送一个另一类型的零值报告。
 * 这可以主动清除PC主机上对应轴的陈旧状态，避免数据残留。
 * 注意：两次发送之间必须有对端点状态的检查。
 */
void usb_hid_report_axes(int16_t trans[3], int16_t rot[3])
{
    uint8_t report_buf[7];

    if (ModeManager_IsRotationMode())
    {
        // === 旋转模式: 发送有效旋转报告 + 零值平移报告 ===

        // 1. 等待端点就绪，然后发送有效的旋转报告 (Report ID = 2)
        while((R8_UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);
        report_buf[0] = 0x02;
        report_buf[1] = rot[0] & 0xFF;
        report_buf[2] = rot[0] >> 8;
        report_buf[3] = rot[1] & 0xFF;
        report_buf[4] = rot[1] >> 8;
        report_buf[5] = rot[2] & 0xFF;
        report_buf[6] = rot[2] >> 8;
        memcpy(pEP2_IN_DataBuf, report_buf, 7);
        DevEP2_IN_Deal(7);

        // 2. 再次等待端点就绪，然后发送零值的平移报告 (Report ID = 1)
        while((R8_UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);
        report_buf[0] = 0x01;
        memset(&report_buf[1], 0, 6); // 将6个字节的数据位全部清零
        memcpy(pEP2_IN_DataBuf, report_buf, 7);
        DevEP2_IN_Deal(7);
    }
    else
    {
        // === 平移模式: 发送有效平移报告 + 零值旋转报告 ===

        // 1. 等待端点就绪，然后发送有效的平移报告 (Report ID = 1)
        while((R8_UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);
        report_buf[0] = 0x01;
        report_buf[1] = trans[0] & 0xFF;
        report_buf[2] = trans[0] >> 8;
        report_buf[3] = trans[1] & 0xFF;
        report_buf[4] = trans[1] >> 8;
        report_buf[5] = trans[2] & 0xFF;
        report_buf[6] = trans[2] >> 8;
        memcpy(pEP2_IN_DataBuf, report_buf, 7);
        DevEP2_IN_Deal(7);

        // 2. 再次等待端点就绪，然后发送零值的旋转报告 (Report ID = 2)
        while((R8_UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);
        report_buf[0] = 0x02;
        memset(&report_buf[1], 0, 6); // 将6个字节的数据位全部清零
        memcpy(pEP2_IN_DataBuf, report_buf, 7);
        DevEP2_IN_Deal(7);
    }
}

void usb_hid_report_buttons(uint32_t buttons)
{
    uint8_t report_buf[5];

    // 等待端点2就绪
    while((R8_UEP2_CTRL & MASK_UEP_T_RES) != UEP_T_RES_NAK);

    // 发送按键报告 (Report ID = 3)
    report_buf[0] = 0x03;
    report_buf[1] = buttons & 0xFF;
    report_buf[2] = (buttons >> 8) & 0xFF;
    report_buf[3] = (buttons >> 16) & 0xFF;
    report_buf[4] = (buttons >> 24) & 0xFF;
    memcpy(pEP2_IN_DataBuf, report_buf, 5); // 注意是 pEP2_IN_DataBuf
    DevEP2_IN_Deal(5); // 注意是 DevEP2_IN_Deal
}

/*********************************************************************
 * @fn      DevEP1_OUT_Deal
 *
 * @brief   端点1数据处理
 *
 * @return  none
 */
void DevEP1_OUT_Deal(uint8_t l)
{ /* 用户可自定义 */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP1_IN_DataBuf[i] = ~pEP1_OUT_DataBuf[i];
    }
    DevEP1_IN_Deal(l);
}

/*********************************************************************
 * @fn      DevEP2_OUT_Deal
 *
 * @brief   端点2数据处理
 *
 * @return  none
 */
void DevEP2_OUT_Deal(uint8_t l)
{ /* 用户可自定义 */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP2_IN_DataBuf[i] = ~pEP2_OUT_DataBuf[i];
    }
    //DevEP2_IN_Deal(l);
}


/*********************************************************************
 * @fn      DevEP3_OUT_Deal
 * @brief   端点3数据处理 (专用于CDC数据接收)
 *********************************************************************/
void DevEP3_OUT_Deal(uint8_t l)
{
    // 因为包含了 mode_manager.h，现在可以无误地访问这些全局变量
    if (g_cdc_data_received_flag || l == 0 || l >= CDC_RX_BUF_SIZE) {
        return;
    }

    // 快速将USB端点缓冲区的数据拷贝到全局缓冲区
    memcpy(g_cdc_rx_buf, pEP3_OUT_DataBuf, l);
    g_cdc_rx_len = l;

    // 设置标志位，通知主循环有数据需要处理
    g_cdc_data_received_flag = true;
}
/*********************************************************************
 * @fn      DevEP4_OUT_Deal
 *
 * @brief   端点4数据处理
 *
 * @return  none
 */
void DevEP4_OUT_Deal(uint8_t l)
{ /* 用户可自定义 */
    uint8_t i;

    for(i = 0; i < l; i++)
    {
        pEP4_IN_DataBuf[i] = ~pEP4_OUT_DataBuf[i];
    }
    DevEP4_IN_Deal(l);
}


/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB中断函数
 *
 * @return  none
 */
__INTERRUPT
__HIGH_CODE
void USB_IRQHandler(void) /* USB中断服务程序,使用寄存器组1 */
{
    uint8_t intflag = R8_USB_INT_FG;

    if (intflag & RB_UIF_BUS_RST)
    {
        // USB总线复位事件，通常意味着USB被插入并被主机识别
        ModeManager_SetUSBAttached(); // 通知管理器USB已连接
        if (ModeManager_GetCurrentMode() == MODE_BLE) {
            ModeManager_RequestSwitchToUSB();
        }
        USB_DevTransProcess();
    }
    else if (intflag & RB_UIF_SUSPEND)
    {
        // USB挂起事件，可以作为USB拔出的一个判断依据
        R8_USB_INT_FG = RB_UIF_SUSPEND; // 清中断
    }
    else if (intflag & RB_UIF_TRANSFER)
    {
        // 只在USB模式下才处理传输
        if (ModeManager_GetCurrentMode() == MODE_USB || ModeManager_GetCurrentMode() == MODE_NONE) {
            USB_DevTransProcess();
        } else {
             R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    }
    else
    {
        R8_USB_INT_FG = intflag;
    }
}
