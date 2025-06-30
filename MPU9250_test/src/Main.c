#include "CH58x_common.h"
#include "driver_mpu9250_basic.h"
#include "ahrs_cal_fuse.h"
//#include "TIM.h" // --- ���� ---: �������ṩ�Ķ�ʱ����ͷ�ļ�
#include "main.h"
#include <math.h>
#include <stdio.h>

/*********************************************************************
 * ȫ�ֱ��� (Global Variables)
 *********************************************************************/
// --- MPU9250 ��ر��� ---
float g[3];
float dps[3];
float ut[3];
float gyro_bias[3] = {0.0f, 0.0f, 0.0f};
float accel_bias[3] = {0.0f, 0.0f, 0.0f};

// --- AHRS �㷨��ر��� ---
MadgwickState ahrs;
const float FUSION_UPDATE_RATE = 100.0f; // ���������ںϸ���Ƶ��Ϊ 100Hz
const float dt = 1.0f / FUSION_UPDATE_RATE; // dt������һ���̶�ֵ

// --- ����: �жϱ�־λ ---
volatile uint8_t g_ahrs_update_flag = 0;

//�ֶ������ĵش�У׼��
const float MAG_OFFSET_X = -2.66f;
const float MAG_OFFSET_Y = 21.06f;
const float MAG_OFFSET_Z = 12.12f;
// ������������ (uT / LSB)
const float MAG_SENSITIVITY = 0.15f;

/*********************************************************************
 * @fn      WA_TIM_PeriodCallBack
 *
 * @brief   ��ʱ�������жϻص����� (������Ҫ��д���__weak_symbol����)
 *
 * @param   watime - ��ʱ�����
 *
 * @return  none
 */
void WA_TIM_PeriodCallBack(TIM_HandleDef *watime)
{
    // �ж��Ƿ�����������AHRS�Ķ�ʱ��TMR1�������ж�
    if (watime == &watime1)
    {
        // ���ж���ֻ����򵥵��£����ñ�־λ
        g_ahrs_update_flag = 1;
    }
}


/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 *********************************************************************/
int main()
{
    // CH58x Ӳ����ʼ��
    HSECFG_Capacitance(HSECap_18p);
    SetSysClock(CLK_SOURCE_HSE_PLL_62_4MHz);
    WA_UART_Init(&wauart1, 115200);

    if (mpu9250_basic_init(MPU9250_INTERFACE_IIC, MPU9250_ADDRESS_AD0_LOW) != 0) {
        PRINT("mpu9250: basic init failed!\n");
        while(1);
    }

    simple_calibrate(200,gyro_bias,accel_bias);
    printf("(dps): %.4f, %.4f, %.4f\n", gyro_bias[0], gyro_bias[1], gyro_bias[2]);
    printf("(g): %.4f, %.4f, %.4f\n", accel_bias[0], accel_bias[1], accel_bias[2]);
    
    // --- ����3: ��ʼ�� AHRS �˲��� (��֮ǰ��ͬ) ---
    ahrs_madgwick_init(&ahrs);
    ahrs.beta=0.1f;
    
    // --- ����: ����4: ��ʼ����ʱ��TMR1�����������ж� ---
    //PRINT("��ʼ�� TMR1 �� %.1f Hz Ƶ�ʴ����ں�...\n", FUSION_UPDATE_RATE);
    WA_TIM_IT_Init(&watime1, WA_TIMER, Pin_Disable, (uint32_t)FUSION_UPDATE_RATE, null, null);
    TMR1_Enable(); // ������ʱ��

    while(1)
    {
        // --- �޸�: ��ѭ���������жϱ�־λ���� ---
        if (g_ahrs_update_flag) 
        {
            g_ahrs_update_flag = 0; // �����־λ

            // a. ��ȡ����������
            if (mpu9250_basic_read(g, dps, ut) == 0) 
            {
                //b. Ӧ�����У׼
                // g[0] -= accel_bias[0];
                // g[1] -= accel_bias[1];
                // g[2] -= accel_bias[2];

                //�شų�ƫ�ò���
                // char motioncal_buf[100];
                // int len = snprintf(motioncal_buf, sizeof(motioncal_buf),
                //        "Raw:%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                //        0, 0, 0,
                //        0, 0, 0,
                //        mag_raw[0], mag_raw[1], mag_raw[2]
                //       );

                // if (len > 0)
                // {
                //     UART1_SendString((uint8_t*)motioncal_buf, len);
                // }

                dps[0] -= gyro_bias[0];
                dps[1] -= gyro_bias[1];
                dps[2] -= gyro_bias[2];
                // --- Ӧ���������յġ��ɿ��Ĵ�����У׼ ---
                ut[0] -= MAG_OFFSET_X*MAG_SENSITIVITY;
                ut[1] -= MAG_OFFSET_Y*MAG_SENSITIVITY;
                ut[2] -= MAG_OFFSET_Z*MAG_SENSITIVITY;

                // c. ���������ں��㷨 (dt �����ǹ̶���)
                const float dps_to_rads = 0.0174532925f;
                ahrs_madgwick_update(&ahrs, 
                                     dps[0] * dps_to_rads, dps[1] * dps_to_rads, dps[2] * dps_to_rads,
                                     g[0], g[1], g[2],
                                     -ut[1], -ut[0], -ut[2],
                                     dt);

                // d. ����Ԫ������ŷ���ǲ���ӡ
                float qw = ahrs.q[0], qx = ahrs.q[1], qy = ahrs.q[2], qz = ahrs.q[3];
                float roll  = atan2f(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy)) * 57.295779513f;
                float pitch = asinf(-2.0f * (qx * qz - qw * qy)) * 57.295779513f;
                float yaw   = atan2f(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz)) * 57.295779513f;
                yaw += 17.0;
                //PRINT("Pitch: %0.2f, Roll: %0.2f, Yaw: %0.2f\n", pitch, roll, yaw);
                // --- ����: ���� FireWater Э���ʽ�������� ---
                char firewater_buf[100]; 
                // ʹ�� snprintf ��ȫ�ؽ����ݸ�ʽ��Ϊ FireWater Э���ַ���
                // ��ʽΪ "����1:��ֵ1,����2:��ֵ2,..." ��β�ӻس�����"\r\n"
                int len = snprintf(firewater_buf, sizeof(firewater_buf), 
                                "Roll:%0.2f,%0.2f,%0.2f\r\n", 
                                roll, pitch, yaw);
                if (len > 0)
                {
                    UART1_SendString((uint8_t*)firewater_buf, len);
                }
            }
        }
    }
}