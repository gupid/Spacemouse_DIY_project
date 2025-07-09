#include "mode_manager.h"
#include "HAL.h"
#include "hiddev.h"
#include "ble_hid_app.h"
#include "usb_hid_composite.h"
#include "motion_engine.h"
#include "rf_middleware.h"
#include "CONFIG.h" // For WA_UART_Init, PRINT etc.
#include "wchrf.h"
#include "keyboard.h"
#include "HID_usage_keyboard.h"
#include <stdlib.h>

// �ⲿ��������
extern MotionEngine_State mouse_motion;
extern volatile uint8_t g_ahrs_update_flag;
extern uint16_t hidEmuConnHandle;

// ģ���ڲ�ȫ�ֱ���
static volatile OperatingMode_t current_mode = MODE_NONE;
static volatile BOOL is_usb_attached = FALSE;
static volatile BOOL request_switch_to_usb = FALSE;
static volatile BOOL request_switch_to_ble = FALSE;

// �ڲ���������
static void start_rf_mode(void);
static void stop_rf_mode(void);
static void start_ble_mode(void);
static void stop_ble_mode(void);
static void start_usb_mode(void);
static void stop_usb_mode(void);
static void ahrs_task(void);
static void keyboard_task(void);
static void dispatch_keyboard_report(const uint8_t* report);
static void build_keyboard_report(uint8_t* report);
static void LED_mode_init();
static void RED_turn_on();
static void BLUE_turn_on();
static void GREEN_turn_on();
static void WHITE_turn_on();

//RF���
#define RF_MY_ACCESS_ADDRESS   0x4B65794D // "KeyM"��ASCII�룬һ���Զ����ַ
#define RF_MY_CHANNEL          40         // ѡ��һ���ŵ� (�ܿ�Wi-Fi)
#define REPORT_INTERVAL_MS     10          // ���ݱ�������10ms = 100Hz ������

//keyboard relevant
// ���ڴ洢���������а�����ʵʱ״̬ (λͼ)
static uint8_t key_matrix_state[8] = {0}; // ����������8��
// ��־λ����ʾ����״̬�Ѹ��£���Ҫ���ͱ���
static volatile bool hid_keyboard_report_dirty = false;
#define KEY_MATRIX_ROWS  5
#define KEY_MATRIX_COLS  6


//RF�����¼��ص�����
void OnDataReceivedFromDongle(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // ����һ�������͵ļ�������������ա�
    // δ�������ڴ��������Ӧ��ACK����Dongle����������ָ�
}

// --- �����ӿ�ʵ�� ---

// ��ʼ��ģʽ����������������ʱ������ʼģʽ
void ModeManager_Init(void)
{
    WA_GPIO_IT_Init(&waGPIOB,GPIO_Pin_8,GPIO_ITMode_RiseEdge);  //����RF��BLE�л����ж�

    LED_mode_init();   //��ʼ����ɫ�ƹ����IO��

    // 1. ��ʼ��USB��Ϊ�����׼��
    usb_hid_composite_init(); // �������USBΪ�豸ģʽ��ʹ���ж�

    // 2. ��ʱ�ȴ�������PC���������߸�λ�ź�
    mDelaymS(500);

    // 3. ��� is_usb_attached ��־���˱�־Ӧ��USB�ж��б����ã�
    if (is_usb_attached)
    {
        // ���USB�����ӣ�����ʽ����USBģʽ
        PRINT("USB Attached. Starting in USB Mode.\n");
        current_mode = MODE_USB;
        WHITE_turn_on();
        PFIC_EnableIRQ(TMR1_IRQn);
        TMR1_Enable(); // ��������AHRS�Ķ�ʱ��
    }
    else
    {
        // ���USBδ���ӣ������BLEģʽ
        PRINT("USB Not Attached. Starting in BLE Mode.\n");
        BLUE_turn_on();    //����
        stop_usb_mode(); 
        start_ble_mode();
    }
}

