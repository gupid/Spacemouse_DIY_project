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

// 外部变量声明
extern MotionEngine_State mouse_motion;
extern volatile uint8_t g_ahrs_update_flag;
extern uint16_t hidEmuConnHandle;

// 模块内部全局变量
static volatile OperatingMode_t current_mode = MODE_NONE;
static volatile BOOL is_usb_attached = FALSE;
static volatile BOOL request_switch_to_usb = FALSE;
static volatile BOOL request_switch_to_ble = FALSE;

// 内部函数声明
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

//RF相关
#define RF_MY_ACCESS_ADDRESS   0x4B65794D // "KeyM"的ASCII码，一个自定义地址
#define RF_MY_CHANNEL          40         // 选择一个信道 (避开Wi-Fi)
#define REPORT_INTERVAL_MS     10          // 数据报告间隔，10ms = 100Hz 报告率

//keyboard relevant
// 用于存储键盘上所有按键的实时状态 (位图)
static uint8_t key_matrix_state[8] = {0}; // 假设键盘最多8行
// 标志位，表示键盘状态已更新，需要发送报告
static volatile bool hid_keyboard_report_dirty = false;
#define KEY_MATRIX_ROWS  5
#define KEY_MATRIX_COLS  6


//RF接收事件回调函数
void OnDataReceivedFromDongle(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // 对于一个纯发送的键鼠，这里可以留空。
    // 未来可用于处理配对响应、ACK、或Dongle发来的配置指令。
}

// --- 公共接口实现 ---

// 初始化模式管理器，并在启动时决定初始模式
void ModeManager_Init(void)
{
    WA_GPIO_IT_Init(&waGPIOB,GPIO_Pin_8,GPIO_ITMode_RiseEdge);  //设置RF和BLE切换的中断

    LED_mode_init();   //初始化颜色灯管理的IO口

    // 1. 初始化USB，为检测做准备
    usb_hid_composite_init(); // 这会配置USB为设备模式并使能中断

    // 2. 延时等待主机（PC）发出总线复位信号
    mDelaymS(500);

    // 3. 检查 is_usb_attached 标志（此标志应在USB中断中被设置）
    if (is_usb_attached)
    {
        // 如果USB已连接，则正式进入USB模式
        PRINT("USB Attached. Starting in USB Mode.\n");
        current_mode = MODE_USB;
        WHITE_turn_on();
        PFIC_EnableIRQ(TMR1_IRQn);
        TMR1_Enable(); // 启动用于AHRS的定时器
    }
    else
    {
        // 如果USB未连接，则进入BLE模式
        PRINT("USB Not Attached. Starting in BLE Mode.\n");
        BLUE_turn_on();    //蓝灯
        stop_usb_mode(); 
        start_ble_mode();
    }
}

// 模式管理器的主循环任务，应在 while(1) 中被持续调用
void ModeManager_MainLoop(void)
{
    // 如果当前是BLE模式，运行TMOS进程
    if (current_mode == MODE_BLE || current_mode == MODE_RF)
    {
        TMOS_SystemProcess();
    }

    // 检查并处理模式切换请求
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

    // 执行姿态解算和鼠标上报任务 (仅在USB模式下)
    if (g_ahrs_update_flag && current_mode == MODE_USB)
    {
        g_ahrs_update_flag = 0;
        ahrs_task();
    }
    else if (g_ahrs_update_flag && current_mode == MODE_RF) {
        g_ahrs_update_flag = 0;
        ahrs_task();
    }

    //执行键盘任务
    keyboard_task(); // 检查并处理所有待处理的按键事件，更新内部状态矩阵

    // 如果键盘状态已更新，则构建并发送报告
    if (hid_keyboard_report_dirty)
    {
        hid_keyboard_report_dirty = false;
        uint8_t kbd_report[8];
        build_keyboard_report(kbd_report); // 根据状态矩阵构建报告
        dispatch_keyboard_report(kbd_report); // 分发报告
    }
}

// 请求切换到USB模式 (供中断等外部模块调用)
void ModeManager_RequestSwitchToUSB(void)
{
    request_switch_to_usb = TRUE;
}

// 请求切换到BLE模式 (供中断等外部模块调用)
void ModeManager_RequestSwitchToBLE(void)
{
    request_switch_to_ble = TRUE;
}

// 获取当前模式
OperatingMode_t ModeManager_GetCurrentMode(void)
{
    return current_mode;
}

// 在USB中断中调用，用于设置连接标志
void ModeManager_SetUSBAttached(void)
{
    is_usb_attached = TRUE;
}

