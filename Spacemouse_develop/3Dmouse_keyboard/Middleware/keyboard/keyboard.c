#include "keyboard.h"
#include "ws2812b_control.h"
#include <math.h>
//重要的回调函数卸载了mode_manager里。
// ================== 模块内部私有定义 ==================

// I2C通信引脚定义 (从Main.c移入)
#define CH450_SCL_PORT      GPIOB
#define CH450_SCL_PIN       GPIO_Pin_0
#define CH450_SDA_PORT      GPIOB
#define CH450_SDA_PIN       GPIO_Pin_1

// CH450中断引脚定义 (从Main.c移入)
#define CH450_INT_PORT      GPIOB
#define CH450_INT_PIN       GPIO_Pin_7

// 将I2C句柄和FIFO实例定义为模块内部的静态全局变量，外部无法访问
I2C_HandleDef g_wai2c_keyboard;
KeyCodeFifo_t g_key_fifo;


// 定义 LED 数量，需要和您的驱动文件 ws2812b_control.c 中的 NUM_LEDS 保持一致
#define APP_NUM_LEDS 25

// 动画参数
#define BREATHING_SPEED     0.01f   // 呼吸速度，值越大呼吸越快
#define RAINBOW_SHIFT_SPEED   0.9f    // 彩虹移动速度，值越大移动越快
#define MIN_BRIGHTNESS  0.1f // 最小亮度，避免完全熄灭 (例如10%)
#define MAX_BRIGHTNESS  1.0f // 最大亮度

volatile static uint8_t keyboard_led_mode =0;
//定义按键类型枚举，方便管理
typedef enum {
    KEY_TYPE_UNUSED,      // 未使用的LED
    KEY_TYPE_MODIFIER,    // 核心修饰键
    KEY_TYPE_ACTION,      // 编辑/操作键
    KEY_TYPE_NUMBER,      // 数字键
    KEY_TYPE_CUSTOM       // 自定义键
} KeyType_e;

//   创建LED索引到按键类型的映射表
//   数组的索引是LED编号，值是该LED对应的按键类型
const KeyType_e g_led_key_type_map[APP_NUM_LEDS] = {
    /* LED 0 */  KEY_TYPE_CUSTOM,
    /* LED 1 */  KEY_TYPE_NUMBER,   // .
    /* LED 2 */  KEY_TYPE_NUMBER,   // 0
    /* LED 3 */  KEY_TYPE_CUSTOM,
    /* LED 4 */  KEY_TYPE_NUMBER,   // 3
    /* LED 5 */  KEY_TYPE_NUMBER,   // 2
    /* LED 6 */  KEY_TYPE_NUMBER,   // 1
    /* LED 7 */  KEY_TYPE_MODIFIER, // Shift
    /* LED 8 */  KEY_TYPE_CUSTOM,
    /* LED 9 */  KEY_TYPE_NUMBER,   // 6
    /* LED 10 */ KEY_TYPE_NUMBER,   // 5
    /* LED 11 */ KEY_TYPE_NUMBER,   // 4
    /* LED 12 */ KEY_TYPE_MODIFIER, // Ctrl
    /* LED 13 */ KEY_TYPE_ACTION,   // enter
    /* LED 14 */ KEY_TYPE_CUSTOM,
    /* LED 15 */ KEY_TYPE_NUMBER,   // 9
    /* LED 16 */ KEY_TYPE_NUMBER,   // 8
    /* LED 17 */ KEY_TYPE_NUMBER,   // 7
    /* LED 18 */ KEY_TYPE_MODIFIER, // Alt
    /* LED 19 */ KEY_TYPE_ACTION,   // Backspace
    /* LED 20 */ KEY_TYPE_ACTION,   // Paste
    /* LED 21 */ KEY_TYPE_ACTION,   // copy
    /* LED 22 */ KEY_TYPE_ACTION,   // undo
    /* LED 23 */ KEY_TYPE_ACTION,   // Delete
    /* LED 24 */ KEY_TYPE_ACTION    // Esc
};
// ================== 公开函数实现 =================

void Keyboard_Init(void)
{
    // 1. 初始化FIFO
    FIFO_Init(&g_key_fifo);

    // 2. 初始化CH450的I2C通信
    // 注意：这里的wai2c句柄使用了模块内的静态变量
    CH450_Init(&g_wai2c_keyboard, CH450_SCL_PORT, CH450_SCL_PIN, CH450_SDA_PORT, CH450_SDA_PIN, 1);
    
    // 3. 初始化CH450的中断引脚 (使用您底层驱动的GPIO中断初始化)
    GPIOB_ModeCfg(CH450_INT_PIN, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(CH450_INT_PIN, GPIO_ITMode_FallEdge);

    // 4. 配置中断优先级并使能
    PFIC_SetPriority(GPIO_B_IRQn, 1); // 优先级可以根据需要调整
    PFIC_EnableIRQ(GPIO_B_IRQn);

    // 5. 开启CH450的显示和键盘功能
   if(CH450_Write_Cmd(&g_wai2c_keyboard, CH450_SYSON2) == 0)
    {
        PRINT("CH450 keyboard and display enabled.\n");
    }
    else
    {
        PRINT("Failed to enable CH450.\n");
    }
}

bool Keyboard_GetKeyEvent(KeyEvent_t *pEvent)
{
    if (pEvent == NULL) return false;

    uint8_t raw_key_code;
    // 尝试从FIFO获取一个原始键值
    if (FIFO_Get(&g_key_fifo, &raw_key_code) == 0) // 0 表示成功
    {
        // 解析原始键值到结构体
        pEvent->pressed = (raw_key_code & 0x40) ? true : false;
        pEvent->row = (raw_key_code >> 3) & 0x07;
        pEvent->col = raw_key_code & 0x07;

        // CH450的列码从2开始，我们转换为从0开始
        if (pEvent->col >= 2)
        {
            pEvent->col -= 2;
        }
        
        return true; // 成功获取并解析了一个事件
    }

    return false; // FIFO为空，没有事件
}


/**
 * @brief 将 HSV 颜色模型转换为 RGB 颜色模型
 * @param h 色相 (0.0f - 360.0f)
 * @param s 饱和度 (0.0f - 1.0f)
 * @param v 亮度 (0.0f - 1.0f)
 * @param r 返回的红色分量指针 (0-255)
 * @param g 返回的绿色分量指针 (0-255)
 * @param b 返回的蓝色分量指针 (0-255)
 */
void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float rf, gf, bf;

    if (h >= 0.0f && h < 60.0f) {
        rf = c; gf = x; bf = 0;
    } else if (h >= 60.0f && h < 120.0f) {
        rf = x; gf = c; bf = 0;
    } else if (h >= 120.0f && h < 180.0f) {
        rf = 0; gf = c; bf = x;
    } else if (h >= 180.0f && h < 240.0f) {
        rf = 0; gf = x; bf = c;
    } else if (h >= 240.0f && h < 300.0f) {
        rf = x; gf = 0; bf = c;
    } else {
        rf = c; gf = 0; bf = x;
    }

    *r = (uint8_t)((rf + m) * 255.0f);
    *g = (uint8_t)((gf + m) * 255.0f);
    *b = (uint8_t)((bf + m) * 255.0f);
}

