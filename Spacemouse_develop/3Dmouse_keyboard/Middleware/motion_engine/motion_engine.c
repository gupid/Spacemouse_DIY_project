#include "motion_engine.h"

// 定义一些模块私有的常量
#define DEG_TO_RAD 0.0174532925f
#define RAD_TO_DEG 57.295779513f

// 手动测量的地磁校准量
const float MAG_OFFSET_X = -2.66f;
const float MAG_OFFSET_Y = 21.06f;
const float MAG_OFFSET_Z = 12.12f;
const float MAG_SENSITIVITY = 0.15f;

float fast_atan2f(float y, float x) {
    const float PI = 3.1415926535f;
    const float HALF_PI = 1.57079632679f;
    float abs_y = fabsf(y) + 1e-10f; // 避免除以零
    float angle;

    if (x >= 0) {
        float r = (x - abs_y) / (x + abs_y);
        angle = (PI / 4.0f) * (1.0f - r);
    } else {
        float r = (x + abs_y) / (abs_y - x);
        angle = (3.0f * PI / 4.0f) - (PI / 4.0f) * r;
    }

    return (y < 0) ? -angle : angle;
}

/**
 * @brief 使用泰勒级数展开来近似 asinf(x)
 * @param x 输入值，必须在 [-1.0, 1.0] 范围内
 * @return asinf(x) 的近似值（弧度）
 *
 * @note 优点：速度极快，无任何库函数依赖。
 * @note 缺点：当 |x| 接近 1 时，误差会变大。
 */
float fast_asinf_taylor(float x) {
    // 限制输入，增加健壮性
    if (x < -1.0f) x = -1.0f;
    if (x > 1.0f) x = 1.0f;

    float x2 = x * x; // x^2
    // 使用泰勒展开式的前三项: x + (1/6)x^3 + (3/40)x^5
    return x * (1.0f + x2 * (0.16666667f + x2 * 0.075f));
}

int MotionEngine_Init(MotionEngine_State* engine) {
    if (mpu9250_basic_init(MPU9250_INTERFACE_IIC, MPU9250_ADDRESS_AD0_LOW) != 0) {
        return 0; // 初始化失败
    }
    DelayMs(5000);
    // 注意：这里传入了 engine 内部的 bias 数组
    simple_calibrate(1000, engine->gyro_bias, engine->accel_bias); 
    
    // 初始化 AHRS 算法
    ahrs_madgwick_init(&engine->ahrs);
    engine->ahrs.beta = 0.1f;
    
    return 1; // 初始化成功
}


void MotionEngine_Update(MotionEngine_State* engine,float interval) {
    float dt = interval; 

    // 1. 读取传感器数据
    if (mpu9250_basic_read(engine->g, engine->dps, engine->ut) == 0) {
        
        // 2. 应用软件校准
        engine->dps[0] -= engine->gyro_bias[0];
        engine->dps[1] -= engine->gyro_bias[1];
        engine->dps[2] -= engine->gyro_bias[2];
        
        engine->ut[0] -= MAG_OFFSET_X * MAG_SENSITIVITY;
        engine->ut[1] -= MAG_OFFSET_Y * MAG_SENSITIVITY;
        engine->ut[2] -= MAG_OFFSET_Z * MAG_SENSITIVITY;
        //printf("Raw Mag Data -> ut[0]: %8.2f, ut[1]: %8.2f, ut[2]: %8.2f\n", engine->ut[0], engine->ut[1], engine->ut[2]);
        // 3. 调用数据融合算法
        ahrs_madgwick_update(&engine->ahrs, 
                             engine->dps[0] * DEG_TO_RAD, engine->dps[1] * DEG_TO_RAD, engine->dps[2] * DEG_TO_RAD,
                             engine->g[0], engine->g[1], engine->g[2],
                             -engine->ut[1], -engine->ut[0], -engine->ut[2],
                             dt);

        // 4. 从四元数计算欧拉角并存储在结构体中
        float qw = engine->ahrs.q[0], qx = engine->ahrs.q[1], qy = engine->ahrs.q[2], qz = engine->ahrs.q[3];
        engine->euler_angles[0] = fast_atan2f(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy)) * RAD_TO_DEG; // Roll
        engine->euler_angles[1] = fast_asinf_taylor(-2.0f * (qx * qz - qw * qy)) * RAD_TO_DEG;                                // Pitch
        engine->euler_angles[2] = fast_atan2f(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz)) * RAD_TO_DEG; // Yaw
        //engine->euler_angles[2] -= 6.0f; 
    }
}

void MotionEngine_GetEulerAngles(MotionEngine_State* engine, float* angles) {
    angles[0] = engine->euler_angles[0];
    angles[1] = engine->euler_angles[1];
    angles[2] = engine->euler_angles[2];
}