// ģʽ����������ѭ������Ӧ�� while(1) �б���������
void ModeManager_MainLoop(void)
{
    // �����ǰ��BLEģʽ������TMOS����
    if (current_mode == MODE_BLE || current_mode == MODE_RF)
    {
        TMOS_SystemProcess();
    }

    // ��鲢����ģʽ�л�����
    if (request_switch_to_usb)
    {
        request_switch_to_usb = FALSE;
        if(current_mode == MODE_BLE|| current_mode == MODE_RF)
        {
            PRINT("Switching from BLE/RF to USB mode.\n");
            WHITE_turn_on();
            stop_ble_mode();
            start_usb_mode();
        }
    }

    if (request_switch_to_ble)
    {
        request_switch_to_ble = FALSE;
        if(current_mode == MODE_USB)
        {
            PRINT("Switching from USB to BLE mode.\n");
            BLUE_turn_on();
            stop_usb_mode();
            start_ble_mode();
        }
    }

    // ִ����̬���������ϱ����� (����USBģʽ��)
    if (g_ahrs_update_flag && current_mode == MODE_USB)
    {
        g_ahrs_update_flag = 0;
        ahrs_task();
    }
    else if (g_ahrs_update_flag && current_mode == MODE_RF) {
        g_ahrs_update_flag = 0;
        ahrs_task();
    }

    //ִ�м�������
    keyboard_task(); // ��鲢�������д�����İ����¼��������ڲ�״̬����

    // �������״̬�Ѹ��£��򹹽������ͱ���
    if (hid_keyboard_report_dirty)
    {
        hid_keyboard_report_dirty = false;
        uint8_t kbd_report[8];
        build_keyboard_report(kbd_report); // ����״̬���󹹽�����
        dispatch_keyboard_report(kbd_report); // �ַ�����
    }
}

// �����л���USBģʽ (���жϵ��ⲿģ�����)
void ModeManager_RequestSwitchToUSB(void)
{
    request_switch_to_usb = TRUE;
}

// �����л���BLEģʽ (���жϵ��ⲿģ�����)
void ModeManager_RequestSwitchToBLE(void)
{
    request_switch_to_ble = TRUE;
}

// ��ȡ��ǰģʽ
OperatingMode_t ModeManager_GetCurrentMode(void)
{
    return current_mode;
}

// ��USB�ж��е��ã������������ӱ�־
void ModeManager_SetUSBAttached(void)
{
    is_usb_attached = TRUE;
}

//��дGPIOB�Ļص�����������BLE��RF���л�����
void WA_GPIOB_EventCallBack(uint32_t gpiopin)
{
    switch (gpiopin) {
        case GPIO_Pin_8:
        //������2.4GHz��2.4GHz�����������߲���
            if(current_mode == MODE_BLE)
            {
                current_mode = MODE_RF;
                GREEN_turn_on();
                stop_ble_mode();
                start_rf_mode();
            }
            else if(current_mode == MODE_RF)
            {
                current_mode = MODE_BLE;
                BLUE_turn_on();
                stop_rf_mode();
                start_ble_mode();
            }
            else {}
            break;
        case GPIO_Pin_7:
        {
            uint8_t temp_key_code;
            if (CH450_Read_Key(&g_wai2c_keyboard, &temp_key_code) == 0)
            {
                // ��ȡ�ɹ���ֱ�ӽ�ԭʼ��ֵ����FIFO
                FIFO_Put(&g_key_fifo, temp_key_code);
            }
            break;
        }
        default:
            break;
    }
}

// --- �ڲ����ܺ��� ---
static void start_rf_mode(void)
{
    CH58x_BLEInit();
    HAL_Init();
    RF_MW_Init(RF_MY_ACCESS_ADDRESS, RF_MY_CHANNEL, OnDataReceivedFromDongle);
    PFIC_EnableIRQ(TMR1_IRQn);
    TMR1_Enable(); // ��������AHRS�Ķ�ʱ��
}

static void stop_rf_mode(void)
{
    RFRole_Stop();
    PFIC_DisableIRQ(TMR1_IRQn);
    TMR1_Disable();
}

static void start_usb_mode(void)
{
    current_mode = MODE_USB;
    usb_hid_composite_init(); // ȷ��USB��ȫ��ʼ��
    DelayMs(500);
    PFIC_EnableIRQ(TMR1_IRQn);
    TMR1_Enable(); // ��������AHRS�Ķ�ʱ��
}

