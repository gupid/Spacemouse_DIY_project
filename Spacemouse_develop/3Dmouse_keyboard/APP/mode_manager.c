#include "mode_manager.h"
#include "HAL.h"
#include "hiddev.h"
#include "ble_hid_app.h"
#include "usb_hid_composite.h"
#include "motion_engine.h"
#include "rf_middleware_tx.h"
#include "CONFIG.h" // For WA_UART_Init, PRINT etc.
#include "wchrf.h"
#include "keyboard.h"
#include "HID_usage_keyboard.h"
#include <stdlib.h>

// 外部变量声明
extern MotionEngine_State mouse_motion;
extern volatile uint8_t g_ahrs_update_flag;
extern uint16_t hidEmuConnHandle;
extern void change_rot_tran_report_ble();
// 模块内部全局变量
static volatile OperatingMode_t current_mode = MODE_NONE;
static volatile BOOL is_usb_attached = FALSE;
static volatile BOOL request_switch_to_usb = FALSE;
static volatile BOOL request_switch_to_ble = FALSE;
static volatile bool g_keyboard_irq_pending = FALSE;
static volatile bool is_waiting_for_release = false; // 标志位，表示是否在等待某个按键释放
static volatile uint8_t last_key_scancode = 0;       // 保存上一个被按下按键的扫描码 (不含状态位)
//陀螺仪arhs-task校准
static int32_t sample_count = 0;
static float yaw_offset = 0.0f; // 用来存储计算出的零点偏移
static float roll_offset = 0.0f; // 用来存储计算出的零点偏移
static float pitch_offset = 0.0f; // 用来存储计算出的零点偏移
static bool is_calibrated = false; // 校准状态标志
// 用于统一管理SpaceMouse旋转/平移模式的全局状态
static bool g_is_rotation_mode = false; 


// 内部函数声明
static void start_rf_mode(void);
static void stop_rf_mode(void);
static void start_ble_mode(void);
static void stop_ble_mode(void);
static void start_usb_mode(void);
static void stop_usb_mode(void);
static void keyboard_task(void);
static void dispatch_keyboard_report(const uint8_t* report);
static void LED_mode_init();
static void RED_turn_on();
static void BLUE_turn_on();
static void GREEN_turn_on();
static void WHITE_turn_on();
static void process_cdc_command(void);
static void Config_Save(void);
static void Config_Load(void);
static void load_default_config(void); // 用于加载硬编码的默认配置
static void ahrs_recalibrate_trigger(void);
//FLASH relevant
#define CONFIG_STORAGE_ADDRESS      (0x6F000) // Flash最后一个4KB扇区的起始地址 (适用于448KB Flash)
#define CONFIG_MAGIC_NUMBER         (0xDEADBEEF) // 自定义一个魔数, 用于判断Flash数据是否有效

//keyboard relevant
// 用于存储键盘上所有按键的实时状态 (位图)
static uint8_t key_matrix_state[8] = {0}; // 假设键盘最多8行
// 标志位，表示键盘状态已更新，需要发送报告
static volatile bool hid_keyboard_report_dirty = false;
#define KEY_MATRIX_ROWS  8
#define KEY_MATRIX_COLS  8
#define ACTION_BASE 0xF0
#define ACTION_TOGGLE_SPACEMOUSE_MODE   (ACTION_BASE + 0) // = 0xF0, 切换时空鼠标的旋转/平移模式
#define ACTION_CHANGE_LED_MODE          (ACTION_BASE + 1) // = 0xF1, 切换LED显示模式
#define ACTION_RECALIBRATE_GYRO         (ACTION_BASE + 2) // = 0xF2, 重新校准陀螺仪零点

static AppConfig_t g_AppConfig;


// 定义从 Key Index (K0-K24) 到 (row, col) 的查找表 (LUT)
static const KeyLocation_t g_key_index_to_location[25] = {
    // K0-K5 (对应代码中的 row 0)
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5},
    // K6-K11 (对应代码中的 row 1)
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5},
    // K12-K16 (对应代码中的 row 2)
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4},
    // K17-K21 (对应代码中的 row 3)
    {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4},
    // K22-K24 (对应代码中的 row 4)
    {4, 2}, {4, 3}, {4, 4}
};

uint8_t g_cdc_rx_buf[CDC_RX_BUF_SIZE];
volatile uint8_t g_cdc_rx_len = 0;
volatile bool g_cdc_data_received_flag = false;

