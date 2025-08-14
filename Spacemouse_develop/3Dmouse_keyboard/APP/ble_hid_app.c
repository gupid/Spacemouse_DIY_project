/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidmouse.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙鼠标应用程序，初始化广播连接参数，然后广播，直至连接主机后，定时上传键值
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "CONFIG.h"
#include "battservice.h"
#include "devinfoservice.h"
#include "hiddev.h"
#include "ble_hid_app.h"
#include "hid_composite_service.h"

//用户自定义
#include "motion_engine.h"
#include <stdlib.h>
#include "mode_manager.h"
#include "rf_middleware_tx.h"
extern MotionEngine_State mouse_motion;
extern void dispatch_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);
#define KEYBOARD_REPORT_INTERVAL_MS 50 // 定义键盘回报周期为50ms
static uint8_t last_kbd_report[8] = {0};
//陀螺仪校准
int32_t sample_count_ble = 0;
static float yaw_offset = 0.0f; // 用来存储计算出的零点偏移
static float roll_offset = 0.0f; // 用来存储计算出的零点偏移
static float pitch_offset = 0.0f; // 用来存储计算出的零点偏移
bool is_calibrated_ble = false; // 校准状态标志
/*********************************************************************
 * MACROS
 */

// Selected HID mouse button values
#define MOUSE_BUTTON_NONE                    0x00

// HID mouse input report length
#define HID_MOUSE_IN_RPT_LEN                 4

/*********************************************************************
 * CONSTANTS
 */
// Param update delay
#define START_PARAM_UPDATE_EVT_DELAY         12800

// Param update delay
#define START_PHY_UPDATE_DELAY               1600

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT             60000

// Minimum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    40

// Maximum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    40

// Slave latency to use if parameter update request
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms)
#define DEFAULT_DESIRED_CONN_TIMEOUT         500

// Default passcode
#define DEFAULT_PASSCODE                     0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                 GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                    FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                 TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES              GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL          6

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
uint8_t hidEmuTaskId = INVALID_TASK_ID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GAP Profile - Name attribute for SCAN RSP data
static uint8_t scanRspData[] = {
    0x0B,                           // length of this data (1 + 10个字符 = 11, 十六进制为0x0B)
    GAP_ADTYPE_LOCAL_NAME_COMPLETE, 
    'S', 'p', 'a', 'c', 'e', 'M', 'o', 'u', 's', 'e',
    // connection interval range
    0x05, // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // service UUIDs
    0x05, // length of this data
    GAP_ADTYPE_16BIT_MORE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
    LO_UINT16(BATT_SERV_UUID),
    HI_UINT16(BATT_SERV_UUID),

    // Tx power level
    0x02, // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm
};

// Advertising data
static uint8_t advertData[] = {
    // flags
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // appearance
    0x03, // length of this data
    GAP_ADTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_GENERIC_HID),
    HI_UINT16(GAP_APPEARE_GENERIC_HID)};

// Device name attribute value
static CONST uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "SpaceMouse DIY";

// HID Dev configuration
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT, // Idle timeout
    HID_FEATURE_FLAGS         // HID feature flags
};

uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void    hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
void    hidEmuSendMouseReport(uint8_t buttons, uint8_t X_data, uint8_t Y_data);
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData);
static void    hidEmuEvtCB(uint8_t evt);
static void    hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,
    hidEmuEvtCB,
    NULL,
    hidEmuStateCB};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      HidEmu_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void HidEmu_Init()
{

    hidEmuTaskId = TMOS_ProcessEventRegister(HidEmu_ProcessEvent);

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t initial_advertising_enable = TRUE;

        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);

        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
    }

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *)attDeviceName);

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = DEFAULT_PASSCODE;
        uint8_t  pairMode = DEFAULT_PAIRING_MODE;
        uint8_t  mitm = DEFAULT_MITM_MODE;
        uint8_t  ioCap = DEFAULT_IO_CAPABILITIES;
        uint8_t  bonding = DEFAULT_BONDING_MODE;
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }

    // Setup Battery Characteristic Values
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL;
        Batt_SetParameter(BATT_PARAM_CRITICAL_LEVEL, sizeof(uint8_t), &critical);
    }

    // Set up HID keyboard service
    //Hid_AddService();
    HidComposite_AddService();

    // Register for HID Dev callback
    HidDev_Register(&hidEmuCfg, &hidEmuHidCBs);

    // Setup a delayed profile startup
    tmos_set_event(hidEmuTaskId, START_DEVICE_EVT);
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t HidEmu_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(hidEmuTaskId)) != NULL)
        {
            hidEmu_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);

            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & START_DEVICE_EVT)
    {
        return (events ^ START_DEVICE_EVT);
    }

    if(events & START_PARAM_UPDATE_EVT)
    {
        // Send connect param update request
        GAPRole_PeripheralConnParamUpdateReq(hidEmuConnHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             hidEmuTaskId);

        return (events ^ START_PARAM_UPDATE_EVT);
    }

    if(events & START_PHY_UPDATE_EVT)
    {
        // start phy update
        PRINT("Send Phy Update %x...\n", GAPRole_UpdatePHY(hidEmuConnHandle, 0, 
                    GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }

    if(events & START_REPORT_EVT)
    {
        ahrs_task();
        tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 50);
        return (events ^ START_REPORT_EVT);
    }
    return 0;
}

