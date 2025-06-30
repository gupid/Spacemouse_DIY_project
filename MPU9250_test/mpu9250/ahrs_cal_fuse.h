#ifndef AHRS_CAL_FUSE_H
#define AHRS_CAL_FUSE_H

#include <stdint.h> // 用于 uint8_t, int16_t 等类型
#include <stddef.h> // 用于 NULL

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 平台相关的硬件接口函数指针结构体
 * @note  您需要在您的主项目中实现这些函数，并将它们的地址提供给校准函数。
 */
typedef struct {
    /**
     * @brief I2C 写入函数指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 指向要写入数据的指针
     * @param len 要写入的数据长度
     * @return 0 表示成功, 非0 表示失败
     */
    uint8_t (*i2c_write)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

    /**
     * @brief I2C 读取函数指针
     * @param dev_addr 7位设备地址
     * @param reg_addr 寄存器地址
     * @param data 指向存储读取数据的指针
     * @param len 要读取的数据长度
     * @return 0 表示成功, 非0 表示失败
     */
    uint8_t (*i2c_read)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

    /**
     * @brief 毫秒延时函数指针
     * @param ms 要延时的毫秒数
     */
    void (*delay_ms)(uint32_t ms);
} AhrsPlatformApi;


/**
 * @brief Madgwick 算法状态结构体
 */
typedef struct {
    float q[4];   // 四元数 {w, x, y, z}
    float beta;   // 滤波器增益, 控制陀螺仪误差的收敛速度, 典型值 0.1f
} MadgwickState;

/**
 * @brief Mahony 算法状态结构体
 */
typedef struct {
    float q[4];   // 四元数 {w, x, y, z}
    float eInt[3]; // 积分误差项 {x, y, z}
    float Kp;     // 比例增益, 典型值 2.0f * 5.0f
    float Ki;     // 积分增益, 典型值 0.0f
} MahonyState;


/**
 * @brief  对 MPU9250 的加速度计和陀螺仪进行零点偏置校准
 * @param  api 指向平台硬件接口函数实现的指针
 * @param  gyro_bias 指向一个 float[3] 数组，用于存储计算出的陀螺仪偏置 (dps)
 * @param  accel_bias 指向一个 float[3] 数组，用于存储计算出的加速度计偏置 (g)
 * @return 0 表示成功, 非0 表示失败
 * @note   执行此函数时，请确保 MPU9250 处于水平静止状态。
 */
uint8_t ahrs_calibrate_mpu9250(const AhrsPlatformApi* api, float* gyro_bias, float* accel_bias);

/**
 * @brief 初始化 Madgwick 滤波器状态
 * @param state 指向 MadgwickState 结构体的指针
 */
void ahrs_madgwick_init(MadgwickState* state);

/**
 * @brief 更新 Madgwick 滤波器
 * @param state 指向 MadgwickState 结构体的指针
 * @param gx, gy, gz 陀螺仪数据 (单位: rad/s)
 * @param ax, ay, az 加速度计数据 (单位: m/s^2, 任何单位都可以，因为会被归一化)
 * @param mx, my, mz 磁力计数据 (单位: uT, 任何单位都可以，因为会被归一化)
 * @param dt 两次更新之间的时间差 (单位: s)
 */
void ahrs_madgwick_update(MadgwickState* state, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt);

/**
 * @brief 初始化 Mahony 滤波器状态
 * @param state 指向 MahonyState 结构体的指针
 */
void ahrs_mahony_init(MahonyState* state);

/**
 * @brief 更新 Mahony 滤波器
 * @param state 指向 MahonyState 结构体的指针
 * @param gx, gy, gz 陀螺仪数据 (单位: rad/s)
 * @param ax, ay, az 加速度计数据 (单位: m/s^2, 任何单位都可以，因为会被归一化)
 * @param mx, my, mz 磁力计数据 (单位: uT, 任何单位都可以，因为会被归一化)
 * @param dt 两次更新之间的时间差 (单位: s)
 */
void ahrs_mahony_update(MahonyState* state, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt);

/**
 * @brief  使用 mpu9250_basic_read 进行简化的零点偏置校准
 * @param  sample_count 校准所需的采样次数，建议 200 次以上
 * @param  gyro_bias_out 指向用于存储陀螺仪偏置结果的数组 (float[3])
 * @param  accel_bias_out 指向用于存储加速度计偏置结果的数组 (float[3])
 * @note   执行此函数时，务必保持 MPU9250 水平且完全静止。
 */
void simple_calibrate(uint16_t sample_count, float* gyro_bias_out, float* accel_bias_out);
#ifdef __cplusplus
}
#endif

#endif // AHRS_CAL_FUSE_H