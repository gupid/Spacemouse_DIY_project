#include "CH58x_common.h"
#include "driver_mpu9250_basic.h"
#include "ahrs_cal_fuse.h"
//#include "TIM.h" // --- 新增 ---: 包含您提供的定时器库头文件
#include "main.h"
#include <math.h>
#include <stdio.h>

/*********************************************************************
 * 全局变量 (Global Variables)
 *********************************************************************/
// --- MPU9250 相关变量 ---
float g[3];
float dps[3];
float ut[3];
float gyro_bias[3] = {0.0f, 0.0f, 0.0f};
float accel_bias[3] = {0.0f, 0.0f, 0.0f};

// --- AHRS 算法相关变量 ---
MadgwickState ahrs;
const float FUSION_UPDATE_RATE = 100.0f; // 设置数据融合更新频率为 100Hz
const float dt = 1.0f / FUSION_UPDATE_RATE; // dt现在是一个固定值

// --- 新增: 中断标志位 ---
volatile uint8_t g_ahrs_update_flag = 0;

//手动测量的地磁校准量
const float MAG_OFFSET_X = -2.66f;
const float MAG_OFFSET_Y = 21.06f;
const float MAG_OFFSET_Z = 12.12f;
// 磁力计灵敏度 (uT / LSB)
const float MAG_SENSITIVITY = 0.15f;

/*********************************************************************
 * @fn      WA_TIM_PeriodCallBack
 *
 * @brief   定时器周期中断回调函数 (我们需要重写这个__weak_symbol函数)
 *
 * @param   watime - 定时器句柄
 *
 * @return  none
 */
void WA_TIM_PeriodCallBack(TIM_HandleDef *watime)
{
    // 判断是否是我们用于AHRS的定时器TMR1触发了中断
    if (watime == &watime1)
    {
        // 在中断里只做最简单的事：设置标志位
        g_ahrs_update_flag = 1;
    }
}


/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 *********************************************************************/
int main()
{
    // CH58x 硬件初始化
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
    
    // --- 步骤3: 初始化 AHRS 滤波器 (与之前相同) ---
    ahrs_madgwick_init(&ahrs);
    ahrs.beta=0.1f;
    
    // --- 新增: 步骤4: 初始化定时器TMR1用于周期性中断 ---
    //PRINT("初始化 TMR1 以 %.1f Hz 频率触发融合...\n", FUSION_UPDATE_RATE);
    WA_TIM_IT_Init(&watime1, WA_TIMER, Pin_Disable, (uint32_t)FUSION_UPDATE_RATE, null, null);
    TMR1_Enable(); // 启动定时器

    while(1)
    {
        // --- 修改: 主循环现在由中断标志位驱动 ---
        if (g_ahrs_update_flag) 
        {
            g_ahrs_update_flag = 0; // 清除标志位

            // a. 读取传感器数据
            if (mpu9250_basic_read(g, dps, ut) == 0) 
            {
                //b. 应用软件校准
                // g[0] -= accel_bias[0];
                // g[1] -= accel_bias[1];
                // g[2] -= accel_bias[2];

                //地磁场偏置测量
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
                // --- 应用我们最终的、可靠的磁力计校准 ---
                ut[0] -= MAG_OFFSET_X*MAG_SENSITIVITY;
                ut[1] -= MAG_OFFSET_Y*MAG_SENSITIVITY;
                ut[2] -= MAG_OFFSET_Z*MAG_SENSITIVITY;

                // c. 调用数据融合算法 (dt 现在是固定的)
                const float dps_to_rads = 0.0174532925f;
                ahrs_madgwick_update(&ahrs, 
                                     dps[0] * dps_to_rads, dps[1] * dps_to_rads, dps[2] * dps_to_rads,
                                     g[0], g[1], g[2],
                                     -ut[1], -ut[0], -ut[2],
                                     dt);

                // d. 从四元数计算欧拉角并打印
                float qw = ahrs.q[0], qx = ahrs.q[1], qy = ahrs.q[2], qz = ahrs.q[3];
                float roll  = atan2f(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy)) * 57.295779513f;
                float pitch = asinf(-2.0f * (qx * qz - qw * qy)) * 57.295779513f;
                float yaw   = atan2f(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz)) * 57.295779513f;
                yaw += 17.0;
                //PRINT("Pitch: %0.2f, Roll: %0.2f, Yaw: %0.2f\n", pitch, roll, yaw);
                // --- 新增: 按照 FireWater 协议格式化并发送 ---
                char firewater_buf[100]; 
                // 使用 snprintf 安全地将数据格式化为 FireWater 协议字符串
                // 格式为 "名称1:数值1,名称2:数值2,..." 结尾加回车换行"\r\n"
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