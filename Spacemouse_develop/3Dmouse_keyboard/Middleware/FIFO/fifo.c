#include "fifo.h"

void FIFO_Init(KeyCodeFifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}

uint8_t FIFO_Put(KeyCodeFifo_t *fifo, uint8_t key_code)
{
    // 计算下一个尾部索引
    uint8_t next_tail = (fifo->tail + 1) % FIFO_BUFFER_SIZE;

    // 检查队列是否已满
    if (next_tail == fifo->head)
    {
        return 1; // 队列已满，返回错误
    }

    // 将数据存入缓冲区
    fifo->buffer[fifo->tail] = key_code;
    // 更新尾部索引
    fifo->tail = next_tail;

    return 0; // 成功
}

uint8_t FIFO_Get(KeyCodeFifo_t *fifo, uint8_t *key_code)
{
    // 检查队列是否为空
    if (fifo->head == fifo->tail)
    {
        return 1; // 队列为空，返回错误
    }

    // 从缓冲区取出数据
    *key_code = fifo->buffer[fifo->head];
    // 更新头部索引
    fifo->head = (fifo->head + 1) % FIFO_BUFFER_SIZE;

    return 0; // 成功
}