/**
 * @brief 更新并显示一帧彩虹呼吸灯动画（已优化）。
 * @note  此函数为非阻塞函数，通过设置最小亮度避免了完全熄灭的问题。
 */
void WS2812B_RainbowBreathe_Update(void)
{
    // 如果DMA正在发送数据，则本次不更新，直接返回
    if (WS2812B_IsBusy())
    {
        return;
    }

    // 使用 static 变量来保存动画的状态
    static float breath_step = 0.0f;
    static float hue_offset = 0.0f;

    // 1. 计算当前呼吸周期的亮度 (V) - 【已修改】
    // 首先，将sinf()函数返回的[-1, 1]范围映射到[0, 1]的相位值
    float breath_phase = (sinf(breath_step) + 1.0f) / 2.0f; 
    // 然后，将[0, 1]的相位映射到我们设定的[MIN_BRIGHTNESS, MAX_BRIGHTNESS]亮度区间
    float brightness = MIN_BRIGHTNESS + breath_phase * (MAX_BRIGHTNESS - MIN_BRIGHTNESS);
    
    // 2. 更新呼吸和色相的步进值
    breath_step += BREATHING_SPEED;
    if(breath_step > 2.0f * 3.1415926f) { // 一个周期后重置，防止浮点数溢出
        breath_step -= 2.0f * 3.1415926f;
    }
    
    hue_offset += RAINBOW_SHIFT_SPEED;
    if(hue_offset > 360.0f) {
        hue_offset -= 360.0f;
    }

    uint8_t r, g, b;

    // 3. 为每一个LED计算颜色并填充到缓冲区
    for (uint16_t i = 0; i < APP_NUM_LEDS; i++)
    {
        // 计算当前LED的色相 (H)
        float hue = fmodf((i * 360.0f / APP_NUM_LEDS) + hue_offset, 360.0f);

        // 将 HSV 转换为 RGB
        hsv_to_rgb(hue, 1.0f, brightness, &r, &g, &b);

        // 设置单个LED的颜色（仅修改内存）
        WS2812B_SetPixelColor(i, r, g, b);
    }

    // 4. 发送数据，刷新LED显示
    WS2812B_Refresh();
}

/**
 * @brief 设置“赛博朋克夜色”功能分区静态灯效
 * @note  此函数会根据 g_led_key_type_map 表为每个LED设置颜色。
 */
void Light_Set_FunctionalZoning_Cyberpunk(void)
{
    // 等待上一次DMA传输完成
    while(WS2812B_IsBusy());

    uint8_t r, g, b;

    for (uint16_t i = 0; i < APP_NUM_LEDS; i++)
    {
        // 从映射表中获取当前LED的类型
        KeyType_e key_type = g_led_key_type_map[i];

        // 根据类型选择颜色
        switch(key_type)
        {
            case KEY_TYPE_MODIFIER: // 深蓝色
                hsv_to_rgb(240.0f, 1.0f, 1.0f, &r, &g, &b);
                break;
            case KEY_TYPE_ACTION:   // 品红色
                hsv_to_rgb(310.0f, 1.0f, 1.0f, &r, &g, &b);
                break;
            case KEY_TYPE_NUMBER:   // 青色
                hsv_to_rgb(180.0f, 1.0f, 1.0f, &r, &g, &b);
                break;
            case KEY_TYPE_CUSTOM:   // 金色/黄色
                hsv_to_rgb(50.0f, 1.0f, 1.0f, &r, &g, &b);
                break;
            case KEY_TYPE_UNUSED:   // 熄灭或设为暗色
            default:
                r = g = b = 0;
                break;
        }
        WS2812B_SetPixelColor(i, r, g, b);
    }

    // 刷新LED显示
    WS2812B_Refresh();
}

void change_LED_display_mode()
{
    keyboard_led_mode++;
    keyboard_led_mode %= 2;
    if (keyboard_led_mode == 1) 
    {
        Light_Set_FunctionalZoning_Cyberpunk();    
    }
}

void keyboard_LED_display()
{
    if(keyboard_led_mode == 0)
    {
        WS2812B_RainbowBreathe_Update();
    }
}