static void stop_usb_mode(void)
{
    PFIC_DisableIRQ(TMR1_IRQn);
    TMR1_Disable();
    
    // �ر�USB�ж�
    //PFIC_DisableIRQ(USB_IRQn);
    // �ر�USB�豸����ֹ��������ģ��γ�
    R8_USB_CTRL = 0;             // ��ֹUSB�豸���ܣ��ر��ڲ���������
    //R16_PIN_CONFIG &= ~RB_PIN_USB_EN; // ��ֹUSB���Ź���
}

static void start_ble_mode(void)
{
    current_mode = MODE_BLE;

    // ��ʼ��BLEЭ��ջ
    CH58x_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    HidDev_Init();
    HidEmu_Init();
}

static void stop_ble_mode(void)
{
    uint8_t advertising_enable = FALSE;
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);

    if (hidEmuConnHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(hidEmuConnHandle);
    }
}

// main.c
void dispatch_mouse_report(uint8_t buttons, int16_t dx, int16_t dy, int8_t wheel) {
    if (current_mode == MODE_USB) {
        usb_hid_composite_report_mouse(buttons, dx, dy, wheel);
    } else if (current_mode == MODE_BLE) {
        if (hidEmuConnHandle != GAP_CONNHANDLE_INIT) {
            // ע�⣺BLE�ĺ�������ֻ����8λλ�ƣ���Ҫ��ת��
            //hidEmuSendMouseReport(buttons, (int8_t)dx, (int8_t)dy);
        }
    } else if (current_mode == MODE_RF) {
        uint16_t rf_data_tx[4]={buttons,dx,dy,wheel};
        RF_MW_SendData((uint8_t*)rf_data_tx,sizeof(rf_data_tx));
    }
}

// AHRS���񣬴�ԭ��ѭ������ȡ����
/**
 * @brief   ����Ч�Ż���AHRS���� (��������������λ�����)
 * @details �˰汾��ǰһ�������Ż��Ļ����ϣ������˸����µ�����ѹե��
 * 1.  **������������**: ���еĳ������� /10, /2�������滻Ϊ���ܼ��ߵġ��˷�+λ�ơ�������������MCU�а���ĳ���ָ�
 * 2.  **λ���Ż�**: ��2���ݴη��ĳ�����ֱ��ʹ��λ��ָ��(>>)�����ǵ����ڼ�����ɵĲ�����
 * 3.  **�ع�LPF�㷨**: ����ͨ�˲��㷨�ع�Ϊ������ʽ��������һ�γ˷�����ʹ�߼���������
 * 4.  **����΢��**: ʹ�� static const ��ȷ����������ʹ洢λ�ã����ṩ�����յ�ǯλ�߼�ʵ��˼·��
 */
