#ifndef __FIFO_H
#define __FIFO_H

#include "main.h" // ����������ͷ�ļ��Ի�ȡ uint8_t �����Ͷ���

// --- �û������� ---
#define FIFO_BUFFER_SIZE    16 // ���建������С���ɴ洢16�������¼�
// --------------------

// ��黺������С�Ƿ�Ϊ2���ݣ��ⲻ��ǿ�Ƶģ��������Ż�ȡģ����
#if (FIFO_BUFFER_SIZE & (FIFO_BUFFER_SIZE - 1)) != 0
#warning "FIFO_BUFFER_SIZE is not a power of 2, which may result in slightly slower performance."
#endif

// FIFO �ṹ�嶨��
typedef struct {
    volatile uint8_t head; // ͷ����������ȡλ�ã�
    volatile uint8_t tail; // β��������д��λ�ã�
    uint8_t buffer[FIFO_BUFFER_SIZE]; // ���ݻ�����
} KeyCodeFifo_t;


/**
 * @brief ��ʼ��FIFO����
 * @param fifo - ָ��Ҫ��ʼ����FIFO�ṹ���ָ��
 */
void FIFO_Init(KeyCodeFifo_t *fifo);

/**
 * @brief ��FIFO�����д���һ����ֵ
 * @param fifo     - ָ��FIFO�ṹ���ָ��
 * @param key_code - Ҫ�����8λ��ֵ
 * @return uint8_t - 0: �ɹ�, 1: ʧ�� (��������)
 */
uint8_t FIFO_Put(KeyCodeFifo_t *fifo, uint8_t key_code);

/**
 * @brief ��FIFO������ȡ��һ����ֵ
 * @param fifo     - ָ��FIFO�ṹ���ָ��
 * @param key_code - ָ�����ڴ洢ȡ����ֵ�ı�����ָ��
 * @return uint8_t - 0: �ɹ�, 1: ʧ�� (����Ϊ��)
 */
uint8_t FIFO_Get(KeyCodeFifo_t *fifo, uint8_t *key_code);

#endif // __FIFO_H