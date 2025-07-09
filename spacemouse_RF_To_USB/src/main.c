// �ļ���: main.c (���ڽ�����Dongle)

#include "CONFIG.h"
#include "HAL.h"
#include "rf_middleware.h"
#include "usb_hid_composite.h"

// --- RF ���� (�����뷢������ȫһ��) ---
#define RF_ACCESS_ADDRESS   0x4B65794D  // �뷢����һ�µĵ�ַ
#define RF_CHANNEL          40          // �뷢����һ�µ��ŵ�

// --- �������� ---
void OnDataReceived_RF(const uint8_t *data, uint8_t length, int8_t rssi);

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
/*********************************************************************
 * @fn      main
 *
 * @brief   ������������
 *
 * @return  none
 */
int main(void)
{
    // --- 1. ϵͳ��ʼ�� ---
#if(defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz);

    CH58x_BLEInit();
    HAL_Init();

    // --- 2. USB HID�����豸��ʼ�� ---
    usb_hid_composite_init();
    PRINT("USB HID Composite Dongle Initialized.\n");

    // --- 3. 2.4G RFģ���ʼ�� ---
    RF_MW_Init(RF_ACCESS_ADDRESS, RF_CHANNEL, OnDataReceived_RF);
    PRINT("RF Receiver Initialized. Listening on Channel %d.\n", RF_CHANNEL);

    // --- 4. ������ѭ�� ---
    while(1)
    {
        // TMOSϵͳ������ѯ������RF������
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      OnDataReceived_RF
 *
 * @brief   RF���ݽ��ջص����� (�����߼�)
 *
 * @param   data    - ָ����յ����ݵ�ָ��
 * @param   length  - ���յ������ݳ���
 * @param   rssi    - �ź�ǿ��
 *
 * @return  none
 */
void OnDataReceived_RF(const uint8_t *data, uint8_t length, int8_t rssi)
{
    // ���ݽ��յ������ݰ����ȣ��ж������Ͳ�ת������Ӧ��USB�˵�

    if (length == 8)
    {
        // ����Ϊ8�ֽڣ��ж�Ϊ���̱���
        // ֱ��ͨ��USB�˵�1���͸�����
        usb_hid_send_report(data);
    }
    else if (length == 7)
    {
        // ����Ϊ7�ֽڣ��ж�Ϊ�ռ����� "ƽ��" �� "��ת" ����
        // data[0] �� Report ID (0x01 �� 0x02)
        // ͨ��USB�˵�2���͸�����
        usb_hid_send_spacemouse_report(data, length);
    }

}