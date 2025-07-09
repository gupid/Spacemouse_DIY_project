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
extern MotionEngine_State mouse_motion;
extern void dispatch_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);
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
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    8

// Maximum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    8

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
        // --- 1. 定义参数 ---
        // 死区角度：小于这个角度的倾斜将被忽略，防止漂移
        const float ANGLE_DEAD_ZONE = 5.0f; // 单位：度
        // 最大输入角度：当设备倾斜到这个角度时，达到最大输出速度
        const float MAX_INPUT_ANGLE = 45.0f; // 单位：度
        // --- 2. 获取姿态数据 ---
        MotionEngine_Update(&mouse_motion, 0.1f);

        float pitch = mouse_motion.euler_angles[1];
        float roll = mouse_motion.euler_angles[0];
        float yaw = mouse_motion.euler_angles[2];

        // --- 3. 应用死区 ---
        if (fabs(pitch) < ANGLE_DEAD_ZONE) pitch = 0.0f;
        if (fabs(roll) < ANGLE_DEAD_ZONE)  roll = 0.0f;
        if (fabs(yaw) < ANGLE_DEAD_ZONE)   yaw = 0.0f;

        // --- 4. 建立简单映射关系 ---
        // 旋转：将 Pitch, Roll, Yaw 直接映射到旋转轴
        float rot_x_f = pitch; // 前后倾斜 -> 围绕X轴旋转
        float rot_y_f = roll;  // 左右倾斜 -> 围绕Y轴旋转
        float rot_z_f = yaw;   // 水平扭动 -> 围绕Z轴旋转

        // 平移：为了演示，我们复用一些轴
        float trans_x_f = roll;  // 左右倾斜 -> 同时也左右平移
        float trans_y_f = 0.0f;  // 暂时不用Y轴平移
        float trans_z_f = -pitch; // 前后倾斜 -> 同时也前后平移(Zoom)，加负号更直观

        // --- 5. 数据缩放 (-350 ~ +350) 和类型转换 ---
        // 这里我们进行线性缩放，并确保值不会超出范围
        int16_t trans_x = (int16_t) fmaxf(-350.0f, fminf(350.0f, (trans_x_f / MAX_INPUT_ANGLE) * 350.0f));
        int16_t trans_y = (int16_t) fmaxf(-350.0f, fminf(350.0f, (trans_y_f / MAX_INPUT_ANGLE) * 350.0f));
        int16_t trans_z = (int16_t) fmaxf(-350.0f, fminf(350.0f, (trans_z_f / MAX_INPUT_ANGLE) * 350.0f));
        int16_t rot_x   = (int16_t) fmaxf(-350.0f, fminf(350.0f, (rot_x_f / MAX_INPUT_ANGLE) * 350.0f));
        int16_t rot_y   = (int16_t) fmaxf(-350.0f, fminf(350.0f, (rot_y_f / MAX_INPUT_ANGLE) * 350.0f));
        int16_t rot_z   = (int16_t) fmaxf(-350.0f, fminf(350.0f, (rot_z_f / MAX_INPUT_ANGLE) * 350.0f));
        hidEmuSendSpaceMouseReport(trans_x, trans_y, trans_z, rot_x, rot_y, rot_z);
        tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 100);
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
    HidDev_Report(HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                  8, (uint8_t*)report);
}

/*********************************************************************
 * @brief   构建并发送SpaceMouse的HID报告 (平移和旋转)
 * @param   trans_x, trans_y, trans_z - 平移轴数据 (-350 ~ 350)
 * @param   rot_x, rot_y, rot_z - 旋转轴数据 (-350 ~ 350)
 * @return  none
 */
void hidEmuSendSpaceMouseReport(int16_t trans_x, int16_t trans_y, int16_t trans_z,
                                  int16_t rot_x, int16_t rot_y, int16_t rot_z)
{
    // 1. 准备并发送平移报告 (ID 2)
    uint8_t trans_buf[6];
    // 将16位的整数拆分为两个8位的字节 (小端模式)
    trans_buf[0] = LO_UINT16(trans_x);
    trans_buf[1] = HI_UINT16(trans_x);
    trans_buf[2] = LO_UINT16(trans_y);
    trans_buf[3] = HI_UINT16(trans_y);
    trans_buf[4] = LO_UINT16(trans_z);
    trans_buf[5] = HI_UINT16(trans_z);

    // 使用我们之前定义的报告ID 2来发送平移数据
    HidDev_Report(HID_RPT_ID_SPACE_TRANS_IN, HID_REPORT_TYPE_INPUT, 6, trans_buf);

    // 2. 准备并发送旋转报告 (ID 3)
    uint8_t rot_buf[6];
    // 将16位的整数拆分为两个8位的字节 (小端模式)
    rot_buf[0] = LO_UINT16(rot_x);
    rot_buf[1] = HI_UINT16(rot_x);
    rot_buf[2] = LO_UINT16(rot_y);
    rot_buf[3] = HI_UINT16(rot_y);
    rot_buf[4] = LO_UINT16(rot_z);
    rot_buf[5] = HI_UINT16(rot_z);

    // 使用我们之前定义的报告ID 3来发送旋转数据
    HidDev_Report(HID_RPT_ID_SPACE_ROT_IN, HID_REPORT_TYPE_INPUT, 6, rot_buf);
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
                // ==================== 新增的核心逻辑 ====================
                // 在连接断开时，重置全局连接句柄
                hidEmuConnHandle = GAP_CONNHANDLE_INIT; //
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
