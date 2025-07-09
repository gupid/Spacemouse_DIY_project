#include "fifo.h"

void FIFO_Init(KeyCodeFifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}

uint8_t FIFO_Put(KeyCodeFifo_t *fifo, uint8_t key_code)
{
    // ������һ��β������
    uint8_t next_tail = (fifo->tail + 1) % FIFO_BUFFER_SIZE;

    // �������Ƿ�����
    if (next_tail == fifo->head)
    {
        return 1; // �������������ش���
    }

    // �����ݴ��뻺����
    fifo->buffer[fifo->tail] = key_code;
    // ����β������
    fifo->tail = next_tail;

    return 0; // �ɹ�
}

uint8_t FIFO_Get(KeyCodeFifo_t *fifo, uint8_t *key_code)
{
    // �������Ƿ�Ϊ��
    if (fifo->head == fifo->tail)
    {
        return 1; // ����Ϊ�գ����ش���
    }

    // �ӻ�����ȡ������
    *key_code = fifo->buffer[fifo->head];
    // ����ͷ������
    fifo->head = (fifo->head + 1) % FIFO_BUFFER_SIZE;

    return 0; // �ɹ�
}