//RF接收事件回调函数
void OnDataReceivedFromDongle(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // 利用现有的CDC命令处理流程来处理来自RF的数据
    // 检查数据长度是否在有效范围内，且小于CDC接收缓冲区的大小
    // (预留一个字节给字符串结束符 '\0')
    if (length > 0 && length < CDC_RX_BUF_SIZE)
    {
        // 确保上一个命令已经被主循环处理完毕。
        if (g_cdc_data_received_flag == false)
        {
            // 1. 将RF接收到的数据复制到全局命令缓冲区
            memcpy((void*)g_cdc_rx_buf, data, length);
            g_cdc_rx_len = length;

            // 2. 设置标志位，通知主循环有新命令需要处理。
            g_cdc_data_received_flag = true;
        }
        else
        {
            // 如果上一个命令还未处理，则丢弃当前数据包，防止缓冲区数据被覆盖
            PRINT("RF command dropped: Command processor is busy.\n");
        }
    }
    else if (length >= CDC_RX_BUF_SIZE)
    {
        // 如果数据包过大，则丢弃
        PRINT("RF command dropped: Packet too large.\n");
    }
}

// --- 公共接口实现 ---

// 初始化模式管理器，并在启动时决定初始模式
void ModeManager_Init(void)
{
    Config_Load();

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
        //WHITE_turn_on();
        PFIC_EnableIRQ(TMR1_IRQn);
        TMR1_Enable(); // 启动用于AHRS的定时器
    }
    else
    {
        // 如果USB未连接，则进入BLE模式
        PRINT("USB Not Attached. Starting in BLE Mode.\n");
        //BLUE_turn_on();    //蓝灯
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

    if(g_keyboard_irq_pending)
    {
        g_keyboard_irq_pending = false;
        uint8_t temp_key_code;
        if(CH450_Read_Key(&g_wai2c_keyboard,&temp_key_code)==0)
        {
            // 确认这是一个“按下”事件 (bit 6 为 1)
            if (temp_key_code & 0x40) 
            {
                FIFO_Put(&g_key_fifo, temp_key_code); // 将“按下”事件放入队列         
                // 记录下是哪个键被按下了，并启动“等待释放”的轮询标志
                last_key_scancode = temp_key_code & 0x3F; // 保存行+列扫描码
                is_waiting_for_release = true;
            }
        }
    }
    // --- 按键轮询处理（只处理“释放”事件） ---
    // 只有在等待某个按键释放时，才执行这部分轮询代码
    if (is_waiting_for_release)
    {
        uint8_t current_key_code;
        // 再次读取CH450获取当前状态
        if (CH450_Read_Key(&g_wai2c_keyboard, &current_key_code) == 0)
        {
            // 检查这是否是一个“释放”事件 (bit 6 为 0)
            if ((current_key_code & 0x40) == 0)
            {
                // 并且，检查这个释放的键是否就是我们等待的那个键
                if ((current_key_code & 0x3F) == last_key_scancode)
                {
                    FIFO_Put(&g_key_fifo, current_key_code); // 将“释放”事件放入队列
                    is_waiting_for_release = false; // 停止轮询
                }
            }
        }
    }

    // 检查并处理模式切换请求
    if (request_switch_to_usb)
    {
        request_switch_to_usb = FALSE;
        if(current_mode == MODE_BLE|| current_mode == MODE_RF)
        {
            PRINT("Switching from BLE/RF to USB mode.\n");
            //WHITE_turn_on();
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
            //BLUE_turn_on();
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

    //执行键盘任务
    keyboard_task(); // 检查并处理所有待处理的按键事件，更新内部状态矩阵

    // 如果键盘状态已更新，则构建并发送报告
    if (hid_keyboard_report_dirty)
    {
        hid_keyboard_report_dirty = false;
        uint8_t kbd_report[8];
        build_keyboard_report_from_state(kbd_report,(const uint8_t*)key_matrix_state);
        dispatch_keyboard_report(kbd_report); // 分发报告
    }

    if (g_cdc_data_received_flag) {
        process_cdc_command();
        g_cdc_data_received_flag = false;
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
                //GREEN_turn_on();
                stop_ble_mode();
                start_rf_mode();
            }
            else if(current_mode == MODE_RF)
            {
                current_mode = MODE_BLE;
                //BLUE_turn_on();
                stop_rf_mode();
                start_ble_mode();
            }
            else {}
            break;
        case GPIO_Pin_7:
        {
            uint8_t temp_key_code;
            g_keyboard_irq_pending = TRUE;
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
    RF_RoleInit();
    RF_MW_Init(OnDataReceivedFromDongle);
    PFIC_EnableIRQ(TMR1_IRQn);
    TMR1_Enable(); // 启动用于AHRS的定时器
}

static void stop_rf_mode(void)
{
    RF_MW_Deinit();
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

void dispatch_spacemouse_report(int16_t trans[3], int16_t rot[3]) {
    if (current_mode == MODE_USB) {
        usb_hid_report_axes(trans, rot);
    } else if (current_mode == MODE_BLE) {
        if (hidEmuConnHandle != GAP_CONNHANDLE_INIT) {
            hidEmuSendSpaceMouseReport(trans[0],trans[1],trans[2],rot[0],rot[1],rot[2]);
        }
    } else if (current_mode == MODE_RF) {
        uint16_t rf_data_tx[6]={trans[0],trans[1],trans[2],rot[0],rot[1],rot[2]};
        // 发送整个新的数据包
        RF_MW_SendData((uint8_t *)rf_data_tx, sizeof(rf_data_tx));
    }
}

void ModeManager_ToggleSpaceMouseMode(void)
{
    g_is_rotation_mode = !g_is_rotation_mode;
    PRINT("SpaceMouse mode toggled. Rotation is now: %s\n", g_is_rotation_mode ? "ON" : "OFF");
}

/**
 * @brief 获取当前SpaceMouse是否处于旋转模式
 * @return true - 旋转模式, false - 平移模式
 */
bool ModeManager_IsRotationMode(void)
{
    return g_is_rotation_mode;
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
void ahrs_task(void)
{
    // --- 1. 定义可调参数 (定点整数与位运算优化版本) ---
    #define INT_SCALE 100
    static const int32_t MAX_INPUT_ANGLE_I = 35 * INT_SCALE;
    static const int32_t ANGLE_DEAD_ZONE_I = 10 * INT_SCALE;
    static const int32_t PITCH_TRANS_DEAD_ZONE_I = 5 * INT_SCALE;      // Pitch平移轴的专属小死区
    static const int32_t PITCH_TRANS_SENS_MULTIPLIER = 2;              // Pitch平移轴的专属灵敏度放大倍数
    static const int32_t YAW_ZOOM_SCALE_NUM = 6;
    static const int32_t LPF_ALPHA_I = 3;
    #define FAST_DIV_BY_10(x) (((x) * 205) >> 11)

    // --- 2. 获取姿态数据 ---
    if(current_mode == MODE_USB)
    {
        MotionEngine_Update(&mouse_motion, 0.02f);
    }
    else if(current_mode == MODE_RF)
    {
        MotionEngine_Update(&mouse_motion, 0.05f);
    }
    else
    {
        MotionEngine_Update(&mouse_motion, 0.05f);
    }

    float pitch = mouse_motion.euler_angles[1];
    float roll  = mouse_motion.euler_angles[0];
    float yaw   = mouse_motion.euler_angles[2];
    if (!is_calibrated)
    {
        // 检查是否是一个新校准周期的开始
        if (sample_count == 0) {
            // 如果是，清零所有累加器
            yaw_offset = 0.0f;
            roll_offset = 0.0f;
            pitch_offset = 0.0f;
            PRINT("Starting new gyro calibration...\n");
        }

        if (sample_count < 500)
        {
            // 1. 累加样本
            yaw_offset += yaw;
            roll_offset += roll;
            pitch_offset += pitch;
            sample_count++;
        }
        else
        {
            // 2. 计算平均偏移量
            yaw_offset = yaw_offset / 500.0f;
            roll_offset = roll_offset / 500.0f;
            pitch_offset = pitch_offset / 500.0f;
            is_calibrated = true; // 标记为已校准
            PRINT("Calibration complete. Offsets: Yaw=%.2f, Roll=%.2f, Pitch=%.2f\n", yaw_offset, roll_offset, pitch_offset);
        }
    }

    if (is_calibrated)
    {
        yaw -= yaw_offset;
        pitch -= pitch_offset;
        roll -= roll_offset;
        // 3. 对后续所有的值应用校准
    }
    //PRINT("Pitch: %0.2f, Roll: %0.2f, Yaw: %0.2f\n", pitch, roll, yaw);
    // --- 3. 初始化输出变量 ---
    int32_t trans_x_i = 0, trans_y_i = 0, trans_z_i = 0;
    int32_t rot_x_i = 0, rot_y_i = 0, rot_z_i = 0;

    // 调用我们新建的函数来获取USB的发送模式
    if (ModeManager_IsRotationMode())
    {
        // **当前为旋转模式 (Rotation Mode)**
        // 只计算旋转数据。平移和缩放相关的数值保持为0。
        int32_t pitch_i = (int32_t)(pitch * INT_SCALE);
        int32_t roll_i  = (int32_t)(roll * INT_SCALE);
        int32_t yaw_i   = (int32_t)(yaw * INT_SCALE);

        rot_x_i = roll_i;
        rot_y_i = pitch_i;
        rot_z_i = yaw_i;
    }
    else
    {
        // **当前为平移/缩放模式 (Translation/Zoom Mode)**
        // 只计算平移(Pan)和缩放(Zoom)数据。旋转相关的数值保持为0。
        int32_t pitch_i = (int32_t)(pitch * INT_SCALE);
        int32_t roll_i  = (int32_t)(roll * INT_SCALE);
        int32_t yaw_i   = (int32_t)(yaw * INT_SCALE);

        // 倾斜设备进行X/Y轴平移
        trans_x_i = roll_i >> 1;
        trans_y_i = -pitch_i >> 1;
        
        // 扭转设备进行Z轴缩放
        trans_z_i = FAST_DIV_BY_10(yaw_i * YAW_ZOOM_SCALE_NUM);
    }

    // --- 5. 统一处理：应用死区、线性缩放和滤波 (全整数运算) ---
    int32_t inputs_i[6] = {trans_x_i, trans_y_i, trans_z_i, rot_x_i, rot_y_i, rot_z_i};
    int16_t output_vals[6] = {0};

    for (int i = 0; i < 6; ++i)
    {
        int32_t val_i = inputs_i[i];
        int32_t dead_zone;

        // 根据轴索引(i)选择不同的死区值
        if (i == 1) { // i=1 对应的是 trans_y_i, 它来源于 pitch
            dead_zone = PITCH_TRANS_DEAD_ZONE_I;
        } else {
            dead_zone = ANGLE_DEAD_ZONE_I;
        }

        if (abs(val_i) < dead_zone) {
            val_i = 0;
        } else {
            if (val_i > MAX_INPUT_ANGLE_I)      val_i = MAX_INPUT_ANGLE_I;
            else if (val_i < -MAX_INPUT_ANGLE_I) val_i = -MAX_INPUT_ANGLE_I;
        }
        output_vals[i] = (int16_t)FAST_DIV_BY_10(val_i);
    }
    
    // --- 整数低通滤波器 (增量形式) ---
    static int16_t filtered_vals[6] = {0};
    static bool prev_is_rotation_mode = false; // 用于存储上一次的模式
    bool current_is_rotation_mode = ModeManager_IsRotationMode();
    if (current_is_rotation_mode != prev_is_rotation_mode)
    {
        // 模式刚刚发生了变化！
        if (current_is_rotation_mode)
        {
            // 如果切换到了旋转模式，就清空平移滤波器的值
            filtered_vals[0] = 0; // trans_x
            filtered_vals[1] = 0; // trans_y
            filtered_vals[2] = 0; // trans_z
        }
        else
        {
            // 如果切换到了平移模式，就清空旋转滤波器的值
            filtered_vals[3] = 0; // rot_x
            filtered_vals[4] = 0; // rot_y
            filtered_vals[5] = 0; // rot_z
        }
        prev_is_rotation_mode = current_is_rotation_mode; // 更新状态记录
    }
    // for (int i = 0; i < 6; ++i) {
    //     int32_t error = output_vals[i] - filtered_vals[i];
    //     filtered_vals[i] += (int16_t)FAST_DIV_BY_10(error * LPF_ALPHA_I);
    // }
    
    // --- 6. 发送最终的6轴数据报告 ---
    int16_t final_trans_vals[3];
    int16_t final_rot_vals[3];

    long sensitivity = g_AppConfig.sensitivity_spacemouse;

    // 基准灵敏度为50，所以除以50
    final_trans_vals[0] = (int16_t)(((int32_t)-output_vals[1] * sensitivity * PITCH_TRANS_SENS_MULTIPLIER) / 50L);
    final_trans_vals[1] = (int16_t)(((int32_t)output_vals[0] * sensitivity) / 50L);
    final_trans_vals[2] = (int16_t)(((int32_t)output_vals[2] * sensitivity) / 50L);

    final_rot_vals[0] = (int16_t)(((int32_t)output_vals[3] * sensitivity) / 50L);
    final_rot_vals[1] = (int16_t)(((int32_t)-output_vals[4] * sensitivity) / 50L);
    final_rot_vals[2] = (int16_t)(((int32_t)-output_vals[5] * sensitivity) / 50L);

    const int16_t FINAL_DEAD_ZONE = 5; 
    
    if (abs(final_trans_vals[0]) < FINAL_DEAD_ZONE) final_trans_vals[0] = 0;
    if (abs(final_trans_vals[1]) < FINAL_DEAD_ZONE) final_trans_vals[1] = 0;
    if (abs(final_trans_vals[2]) < FINAL_DEAD_ZONE) final_trans_vals[2] = 0;
    
    if (abs(final_rot_vals[0]) < FINAL_DEAD_ZONE) final_rot_vals[0] = 0;
    if (abs(final_rot_vals[1]) < FINAL_DEAD_ZONE) final_rot_vals[1] = 0;
    if (abs(final_rot_vals[2]) < FINAL_DEAD_ZONE) final_rot_vals[2] = 0;
    dispatch_spacemouse_report(final_trans_vals, final_rot_vals);
}

static void ahrs_recalibrate_trigger(void)
{
    PRINT("Gyro recalibration requested!\n");
    is_calibrated = false;
    sample_count = 0; 
}

//键盘部分

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
        // 4. 发送这个符合我们自定义协议的、总长8字节的数据包
        RF_MW_SendHighPrioData(report,8);
    }
}


static void keyboard_task(void)
{
    KeyEvent_t key_event;
    bool state_changed = false;

    // 循环处理，直到清空FIFO中的所有待处理事件
    while (Keyboard_GetKeyEvent(&key_event))
    {
        // 1. 根据按键的物理位置，从配置中查找其映射
        KeyMapping_t mapping = g_AppConfig.key_mappings[key_event.row][key_event.col];

        if(key_event.pressed)
        {
            // 2. 检查映射的 hid_code 是否为特殊功能代码
            if (mapping.hid_code >= ACTION_BASE)
            {
                // --- 这是个特殊功能按键 ---
                // 使用 switch 来处理不同的特殊功能
                switch (mapping.hid_code)
                {
                    case ACTION_TOGGLE_SPACEMOUSE_MODE:
                            ModeManager_ToggleSpaceMouseMode();
                        break;

                    case ACTION_CHANGE_LED_MODE:
                        change_LED_display_mode();
                        break;

                    case ACTION_RECALIBRATE_GYRO:
                        ahrs_recalibrate_trigger();
                        break;

                    default:
                        // 未知的特殊功能代码，不做任何事
                        break;
                }
                // 特殊功能键执行后，不应再作为普通按键处理，直接跳过后续逻辑
                continue;
            }
            else
            {
                // --- 这是个标准HID按键 ---
                // 按下事件：在状态矩阵中将对应位置1
                key_matrix_state[key_event.row] |= (1 << key_event.col);
                state_changed = true; // 标记状态已改变
            }
        }
        else
        {
            // --- 释放事件 ---
            // 对于标准HID按键，在状态矩阵中将对应位置0
            if(mapping.hid_code < ACTION_BASE){
                 key_matrix_state[key_event.row] &= ~(1 << key_event.col);
                 state_changed = true; // 标记状态已改变
            }
            // 注意：特殊功能按键通常只在“按下”时触发一次，释放时无需额外操作。
        }
    }

    // 3. 如果HID按键状态发生变化，则标记需要发送报告
    if (state_changed)
    {
            hid_keyboard_report_dirty = true;
    }
}

/**
 * @brief 获取当前键盘矩阵状态的快照
 */
void ModeManager_GetKeyboardState(uint8_t* state_buffer)
{
    // 使用memcpy保证读取的原子性
    memcpy(state_buffer, (const void*)key_matrix_state, 8);
}


/**
 * @brief 根据给定的键盘状态矩阵，使用可自定义的 g_key_mappings 表
 * 构建一个标准的8字节HID键盘回报。
 * @param report      (输出) 8字节的HID报告缓冲区
 * @param key_state   (输入) 8字节的键盘扫描状态矩阵
 */
void build_keyboard_report_from_state(uint8_t* report, const uint8_t* key_state)
{
    // 1. 清空报告，特别是修饰键字节
    memset(report, 0, 8); // report[0]是修饰键, report[1]保留, report[2-7]是普通键
    uint8_t key_count = 0;
    int row, col;

    // 2. 遍历整个键盘物理扫描矩阵
    for (row = 0; row < KEY_MATRIX_ROWS; ++row)
    {
        // 优化：如果这一行没有任何按键按下，快速跳到下一行
        if (key_state[row] == 0) continue;

        for (col = 0; col < KEY_MATRIX_COLS; ++col)
        {
            // 3. 检查当前(row, col)的按键是否被按下
            if (key_state[row] & (1 << col))
            {
                // 4. 从全局映射表中查找对应的键值和修饰键
                KeyMapping_t mapping = g_AppConfig.key_mappings[row][col];

                // 5. 应用修饰键 (使用'|='位或运算，可以叠加多个按键的修饰键)
                report[0] |= mapping.modifier;

                // 6. 添加普通键 (如果hid_code不为0，并且报告中还有空间)
                if (mapping.hid_code != KEY_NONE && key_count < 6)
                {
                    // (可选但推荐) 检查键码是否已在报告中，防止重复添加
                    bool found = false;
                    for(int i = 0; i < key_count; ++i) {
                        if(report[2 + i] == mapping.hid_code) {
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        report[2 + key_count] = mapping.hid_code;
                        key_count++;
                    }
                }
            }
        }
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

/**
 * @brief 解析并处理从CDC（虚拟串口）接收到的命令字符串。
 * 此函数在主循环中被调用，当 g_cdc_data_received_flag 为 true 时执行。
 */
static void process_cdc_command(void)
{
    // 1. 为确保字符串处理安全，在数据末尾添加'\0'
    g_cdc_rx_buf[g_cdc_rx_len] = '\0';
    // 打印收到的原始数据，用于调试
    printf("CDC Recv %d bytes: %s", g_cdc_rx_len, (char *)g_cdc_rx_buf);

    // 2. 使用strtok分割字符串。第一个分隔符是':'
    // strtok会修改原始字符串，我们直接在接收缓冲区上操作
    char* token = strtok((char*)g_cdc_rx_buf, ":");

    if (token == NULL) {
        printf("Error: Malformed command, no token found.\n");
        return;
    }

    // 3. 根据第一个令牌（命令类型）进行分发
    if (strcmp(token, "KEY") == 0)
    {
        // --- 处理键盘映射指令: KEY:keyIndex:modifierByte:keyCode\n ---
        char* keyIndex_str = strtok(NULL, ":");
        char* modifierByte_str = strtok(NULL, ":");
        char* keyCode_str = strtok(NULL, "\r\n"); // 兼容\r\n和\n结尾

        if (keyIndex_str && modifierByte_str && keyCode_str) {
            // 将字符串参数转换为长整型
            long keyIndex = strtol(keyIndex_str, NULL, 10);     // 10表示十进制
            long modifierByte = strtol(modifierByte_str, NULL, 16); // 16表示十六进制
            long keyCode = strtol(keyCode_str, NULL, 16);       // 16表示十六进制

            // 调用具体的处理函数
            handle_key_mapping_update(keyIndex, modifierByte, keyCode);
        } else {
            printf("Error: Incomplete KEY command.\n");
        }

    } else if (strcmp(token, "MOUSE") == 0)
    {
        // --- 处理鼠标灵敏度指令: MOUSE:sensitivity\n ---
        char* sensitivity_str = strtok(NULL, "\r\n"); // 兼容\r\n和\n结尾

        if (sensitivity_str) {
            long sensitivity = strtol(sensitivity_str, NULL, 10); // 10表示十进制
            handle_mouse_sensitivity_update(sensitivity);
        } else {
            printf("Error: Incomplete MOUSE command.\n");
        }

    } else if (strcmp(token,"EXT") == 0) 
    {
        char* keyindex_ext = strtok(NULL,":");
        char* typehex_ext = strtok(NULL,":");
        char* command_ext = strtok(NULL,"\r\n");
        if(keyindex_ext && typehex_ext && command_ext)
        {
            long key_index_ext = strtol(keyindex_ext,NULL,10);
            long type_hex_ext = strtol(typehex_ext,NULL,16);
            long cmd_hex_ext = strtol(command_ext,NULL,16);
            handle_ext(key_index_ext,type_hex_ext,cmd_hex_ext);
        }
        else{
            PRINT("Error:Incomplete EXT command.\n");
        }
    }
    else {
        // --- 未知指令 ---
        printf("Error: Unknown command '%s'.\n", token);
    }
}



/**
 * @brief 处理来自上位机的按键映射更新指令。
 * 此函数由 process_cdc_command 调用。
 * @param keyIndex       要修改的按键索引 (范围: 0-24)
 * @param modifierByte   新的修饰键字节 (例如 MOD_LEFT_CTRL)
 * @param keyCode        新的HID键码 (例如 KEY_A)
 */
void handle_key_mapping_update(long keyIndex, long modifierByte, long keyCode)
{
    // 1. 输入验证：检查 keyIndex 是否在合法的 0-24 范围内
    if (keyIndex < 0 || keyIndex >= 25) {
        printf("Error: Invalid Key Index %ld. Must be between 0 and 24.\n", keyIndex);
        return;
    }

    // 2. 使用查找表将 keyIndex (逻辑索引) 转换为 (row, col) (物理位置)
    KeyLocation_t loc = g_key_index_to_location[keyIndex];

    // 3. 更新全局映射表
    g_AppConfig.key_mappings[loc.row][loc.col].modifier = (uint8_t)modifierByte;
    g_AppConfig.key_mappings[loc.row][loc.col].hid_code = (uint8_t)keyCode;

    Config_Save();
}

/**
 * @brief  处理鼠标灵敏度更新指令
 */
void handle_mouse_sensitivity_update(long sensitivity)
{
    // 1. 更新内存中的配置副本
    g_AppConfig.sensitivity_spacemouse = sensitivity;
    printf("Mouse sensitivity set to %ld\n", g_AppConfig.sensitivity_spacemouse);

    // 2. 将更新后的配置保存到Flash
    Config_Save();
}

/**
 * @brief 处理EXT命令
 */
void handle_ext(long keyIndex, long typehex, long commandhex)
{
    if (keyIndex < 0 || keyIndex >= 25) {
        printf("Error: Invalid Key Index %ld. Must be between 0 and 24.\n", keyIndex);
        return;
    }
    KeyLocation_t loc = g_key_index_to_location[keyIndex];
    switch(typehex)
    {
        case 0x01: //旋转平移模式切换
            g_AppConfig.key_mappings[loc.row][loc.col].modifier = 0;
            g_AppConfig.key_mappings[loc.row][loc.col].hid_code = ACTION_TOGGLE_SPACEMOUSE_MODE;
            break;
        case 0x02: //陀螺仪校准
            g_AppConfig.key_mappings[loc.row][loc.col].modifier = 0;
            g_AppConfig.key_mappings[loc.row][loc.col].hid_code = ACTION_RECALIBRATE_GYRO;
            break;
        case 0x03: //LED控制
            g_AppConfig.key_mappings[loc.row][loc.col].modifier = 0;
            g_AppConfig.key_mappings[loc.row][loc.col].hid_code = ACTION_CHANGE_LED_MODE;
            break;
    }
}

/**
 * @brief 将当前内存中的 g_AppConfig 结构体保存到Flash.
 */
static void Config_Save(void)
{
    printf("Saving configuration to Flash...\n");
    // 步骤1: 擦除将要写入的目标Flash扇区 (4KB)
    if (FLASH_ROM_ERASE(CONFIG_STORAGE_ADDRESS, 4096) == 0)
    {
        // 步骤2: 擦除成功后，将内存中的 g_AppConfig 结构体完整写入Flash
        if (FLASH_ROM_WRITE(CONFIG_STORAGE_ADDRESS, &g_AppConfig, sizeof(AppConfig_t)) == 0)
        {
            printf("Configuration saved successfully.\n");
        } else {
            printf("Error: Failed to write config to Flash.\n");
        }
    } else {
        printf("Error: Failed to erase Flash sector.\n");
    }
}

/**
 * @brief 从Flash中加载配置到 g_AppConfig.
 * 如果Flash数据无效或为空，则加载默认配置并回存到Flash.
 */
static void Config_Load(void)
{
    printf("Loading configuration from Flash...\n");
    // 步骤1: 直接从Flash的指定地址读取数据到 g_AppConfig 结构体
    FLASH_ROM_READ(CONFIG_STORAGE_ADDRESS, &g_AppConfig, sizeof(AppConfig_t));

    // 步骤2: 检查魔数是否匹配
    if (g_AppConfig.magic_number == CONFIG_MAGIC_NUMBER)
    {
        printf("Valid configuration found in Flash.\n");
    }
    else
    {
        printf("No valid config found. Loading defaults and saving to Flash.\n");
        // 步骤3: 加载硬编码的默认值
        load_default_config();
        // 步骤4: 将这份全新的默认配置保存回Flash，供下次启动使用
        Config_Save();
    }
}

/**
 * @brief 将硬编码的默认值加载到 g_AppConfig 结构体中.
 * @details 此版本已更新，包含了特殊功能按键的默认映射。
 */
static void load_default_config(void)
{
    // 设置魔数和默认灵敏度
    g_AppConfig.magic_number = CONFIG_MAGIC_NUMBER;
    g_AppConfig.sensitivity_spacemouse = 50L;

    // 使用memset将整个映射表清零
    memset(g_AppConfig.key_mappings, 0, sizeof(g_AppConfig.key_mappings));

    // --- 填充标准HID按键的默认映射 ---
    // Row 0
    g_AppConfig.key_mappings[0][0] = (KeyMapping_t){.hid_code = KEY_ESCAPE, .modifier = 0};
    g_AppConfig.key_mappings[0][1] = (KeyMapping_t){.hid_code = KEY_DELETE, .modifier = 0};
    g_AppConfig.key_mappings[0][2] = (KeyMapping_t){.hid_code = KEY_Z, .modifier = MOD_LEFT_CTRL};
    g_AppConfig.key_mappings[0][3] = (KeyMapping_t){.hid_code = KEY_C, .modifier = MOD_LEFT_CTRL};
    g_AppConfig.key_mappings[0][4] = (KeyMapping_t){.hid_code = KEY_V, .modifier = MOD_LEFT_CTRL};
    g_AppConfig.key_mappings[0][5] = (KeyMapping_t){.hid_code = KEY_BACKSPACE, .modifier = 0};

    // Row 1
    g_AppConfig.key_mappings[1][0] = (KeyMapping_t){.hid_code = KEY_NONE, .modifier = MOD_LEFT_ALT};
    g_AppConfig.key_mappings[1][1] = (KeyMapping_t){.hid_code = KEY_7, .modifier = 0};
    g_AppConfig.key_mappings[1][2] = (KeyMapping_t){.hid_code = KEY_8, .modifier = 0};
    g_AppConfig.key_mappings[1][3] = (KeyMapping_t){.hid_code = KEY_9, .modifier = 0};
    g_AppConfig.key_mappings[1][4] = (KeyMapping_t){.hid_code = KEY_B, .modifier = MOD_LEFT_CTRL};
    g_AppConfig.key_mappings[1][5] = (KeyMapping_t){.hid_code = KEY_ENTER, .modifier = 0};

    // Row 2
    g_AppConfig.key_mappings[2][0] = (KeyMapping_t){.hid_code = KEY_NONE, .modifier = MOD_LEFT_CTRL};
    g_AppConfig.key_mappings[2][1] = (KeyMapping_t){.hid_code = KEY_4, .modifier = 0};
    g_AppConfig.key_mappings[2][2] = (KeyMapping_t){.hid_code = KEY_5, .modifier = 0};
    g_AppConfig.key_mappings[2][3] = (KeyMapping_t){.hid_code = KEY_6, .modifier = 0};
    g_AppConfig.key_mappings[2][4] = (KeyMapping_t){.hid_code = ACTION_RECALIBRATE_GYRO, .modifier = 0};

    // Row 3
    g_AppConfig.key_mappings[3][0] = (KeyMapping_t){.hid_code = KEY_NONE, .modifier = MOD_LEFT_SHIFT};
    g_AppConfig.key_mappings[3][1] = (KeyMapping_t){.hid_code = KEY_1, .modifier = 0};
    g_AppConfig.key_mappings[3][2] = (KeyMapping_t){.hid_code = KEY_2, .modifier = 0};
    g_AppConfig.key_mappings[3][3] = (KeyMapping_t){.hid_code = KEY_3, .modifier = 0};
    g_AppConfig.key_mappings[3][4] = (KeyMapping_t){.hid_code = ACTION_CHANGE_LED_MODE, .modifier = 0};

    // Row 4
    g_AppConfig.key_mappings[4][2] = (KeyMapping_t){.hid_code = KEY_0, .modifier = 0};
    g_AppConfig.key_mappings[4][3] = (KeyMapping_t){.hid_code = KEY_DOT, .modifier = 0};
    g_AppConfig.key_mappings[4][4] = (KeyMapping_t){.hid_code = ACTION_TOGGLE_SPACEMOUSE_MODE, .modifier = 0};
}