//重写GPIOB的回调函数，处理BLE和RF的切换请求
void WA_GPIOB_EventCallBack(uint32_t gpiopin)
{
    switch (gpiopin) {
        case GPIO_Pin_8:
        //蓝牙切2.4GHz、2.4GHz切蓝牙；有线不切
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
                // 读取成功后，直接将原始键值放入FIFO
                FIFO_Put(&g_key_fifo, temp_key_code);
            }
            break;
        }
        default:
            break;
    }
}

// --- 内部功能函数 ---
static void start_rf_mode(void)
{
    CH58x_BLEInit();
    HAL_Init();
    RF_MW_Init(RF_MY_ACCESS_ADDRESS, RF_MY_CHANNEL, OnDataReceivedFromDongle);
    PFIC_EnableIRQ(TMR1_IRQn);
    TMR1_Enable(); // 启动用于AHRS的定时器
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
    usb_hid_composite_init(); // 确保USB完全初始化
    DelayMs(500);
    PFIC_EnableIRQ(TMR1_IRQn);
    TMR1_Enable(); // 启动用于AHRS的定时器
}

static void stop_usb_mode(void)
{
    PFIC_DisableIRQ(TMR1_IRQn);
    TMR1_Disable();
    
    // 关闭USB中断
    //PFIC_DisableIRQ(USB_IRQn);
    // 关闭USB设备，禁止上拉，以模拟拔出
    R8_USB_CTRL = 0;             // 禁止USB设备功能，关闭内部上拉电阻
    //R16_PIN_CONFIG &= ~RB_PIN_USB_EN; // 禁止USB引脚功能
}

static void start_ble_mode(void)
{
    current_mode = MODE_BLE;

    // 初始化BLE协议栈
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
            // 注意：BLE的函数可能只接受8位位移，需要做转换
            //hidEmuSendMouseReport(buttons, (int8_t)dx, (int8_t)dy);
        }
    } else if (current_mode == MODE_RF) {
        uint16_t rf_data_tx[4]={buttons,dx,dy,wheel};
        RF_MW_SendData((uint8_t*)rf_data_tx,sizeof(rf_data_tx));
    }
}

// AHRS任务，从原主循环中提取出来
/**
 * @brief   超高效优化的AHRS任务 (极致整数计算与位运算版)
 * @details 此版本在前一版整数优化的基础上，进行了更极致的性能压榨：
 * 1.  **消除整数除法**: 所有的除法（如 /10, /2）都被替换为性能极高的“乘法+位移”操作，避免了MCU中昂贵的除法指令。
 * 2.  **位移优化**: 对2的幂次方的除法，直接使用位移指令(>>)，这是单周期即可完成的操作。
 * 3.  **重构LPF算法**: 将低通滤波算法重构为增量形式，减少了一次乘法，并使逻辑更清晰。
 * 4.  **代码微调**: 使用 static const 明确常量作用域和存储位置，并提供更紧凑的钳位逻辑实现思路。
 */