/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmu_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        default:
            break;
    }
}

/*********************************************************************
 * @brief   Build and send a HID keyboard report over BLE.
 * @param   report - a pointer to the 8-byte keyboard report.
 * @return  none
 */
void hidEmuSendKeyReport(const uint8_t *report)
{
    // 调用底层函数，使用键盘的报告ID (HID_RPT_ID_KEY_IN) 发送数据
    // 调用底层函数，并检查返回值
    uint8_t status = HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                                   8, (uint8_t*)report);

    // 如果发送失败 (没有可用缓冲区)
    if (status == MSG_BUFFER_NOT_AVAIL)
    {
        // 在这里处理失败情况
        // 简单的处理方式是稍后重试，例如设置一个标志位，在下一个事件循环中再次发送
        // 也可以在此打印一条错误信息，方便调试
        PRINT("Keyboard report dropped, no buffer available.\n");
    }
}

/**
 * @brief 通过BLE发送SpaceMouse的6轴数据报告（升级版）
 * @note  此版本根据当前模式（平移或旋转），发送一个有效的数据报告，
 * 并紧接着发送一个另一类型的零值报告。
 * 这可以主动清除主机上对应轴的陈旧状态，是比交替发送更稳健的策略。
 */
void hidEmuSendSpaceMouseReport(int16_t trans_x, int16_t trans_y, int16_t trans_z,
                                int16_t rot_x, int16_t rot_y, int16_t rot_z)
{
    uint8_t report_buf[6];

    if (ModeManager_IsRotationMode())
    {
        // === 旋转模式: 连续发送 有效旋转 + 零值平移 ===
        // 1. 发送有效的旋转报告
        report_buf[0] = LO_UINT16(rot_x);
        report_buf[1] = HI_UINT16(rot_x);
        report_buf[2] = LO_UINT16(rot_y);
        report_buf[3] = HI_UINT16(rot_y);
        report_buf[4] = LO_UINT16(rot_z);
        report_buf[5] = HI_UINT16(rot_z);
        HidDev_Report(HID_RPT_ID_SPACE_ROT_IN, HID_REPORT_TYPE_INPUT, 6, report_buf);

        // 2. 立即发送零值的平移报告
        memset(report_buf, 0, 6);
        HidDev_Report(HID_RPT_ID_SPACE_TRANS_IN, HID_REPORT_TYPE_INPUT, 6, report_buf);
    }
    else
    {
        // === 平移模式: 连续发送 有效平移 + 零值旋转 ===
        // 1. 发送有效的平移报告
        report_buf[0] = LO_UINT16(trans_x);
        report_buf[1] = HI_UINT16(trans_x);
        report_buf[2] = LO_UINT16(trans_y);
        report_buf[3] = HI_UINT16(trans_y);
        report_buf[4] = LO_UINT16(trans_z);
        report_buf[5] = HI_UINT16(trans_z);
        HidDev_Report(HID_RPT_ID_SPACE_TRANS_IN, HID_REPORT_TYPE_INPUT, 6, report_buf);

        // 2. 立即发送零值的旋转报告
        memset(report_buf, 0, 6);
        HidDev_Report(HID_RPT_ID_SPACE_ROT_IN, HID_REPORT_TYPE_INPUT, 6, report_buf);
    }
}
/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP state change callback.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void hidEmuStateCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState & GAPROLE_STATE_ADV_MASK)
    {
        case GAPROLE_STARTED:
        {
            uint8_t ownAddr[6];
            GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddr);
            GAP_ConfigDeviceAddr(ADDRTYPE_STATIC, ownAddr);
            PRINT("Initialized..\n");
        }
        break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

                // get connection handle
                hidEmuConnHandle = event->connectionHandle;
                tmos_start_task(hidEmuTaskId, START_PARAM_UPDATE_EVT, START_PARAM_UPDATE_EVT_DELAY);
                PRINT("Connected..\n");
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
                // 在连接断开时，重置全局连接句柄
                hidEmuConnHandle = GAP_CONNHANDLE_INIT;
                // 停止所有周期性回报任务
                tmos_stop_task(hidEmuTaskId, START_REPORT_EVT);
                // =======================================================
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                PRINT("Advertising timeout..\n");
            }
            // Enable advertising
            {
                uint8_t initial_advertising_enable = TRUE;
                // Set the GAP Role Parameters
                GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
            }
            break;

        case GAPROLE_ERROR:
            PRINT("Error %x ..\n", pEvent->gap.opcode);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      hidEmuRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8_t hidEmuRptCB(uint8_t id, uint8_t type, uint16_t uuid,
                           uint8_t oper, uint16_t *pLen, uint8_t *pData)
{
    uint8_t status = SUCCESS;

    // write
    if(oper == HID_DEV_OPER_WRITE)
    {
        status = Hid_SetParameter(id, type, uuid, *pLen, pData);
    }
    // read
    else if(oper == HID_DEV_OPER_READ)
    {
        status = Hid_GetParameter(id, type, uuid, pLen, pData);
    }
    // notifications enabled
    else if(oper == HID_DEV_OPER_ENABLE)
    {
        tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 500);
    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuEvtCB(uint8_t evt)
{
    // process enter/exit suspend or enter/exit boot mode
    return;
}

/*********************************************************************
*********************************************************************/