static void ahrs_task(void)
{
    // --- 1. ����ɵ����� (����������λ�����Ż��汾) ---
    // ��������
    #define INT_SCALE 100

    // --- ���Ĳ��� (static const�����ڱ������Ż�) ---
    // ������ԭʼֵ 2.0f -> ����ֵ 200
    static const int32_t ANGLE_DEAD_ZONE_I = 2 * INT_SCALE;
    // �������Ƕȣ�ԭʼֵ 35.0f -> ����ֵ 3500
    static const int32_t MAX_INPUT_ANGLE_I = 35 * INT_SCALE;
    // ƽ��ģʽ������ֵ��ԭʼֵ 40.0f -> ����ֵ 4000
    static const int32_t PAN_MODE_THRESHOLD_I = 40 * INT_SCALE;

    // --- �����������Ų��� (��������) ---
    // Yawת����������: * 1.2f -> ��ЧΪ *(12/10)�����ǽ��ó˷�+λ��ʵ�֡�
    static const int32_t YAW_ZOOM_SCALE_NUM = 12;
    // ��ͨ�˲�(LPF)ϵ��: 0.6f -> ����6 (����10)
    static const int32_t LPF_ALPHA_I = 6;
    
    // �����꣬����ִ�и�Ч�ġ�����10������
    // ԭ��: x / 10 �ȼ��� x * (1/10)���ڶ�����ѧ�У�1/10 �� 205 / 2048 = 205 / (2^11)
    // ���� x / 10 �� (x * 205) >> 11�����ֱ���ó�����öࡣ
    #define FAST_DIV_BY_10(x) (((x) * 205) >> 11)

    // --- 2. ��ȡ��̬���� ---
    MotionEngine_Update(&mouse_motion, 0.02f);
    float pitch = mouse_motion.euler_angles[1];
    float roll  = mouse_motion.euler_angles[0];
    float yaw   = mouse_motion.euler_angles[2];
    //PRINT("Pitch: %0.2f, Roll: %0.2f, Yaw: %0.2f\n", pitch, roll, yaw);

    // --- 3. ��ʼ��������� ---
    int32_t trans_x_i = 0, trans_y_i = 0, trans_z_i = 0;
    int32_t rot_x_i = 0, rot_y_i = 0, rot_z_i = 0;

    // --- 4. �����߼���ģʽ�л� ---
    int32_t pitch_i = (int32_t)(pitch * INT_SCALE);
    int32_t roll_i  = (int32_t)(roll * INT_SCALE);

    bool enter_pan_mode = (abs(pitch_i) > PAN_MODE_THRESHOLD_I) || (abs(roll_i) > PAN_MODE_THRESHOLD_I);

    if (enter_pan_mode)
    {
        // **ƽ��(Pan)ģʽ**
        // �Ż�: / 2 �滻Ϊλ�� >> 1
        trans_x_i = roll_i >> 1;
        trans_y_i = -pitch_i >> 1;
    }
    else
    {
        // **Ĭ�ϵġ���ת+���š�ģʽ**
        int32_t yaw_i = (int32_t)(yaw * INT_SCALE);
        rot_x_i = roll_i;
        rot_y_i = pitch_i;
        rot_z_i = yaw_i;
        
        // �Ż�: (val * 12) / 10 �滻Ϊ (val * 12) �ٽ��п��ٳ���10
        trans_z_i = FAST_DIV_BY_10(yaw_i * YAW_ZOOM_SCALE_NUM);
    }

    // --- 5. ͳһ����Ӧ���������������ź��˲� (ȫ��������) ---
    int32_t inputs_i[6] = {trans_x_i, trans_y_i, trans_z_i, rot_x_i, rot_y_i, rot_z_i};
    int16_t output_vals[6] = {0};

    for (int i = 0; i < 6; ++i)
    {
        int32_t val_i = inputs_i[i];

        // Ӧ������
        if (abs(val_i) < ANGLE_DEAD_ZONE_I) {
            val_i = 0;
        } else {
            // ǯλ����������ʹ�ø����յ�д����һЩ�����������Ż��ø���
            // val_i = (val_i > MAX_INPUT_ANGLE_I) ? MAX_INPUT_ANGLE_I : val_i;
            // val_i = (val_i < -MAX_INPUT_ANGLE_I) ? -MAX_INPUT_ANGLE_I : val_i;
            if (val_i > MAX_INPUT_ANGLE_I)       val_i = MAX_INPUT_ANGLE_I;
            else if (val_i < -MAX_INPUT_ANGLE_I) val_i = -MAX_INPUT_ANGLE_I;
        }
        
        // �Ż�: val_i / 10 �滻Ϊ���ٳ�����
        output_vals[i] = (int16_t)FAST_DIV_BY_10(val_i);
    }
    
    // --- ������ͨ�˲��� (������ʽ) ---
    static int16_t filtered_vals[6] = {0};
    for (int i = 0; i < 6; ++i) {
        // �Ż�: �ع�LPF��ʽ��ʹ�ÿ��ٳ���
        // ԭ: filtered = (output * alpha + filtered * (10 - alpha)) / 10
        // ��: filtered += (output - filtered) * alpha / 10
        int32_t error = output_vals[i] - filtered_vals[i];
        filtered_vals[i] += (int16_t)FAST_DIV_BY_10(error * LPF_ALPHA_I);
    }
    
    // --- 6. �������յ�6�����ݱ��� ---
    int16_t final_trans_vals[3] = {filtered_vals[0], filtered_vals[1], filtered_vals[2]};
    int16_t final_rot_vals[3]   = {filtered_vals[3], filtered_vals[4], filtered_vals[5]};
    usb_hid_report_axes(final_trans_vals, final_rot_vals);
}

//���̲���

/**
 * @brief ���ݼ���״̬���󣬹���һ����׼��8�ֽ�HID���̱��档
 * @param report һ��ָ��8�ֽ������ָ�룬���ڴ�Ź����õı��档
 * @note  �˺�����ӳ���߼�������Ŀ����ֵ��.docx�������ݡ�
 */
