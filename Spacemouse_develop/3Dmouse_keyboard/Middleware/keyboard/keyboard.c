#include "keyboard.h"

//��Ҫ�Ļص�����ж����mode_manager�
// ================== ģ���ڲ�˽�ж��� ==================

// I2Cͨ�����Ŷ��� (��Main.c����)
#define CH450_SCL_PORT      GPIOB
#define CH450_SCL_PIN       GPIO_Pin_0
#define CH450_SDA_PORT      GPIOB
#define CH450_SDA_PIN       GPIO_Pin_1

// CH450�ж����Ŷ��� (��Main.c����)
#define CH450_INT_PORT      GPIOB
#define CH450_INT_PIN       GPIO_Pin_7

// ��I2C�����FIFOʵ������Ϊģ���ڲ��ľ�̬ȫ�ֱ������ⲿ�޷�����
I2C_HandleDef g_wai2c_keyboard;
KeyCodeFifo_t g_key_fifo;

// ================== ��������ʵ�� =================

void Keyboard_Init(void)
{
    // 1. ��ʼ��FIFO
    FIFO_Init(&g_key_fifo);

    // 2. ��ʼ��CH450��I2Cͨ��
    // ע�⣺�����wai2c���ʹ����ģ���ڵľ�̬����
    CH450_Init(&g_wai2c_keyboard, CH450_SCL_PORT, CH450_SCL_PIN, CH450_SDA_PORT, CH450_SDA_PIN, 1);
    
    // 3. ��ʼ��CH450���ж����� (ʹ�����ײ�������GPIO�жϳ�ʼ��)
    GPIOB_ModeCfg(CH450_INT_PIN, GPIO_ModeIN_PU);
    GPIOB_ITModeCfg(CH450_INT_PIN, GPIO_ITMode_FallEdge);

    // 4. �����ж����ȼ���ʹ��
    PFIC_SetPriority(GPIO_B_IRQn, 0); // ���ȼ����Ը�����Ҫ����
    PFIC_EnableIRQ(GPIO_B_IRQn);

    // 5. ����CH450����ʾ�ͼ��̹���
    CH450_Write_Cmd(&g_wai2c_keyboard, CH450_SYSON2);
}

bool Keyboard_GetKeyEvent(KeyEvent_t *pEvent)
{
    if (pEvent == NULL) return false;

    uint8_t raw_key_code;
    // ���Դ�FIFO��ȡһ��ԭʼ��ֵ
    if (FIFO_Get(&g_key_fifo, &raw_key_code) == 0) // 0 ��ʾ�ɹ�
    {
        // ����ԭʼ��ֵ���ṹ��
        pEvent->pressed = (raw_key_code & 0x40) ? true : false;
        pEvent->row = (raw_key_code >> 3) & 0x07;
        pEvent->col = raw_key_code & 0x07;

        // CH450�������2��ʼ������ת��Ϊ��0��ʼ
        if (pEvent->col >= 2)
        {
            pEvent->col -= 2;
        }
        
        return true; // �ɹ���ȡ��������һ���¼�
    }

    return false; // FIFOΪ�գ�û���¼�
}
