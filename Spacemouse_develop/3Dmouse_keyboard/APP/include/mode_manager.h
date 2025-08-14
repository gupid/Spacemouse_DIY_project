#ifndef __MODE_MANAGER_H__
#define __MODE_MANAGER_H__

#include "CH58x_common.h"
#include "main.h" 
#define CDC_RX_BUF_SIZE 64 // 确保大小足够
extern uint8_t g_cdc_rx_buf[CDC_RX_BUF_SIZE];
extern volatile uint8_t g_cdc_rx_len;
extern volatile bool g_cdc_data_received_flag;

typedef enum {
    MODE_NONE,
    MODE_USB,
    MODE_BLE,
    MODE_RF
} OperatingMode_t;
// 定义一个结构体，用于存储单个按键的映射信息
typedef struct {
    uint8_t hid_code;   // HID Usage ID (例如 KEY_A, KEY_1, KEY_F1)
    uint8_t modifier;   // 修饰键 (例如 MOD_LEFT_CTRL, MOD_LEFT_SHIFT)
} KeyMapping_t;

// 定义一个结构体，用于查找表，连接Key Index和物理扫描位置
typedef struct {
    uint8_t row;
    uint8_t col;
} KeyLocation_t;

// 定义保存在Flash中的总配置结构体
// 注意：该结构体的大小必须是4字节的倍数，以满足Flash写入要求。
typedef struct {
    uint32_t     magic_number;               // 用于校验数据有效性的魔数
    KeyMapping_t key_mappings[8][8];         // 键盘映射表
    long         sensitivity_spacemouse;     // 鼠标灵敏度
} AppConfig_t;

// 函数声明
void ModeManager_Init(void);
void ModeManager_MainLoop(void);
void ModeManager_RequestSwitchToUSB(void);
void ModeManager_RequestSwitchToBLE(void);
OperatingMode_t ModeManager_GetCurrentMode(void);
void ModeManager_SetUSBAttached(void);
/**
 * @brief 获取当前键盘矩阵状态的快照。
 * @param state_buffer - 一个指向8字节数组的指针，用于存放状态矩阵。
 */
void ModeManager_GetKeyboardState(uint8_t* state_buffer);

/**
 * @brief 根据给定的键盘状态矩阵，构建一个标准的8字节HID键盘回报。
 * @param report - 一个指向8字节数组的指针，用于存放构建好的回报。
 * @param key_state - 一个指向8字节状态矩阵的指针。
 */
void build_keyboard_report_from_state(uint8_t* report, const uint8_t* key_state);
extern uint8_t hidEmuTaskId;
void handle_key_mapping_update(long keyIndex, long modifierByte, long keyCode);
void handle_mouse_sensitivity_update(long sensitivity);
void handle_ext(long keyIndex, long typehex, long commandhex);
void ahrs_task(void);
void ModeManager_ToggleSpaceMouseMode(void);
bool ModeManager_IsRotationMode(void);
#endif // __MODE_MANAGER_H__