static void ahrs_task(void)
{
    // --- 1. 定义可调参数 (定点整数与位运算优化版本) ---
    // 缩放因子
    #define INT_SCALE 100

    // --- 核心参数 (static const有助于编译器优化) ---
    // 死区：原始值 2.0f -> 整数值 200
    static const int32_t ANGLE_DEAD_ZONE_I = 2 * INT_SCALE;
    // 最大输入角度：原始值 35.0f -> 整数值 3500
    static const int32_t MAX_INPUT_ANGLE_I = 35 * INT_SCALE;
    // 平移模式触发阈值：原始值 40.0f -> 整数值 4000
    static const int32_t PAN_MODE_THRESHOLD_I = 40 * INT_SCALE;

    // --- 灵敏度与缩放参数 (消除除法) ---
    // Yaw转缩放灵敏度: * 1.2f -> 等效为 *(12/10)。我们将用乘法+位移实现。
    static const int32_t YAW_ZOOM_SCALE_NUM = 12;
    // 低通滤波(LPF)系数: 0.6f -> 整数6 (满分10)
    static const int32_t LPF_ALPHA_I = 6;
    
    // 辅助宏，用于执行高效的“除以10”操作
    // 原理: x / 10 等价于 x * (1/10)。在定点数学中，1/10 ≈ 205 / 2048 = 205 / (2^11)
    // 所以 x / 10 ≈ (x * 205) >> 11。这比直接用除法快得多。
    #define FAST_DIV_BY_10(x) (((x) * 205) >> 11)

    // --- 2. 获取姿态数据 ---
    MotionEngine_Update(&mouse_motion, 0.02f);
    float pitch = mouse_motion.euler_angles[1];
    float roll  = mouse_motion.euler_angles[0];
    float yaw   = mouse_motion.euler_angles[2];
    //PRINT("Pitch: %0.2f, Roll: %0.2f, Yaw: %0.2f\n", pitch, roll, yaw);

    // --- 3. 初始化输出变量 ---
    int32_t trans_x_i = 0, trans_y_i = 0, trans_z_i = 0;
    int32_t rot_x_i = 0, rot_y_i = 0, rot_z_i = 0;

    // --- 4. 核心逻辑：模式切换 ---
    int32_t pitch_i = (int32_t)(pitch * INT_SCALE);
    int32_t roll_i  = (int32_t)(roll * INT_SCALE);

    bool enter_pan_mode = (abs(pitch_i) > PAN_MODE_THRESHOLD_I) || (abs(roll_i) > PAN_MODE_THRESHOLD_I);

    if (enter_pan_mode)
    {
        // **平移(Pan)模式**
        // 优化: / 2 替换为位移 >> 1
        trans_x_i = roll_i >> 1;
        trans_y_i = -pitch_i >> 1;
    }
    else
    {
        // **默认的“旋转+缩放”模式**
        int32_t yaw_i = (int32_t)(yaw * INT_SCALE);
        rot_x_i = roll_i;
        rot_y_i = pitch_i;
        rot_z_i = yaw_i;
        
        // 优化: (val * 12) / 10 替换为 (val * 12) 再进行快速除以10
        trans_z_i = FAST_DIV_BY_10(yaw_i * YAW_ZOOM_SCALE_NUM);
    }

    // --- 5. 统一处理：应用死区、线性缩放和滤波 (全整数运算) ---
    int32_t inputs_i[6] = {trans_x_i, trans_y_i, trans_z_i, rot_x_i, rot_y_i, rot_z_i};
    int16_t output_vals[6] = {0};

    for (int i = 0; i < 6; ++i)
    {
        int32_t val_i = inputs_i[i];

        // 应用死区
        if (abs(val_i) < ANGLE_DEAD_ZONE_I) {
            val_i = 0;
        } else {
            // 钳位操作，可以使用更紧凑的写法，一些编译器可能优化得更好
            // val_i = (val_i > MAX_INPUT_ANGLE_I) ? MAX_INPUT_ANGLE_I : val_i;
            // val_i = (val_i < -MAX_INPUT_ANGLE_I) ? -MAX_INPUT_ANGLE_I : val_i;
            if (val_i > MAX_INPUT_ANGLE_I)       val_i = MAX_INPUT_ANGLE_I;
            else if (val_i < -MAX_INPUT_ANGLE_I) val_i = -MAX_INPUT_ANGLE_I;
        }
        
        // 优化: val_i / 10 替换为快速除法宏
        output_vals[i] = (int16_t)FAST_DIV_BY_10(val_i);
    }
    
    // --- 整数低通滤波器 (增量形式) ---
    static int16_t filtered_vals[6] = {0};
    for (int i = 0; i < 6; ++i) {
        // 优化: 重构LPF公式并使用快速除法
        // 原: filtered = (output * alpha + filtered * (10 - alpha)) / 10
        // 新: filtered += (output - filtered) * alpha / 10
        int32_t error = output_vals[i] - filtered_vals[i];
        filtered_vals[i] += (int16_t)FAST_DIV_BY_10(error * LPF_ALPHA_I);
    }
    
    // --- 6. 发送最终的6轴数据报告 ---
    int16_t final_trans_vals[3] = {filtered_vals[0], filtered_vals[1], filtered_vals[2]};
    int16_t final_rot_vals[3]   = {filtered_vals[3], filtered_vals[4], filtered_vals[5]};
    usb_hid_report_axes(final_trans_vals, final_rot_vals);
}

//键盘部分

/**
 * @brief 根据键盘状态矩阵，构建一个标准的8字节HID键盘报告。
 * @param report 一个指向8字节数组的指针，用于存放构建好的报告。
 * @note  此函数的映射逻辑基于项目“键值表.docx”的内容。
 */
