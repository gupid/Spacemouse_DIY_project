#ifndef __FIFO_H
#define __FIFO_H

#include "main.h" // 引入您的主头文件以获取 uint8_t 等类型定义

// --- 用户可配置 ---
#define FIFO_BUFFER_SIZE    16 // 定义缓冲区大小，可存储16个按键事件
// --------------------

// 检查缓冲区大小是否为2的幂，这不是强制的，但可以优化取模运算
#if (FIFO_BUFFER_SIZE & (FIFO_BUFFER_SIZE - 1)) != 0
#warning "FIFO_BUFFER_SIZE is not a power of 2, which may result in slightly slower performance."
#endif

// FIFO 结构体定义
typedef struct {
    volatile uint8_t head; // 头部索引（读取位置）
    volatile uint8_t tail; // 尾部索引（写入位置）
    uint8_t buffer[FIFO_BUFFER_SIZE]; // 数据缓冲区
} KeyCodeFifo_t;


/**
 * @brief 初始化FIFO队列
 * @param fifo - 指向要初始化的FIFO结构体的指针
 */
void FIFO_Init(KeyCodeFifo_t *fifo);

/**
 * @brief 向FIFO队列中存入一个键值
 * @param fifo     - 指向FIFO结构体的指针
 * @param key_code - 要存入的8位键值
 * @return uint8_t - 0: 成功, 1: 失败 (队列已满)
 */
uint8_t FIFO_Put(KeyCodeFifo_t *fifo, uint8_t key_code);

/**
 * @brief 从FIFO队列中取出一个键值
 * @param fifo     - 指向FIFO结构体的指针
 * @param key_code - 指向用于存储取出键值的变量的指针
 * @return uint8_t - 0: 成功, 1: 失败 (队列为空)
 */
uint8_t FIFO_Get(KeyCodeFifo_t *fifo, uint8_t *key_code);

#endif // __FIFO_H