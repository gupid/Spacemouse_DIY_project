#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "main.h" // ����������ͷ�ļ�
#include "CH450_driver.h"
#include "fifo.h"

extern I2C_HandleDef g_wai2c_keyboard;
extern KeyCodeFifo_t g_key_fifo;

// ����һ���ṹ������ʾһ�������¼�
typedef struct {
    uint8_t row;        // �������к� (0-7)
    uint8_t col;        // �������к� (0-5)
    bool pressed;       // ����״̬: trueΪ����, falseΪ�ͷ�
} KeyEvent_t;

/**
 * @brief  ��ʼ������ģ��
 * @note   �˺�����������б�Ҫ�ĵײ��ʼ��������GPIO, I2C, �ж��Լ�CH450оƬ��
 * @return ��
 */
void Keyboard_Init(void);

/**
 * @brief  �Ӽ����¼������л�ȡһ�������¼�
 * @param  pEvent - ָ�� KeyEvent_t �ṹ���ָ�룬���ڴ洢��ȡ�����¼�
 * @return bool   - true: �ɹ���ȡ��һ���¼�; false: ��ǰû�а����¼�
 */
bool Keyboard_GetKeyEvent(KeyEvent_t *pEvent);

#endif // __KEYBOARD_H