static void build_keyboard_report(uint8_t* report)
{
    // 1. ��ձ��棬ȷ����һ���ɾ���״̬��ʼ
    memset(report, 0, 8);

    uint8_t key_count = 0; // ���ڼ�¼��������ͨ�������������ܳ���6��
    int row, col;

    // 2. ������������״̬����
    for (row = 0; row < KEY_MATRIX_ROWS; ++row) {
        if (key_matrix_state[row] == 0) continue; // �������û�м����£�ֱ��������һ�������Ч��

        for (col = 0; col < KEY_MATRIX_COLS; ++col) {
            // ���(row, col)λ�õļ��Ƿ񱻰���
            if (key_matrix_state[row] & (1 << col)) {
                
                // 3. ���ģ���(row, col)ӳ�䵽HID Usage ID
                //    ���switch�ṹ�ǡ���ֵ�������ʵ��
                uint8_t hid_code = KEY_NONE;

                uint8_t ch450_addr = row * 8 + col; 

                switch (ch450_addr)
                {
                    // --- ���μ� (ֱ���޸�report[0]��Ȼ������) ---
                    case 0x12: report[0] |= MOD_LEFT_CTRL;  continue; // Ctrl
                    case 0x1A: report[0] |= MOD_LEFT_SHIFT; continue; // Shift
                    case 0x0A: report[0] |= MOD_LEFT_ALT;   continue; // Alt
                    
                    // --- ��ͨ���� (��ȡhid_code������ͳһ���) ---
                    case 0x02: hid_code = KEY_ESCAPE;     break; // Esc
                    case 0x03: hid_code = KEY_DELETE;     break; // Delete
                    case 0x07: hid_code = KEY_BACKSPACE;  break; // Backspace
                    case 0x0F: hid_code = KEY_ENTER;      break; // Enter
                    
                    case 0x1B: hid_code = KEY_1;          break; // 1
                    case 0x1C: hid_code = KEY_2;          break; // 2
                    case 0x1D: hid_code = KEY_3;          break; // 3
                    case 0x13: hid_code = KEY_4;          break; // 4
                    case 0x14: hid_code = KEY_5;          break; // 5
                    case 0x15: hid_code = KEY_6;          break; // 6
                    case 0x0B: hid_code = KEY_7;          break; // 7
                    case 0x0C: hid_code = KEY_8;          break; // 8
                    case 0x0D: hid_code = KEY_9;          break; // 9
                    case 0x24: hid_code = KEY_0;          break; // 0
                    
                    case 0x25: hid_code = KEY_DOT;        break; // . (С����)

                    // --- �깦�ܼ� (��keyboard_task�д����˴�����) ---
                    case 0x04: // Undo
                    case 0x05: // Copy
                    case 0x06: // Paste
                    
                    // --- �Զ��尴�� (ͬ�������ɱ�׼����) ---
                    case 0x0E: // �Զ���1
                    case 0x16: // �Զ���2
                    case 0x1E: // �Զ���3
                    case 0x26: // �Զ���4
                        continue; // ����������䵽������
                }

                // 4. ����ȡ������ͨ��������䵽 report[2] - report[7]
                if (hid_code != KEY_NONE && key_count < 6) {
                    // Ϊ��ֹ�ظ���䣬���Լ�һ�����
                    bool found = false;
                    for(int i = 0; i < key_count; ++i) {
                        if(report[2 + i] == hid_code) {
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        report[2 + key_count] = hid_code;
                        key_count++;
                    }
                }
            }
        }
    }
}

// ͳһ�ļ��̱���ַ�����
static void dispatch_keyboard_report(const uint8_t* report)
{
    if (current_mode == MODE_USB) {
        // ����USB���ͼ��̱���ĺ��� (����Ҫ��usb_hid_composite��ʵ��)
        usb_hid_send_report(report); 
    } else if (current_mode == MODE_BLE) {
        if (hidEmuConnHandle != GAP_CONNHANDLE_INIT) {
            // ����BLE���ͼ��̱���ĺ���
            hidEmuSendKeyReport(report);
        }
    } else if (current_mode == MODE_RF) {
        // ����2.4G RF���ͼ��̱���ĺ���
        RF_MW_SendData((uint8_t*)report, 8);
    }
}

// �������������ڴ����FIFO�����¼�������״̬
static void keyboard_task(void)
{
    KeyEvent_t key_event;
    bool state_changed = false;
    uint8_t kbd_report_temp[8];

    // ѭ������ֱ�����FIFO�е����д������¼�
    while (Keyboard_GetKeyEvent(&key_event))
    {
        if (key_event.pressed) {
            uint8_t ch450_addr_temp = key_event.row*8+key_event.col;
            switch (ch450_addr_temp) {
                case 0x05: // Copy �������� 
                    // --- ��ʼִ�С����ơ����� ---
                    // a. ���������¡����� (Ctrl + C)
                    memset(kbd_report_temp, 0, 8);
                    kbd_report_temp[0] = MOD_LEFT_CTRL;  // ����Ctrl���μ�
                    kbd_report_temp[2] = KEY_C;          // ����C��
                    // b. ���̷ַ������¡�����
                    dispatch_keyboard_report(kbd_report_temp);
                    // c. ������ʱ����������Ӧʱ��
                    mDelaymS(15);
                    // d. �������ַ���ȫ���ͷš�����
                    memset(kbd_report_temp, 0, 8);
                    dispatch_keyboard_report(kbd_report_temp);
                    // e. ��������״̬���£���ɱ����¼�����
                    continue;

                case 0x06: // Paste �������� 
                    // --- ��ʼִ�С�ճ�������� ---
                    memset(kbd_report_temp, 0, 8);
                    kbd_report_temp[0] = MOD_LEFT_CTRL;
                    kbd_report_temp[2] = KEY_V;
                    dispatch_keyboard_report(kbd_report_temp);
                    mDelaymS(15);
                    memset(kbd_report_temp, 0, 8);
                    dispatch_keyboard_report(kbd_report_temp);
                    continue;

                case 0x04: // Undo �������� 
                    // --- ��ʼִ�С����������� ---
                    memset(kbd_report_temp, 0, 8);
                    kbd_report_temp[0] = MOD_LEFT_CTRL;
                    kbd_report_temp[2] = KEY_Z;
                    dispatch_keyboard_report(kbd_report_temp);
                    mDelaymS(15);
                    memset(kbd_report_temp, 0, 8);
                    dispatch_keyboard_report(kbd_report_temp);
                    continue;
            }
        }
        state_changed = true; //���˵��������
        if(key_event.pressed){
            // �����¼�����״̬�����н���Ӧλ��1
            key_matrix_state[key_event.row] |= (1 << key_event.col);
        } else {
            // �ͷ��¼�����״̬�����н���Ӧλ��0
            key_matrix_state[key_event.row] &= ~(1 << key_event.col);
        }
    }
    // ������δ�����κΰ����¼���������Ҫ���ͱ���
    if (state_changed) {
        hid_keyboard_report_dirty = true;
    }
}

//���ڼ��̵�ָʾ�ƿ���
static void LED_mode_init()
{
    GPIOB_ModeCfg(GPIO_Pin_9,GPIO_ModeOut_PP_20mA);
    GPIOB_ModeCfg(GPIO_Pin_17,GPIO_ModeOut_PP_20mA);
    GPIOB_ModeCfg(GPIO_Pin_16,GPIO_ModeOut_PP_20mA);
}

static void RED_turn_on()
{
    GPIOB_ResetBits(GPIO_Pin_17);
    GPIOB_SetBits(GPIO_Pin_16);
    GPIOB_SetBits(GPIO_Pin_9);
}

static void BLUE_turn_on()
{
    GPIOB_ResetBits(GPIO_Pin_9);
    GPIOB_SetBits(GPIO_Pin_17);
    GPIOB_SetBits(GPIO_Pin_16);
}

static void GREEN_turn_on()
{
    GPIOB_ResetBits(GPIO_Pin_16);
    GPIOB_SetBits(GPIO_Pin_9);
    GPIOB_SetBits(GPIO_Pin_17);
}

static void WHITE_turn_on()
{
    GPIOB_ResetBits(GPIO_Pin_17);
    GPIOB_ResetBits(GPIO_Pin_16);
    GPIOB_ResetBits(GPIO_Pin_9);
}