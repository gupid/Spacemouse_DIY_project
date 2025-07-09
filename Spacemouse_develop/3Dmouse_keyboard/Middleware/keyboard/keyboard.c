#include "keyboard.h"

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
    PFIC_SetPriority(GPIO_B_IRQn, 0); // 优先级可以根据需要调整
    PFIC_EnableIRQ(GPIO_B_IRQn);

    // 5. 开启CH450的显示和键盘功能
    CH450_Write_Cmd(&g_wai2c_keyboard, CH450_SYSON2);
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