static void build_keyboard_report(uint8_t* report)
{
    // 1. 清空报告，确保从一个干净的状态开始
    memset(report, 0, 8);

    uint8_t key_count = 0; // 用于记录已填充的普通按键数量，不能超过6个
    int row, col;

    // 2. 遍历整个键盘状态矩阵
    for (row = 0; row < KEY_MATRIX_ROWS; ++row) {
        if (key_matrix_state[row] == 0) continue; // 如果此行没有键按下，直接跳到下一行以提高效率

        for (col = 0; col < KEY_MATRIX_COLS; ++col) {
            // 检查(row, col)位置的键是否被按下
            if (key_matrix_state[row] & (1 << col)) {
                
                // 3. 核心：将(row, col)映射到HID Usage ID
                //    这个switch结构是“键值表”的软件实现
                uint8_t hid_code = KEY_NONE;

                uint8_t ch450_addr = row * 8 + col; 

                switch (ch450_addr)
                {
                    // --- 修饰键 (直接修改report[0]，然后跳过) ---
                    case 0x12: report[0] |= MOD_LEFT_CTRL;  continue; // Ctrl
                    case 0x1A: report[0] |= MOD_LEFT_SHIFT; continue; // Shift
                    case 0x0A: report[0] |= MOD_LEFT_ALT;   continue; // Alt
                    
                    // --- 普通按键 (获取hid_code，后续统一填充) ---
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
                    
                    case 0x25: hid_code = KEY_DOT;        break; // . (小数点)

                    // --- 宏功能键 (在keyboard_task中处理，此处忽略) ---
                    case 0x04: // Undo
                    case 0x05: // Copy
                    case 0x06: // Paste
                    
                    // --- 自定义按键 (同样不生成标准报告) ---
                    case 0x0E: // 自定义1
                    case 0x16: // 自定义2
                    case 0x1E: // 自定义3
                    case 0x26: // 自定义4
                        continue; // 跳过，不填充到报告中
                }

                // 4. 将获取到的普通按键码填充到 report[2] - report[7]
                if (hid_code != KEY_NONE && key_count < 6) {
                    // 为防止重复填充，可以加一个检查
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

// 统一的键盘报告分发函数
static void dispatch_keyboard_report(const uint8_t* report)
{
    if (current_mode == MODE_USB) {
        // 调用USB发送键盘报告的函数 (您需要在usb_hid_composite中实现)
        usb_hid_send_report(report); 
    } else if (current_mode == MODE_BLE) {
        if (hidEmuConnHandle != GAP_CONNHANDLE_INIT) {
            // 调用BLE发送键盘报告的函数
            hidEmuSendKeyReport(report);
        }
    } else if (current_mode == MODE_RF) {
        // 调用2.4G RF发送键盘报告的函数
        RF_MW_SendData((uint8_t*)report, 8);
    }
}

// 键盘主任务，用于处理从FIFO来的事件并更新状态
static void keyboard_task(void)
{
    KeyEvent_t key_event;
    bool state_changed = false;
    uint8_t kbd_report_temp[8];

    // 循环处理，直到清空FIFO中的所有待处理事件
    while (Keyboard_GetKeyEvent(&key_event))
    {
        if (key_event.pressed) {
            uint8_t ch450_addr_temp = key_event.row*8+key_event.col;
            switch (ch450_addr_temp) {
                case 0x05: // Copy 键被按下 
                    // --- 开始执行“复制”序列 ---
                    // a. 构建“按下”报告 (Ctrl + C)
                    memset(kbd_report_temp, 0, 8);
                    kbd_report_temp[0] = MOD_LEFT_CTRL;  // 设置Ctrl修饰键
                    kbd_report_temp[2] = KEY_C;          // 设置C键
                    // b. 立刻分发“按下”报告
                    dispatch_keyboard_report(kbd_report_temp);
                    // c. 短暂延时，给主机响应时间
                    mDelaymS(15);
                    // d. 构建并分发“全部释放”报告
                    memset(kbd_report_temp, 0, 8);
                    dispatch_keyboard_report(kbd_report_temp);
                    // e. 跳过后续状态更新，完成本次事件处理
                    continue;

                case 0x06: // Paste 键被按下 
                    // --- 开始执行“粘贴”序列 ---
                    memset(kbd_report_temp, 0, 8);
                    kbd_report_temp[0] = MOD_LEFT_CTRL;
                    kbd_report_temp[2] = KEY_V;
                    dispatch_keyboard_report(kbd_report_temp);
                    mDelaymS(15);
                    memset(kbd_report_temp, 0, 8);
                    dispatch_keyboard_report(kbd_report_temp);
                    continue;

                case 0x04: // Undo 键被按下 
                    // --- 开始执行“撤销”序列 ---
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
        state_changed = true; //过滤掉宏命令键
        if(key_event.pressed){
            // 按下事件：在状态矩阵中将对应位置1
            key_matrix_state[key_event.row] |= (1 << key_event.col);
        } else {
            // 释放事件：在状态矩阵中将对应位置0
            key_matrix_state[key_event.row] &= ~(1 << key_event.col);
        }
    }
    // 如果本次处理过任何按键事件，则标记需要发送报告
    if (state_changed) {
        hid_keyboard_report_dirty = true;
    }
}

//用于键盘的指示灯控制
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