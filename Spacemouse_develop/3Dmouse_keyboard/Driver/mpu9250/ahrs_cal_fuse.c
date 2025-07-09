#include "ahrs_cal_fuse.h"
#include <math.h>
#include "driver_mpu9250_basic.h"
// MPU9250 寄存器地址定义
#define PWR_MGMT_1       0x6B
#define PWR_MGMT_2       0x6C
#define INT_ENABLE       0x38
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24
#define USER_CTRL        0x6A
#define CONFIG           0x1A
#define SMPLRT_DIV       0x19
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define FIFO_COUNTH      0x72
#define FIFO_R_W         0x74
#define MPU9250_ADDRESS  0x68 // 假设 AD0 为低电平

uint8_t ahrs_calibrate_mpu9250(const AhrsPlatformApi* api, float* gyro_bias, float* accel_bias)
{
    if (api == NULL || api->i2c_write == NULL || api->i2c_read == NULL || api->delay_ms == NULL) {
        return 1; // 平台接口未提供
    }

    uint8_t data[12];
    uint16_t ii, packet_count, fifo_count;
    int32_t gyro_bias_raw[3] = {0, 0, 0}, accel_bias_raw[3] = {0, 0, 0};
    uint8_t temp_data;

    // 1. 重置设备
    temp_data = 0x80;
    api->i2c_write(MPU9250_ADDRESS, PWR_MGMT_1, &temp_data, 1);
    api->delay_ms(100);

    // 2. 配置时钟源和电源管理
    temp_data = 0x01;
    api->i2c_write(MPU9250_ADDRESS, PWR_MGMT_1, &temp_data, 1);
    temp_data = 0x00;
    api->i2c_write(MPU9250_ADDRESS, PWR_MGMT_2, &temp_data, 1);
    api->delay_ms(200);

    // 3. 配置设备用于偏置计算
    temp_data = 0x00;
    api->i2c_write(MPU9250_ADDRESS, INT_ENABLE, &temp_data, 1);
    api->i2c_write(MPU9250_ADDRESS, FIFO_EN, &temp_data, 1);
    api->i2c_write(MPU9250_ADDRESS, PWR_MGMT_1, &temp_data, 1);
    api->i2c_write(MPU9250_ADDRESS, I2C_MST_CTRL, &temp_data, 1);
    api->i2c_write(MPU9250_ADDRESS, USER_CTRL, &temp_data, 1);
    temp_data = 0x0C; // 重置 FIFO 和 DMP
    api->i2c_write(MPU9250_ADDRESS, USER_CTRL, &temp_data, 1);
    api->delay_ms(15);

    // 4. 配置传感器为最高灵敏度
    temp_data = 0x01;
    api->i2c_write(MPU9250_ADDRESS, CONFIG, &temp_data, 1);
    temp_data = 0x00;
    api->i2c_write(MPU9250_ADDRESS, SMPLRT_DIV, &temp_data, 1);
    api->i2c_write(MPU9250_ADDRESS, GYRO_CONFIG, &temp_data, 1); // ±250dps
    api->i2c_write(MPU9250_ADDRESS, ACCEL_CONFIG, &temp_data, 1); // ±2g

    uint16_t gyrosensitivity  = 131;   // LSB/dps
    uint16_t accelsensitivity = 16384; // LSB/g

    // 5. 启用FIFO并采集数据
    temp_data = 0x40;
    api->i2c_write(MPU9250_ADDRESS, USER_CTRL, &temp_data, 1);
    temp_data = 0x78; // 启用加速度计和陀螺仪的FIFO
    api->i2c_write(MPU9250_ADDRESS, FIFO_EN, &temp_data, 1);
    api->delay_ms(40); // 采集40个样本

    // 6. 停止FIFO并读取数据
    temp_data = 0x00;
    api->i2c_write(MPU9250_ADDRESS, FIFO_EN, &temp_data, 1);
    api->i2c_read(MPU9250_ADDRESS, FIFO_COUNTH, data, 2);
    fifo_count = ((uint16_t)data[0] << 8) | data[1];
    packet_count = fifo_count / 12;

    for (ii = 0; ii < packet_count; ii++) {
        int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
        api->i2c_read(MPU9250_ADDRESS, FIFO_R_W, data, 12);
        
        accel_temp[0] = (int16_t)(((int16_t)data[0] << 8) | data[1]);
        accel_temp[1] = (int16_t)(((int16_t)data[2] << 8) | data[3]);
        accel_temp[2] = (int16_t)(((int16_t)data[4] << 8) | data[5]);
        gyro_temp[0]  = (int16_t)(((int16_t)data[6] << 8) | data[7]);
        gyro_temp[1]  = (int16_t)(((int16_t)data[8] << 8) | data[9]);
        gyro_temp[2]  = (int16_t)(((int16_t)data[10] << 8) | data[11]);
        
        accel_bias_raw[0] += (int32_t)accel_temp[0];
        accel_bias_raw[1] += (int32_t)accel_temp[1];
        accel_bias_raw[2] += (int32_t)accel_temp[2];
        gyro_bias_raw[0]  += (int32_t)gyro_temp[0];
        gyro_bias_raw[1]  += (int32_t)gyro_temp[1];
        gyro_bias_raw[2]  += (int32_t)gyro_temp[2];
    }

    accel_bias_raw[0] /= (int32_t)packet_count;
    accel_bias_raw[1] /= (int32_t)packet_count;
    accel_bias_raw[2] /= (int32_t)packet_count;
    gyro_bias_raw[0]  /= (int32_t)packet_count;
    gyro_bias_raw[1]  /= (int32_t)packet_count;
    gyro_bias_raw[2]  /= (int32_t)packet_count;

    // 7. 移除Z轴重力影响
    if (accel_bias_raw[2] > 0L) {
        accel_bias_raw[2] -= (int32_t)accelsensitivity;
    } else {
        accel_bias_raw[2] += (int32_t)accelsensitivity;
    }

    // 8. 转换为物理单位并返回
    gyro_bias[0] = (float)gyro_bias_raw[0] / (float)gyrosensitivity;
    gyro_bias[1] = (float)gyro_bias_raw[1] / (float)gyrosensitivity;
    gyro_bias[2] = (float)gyro_bias_raw[2] / (float)gyrosensitivity;
    accel_bias[0] = (float)accel_bias_raw[0] / (float)accelsensitivity;
    accel_bias[1] = (float)accel_bias_raw[1] / (float)accelsensitivity;
    accel_bias[2] = (float)accel_bias_raw[2] / (float)accelsensitivity;
    
    return 0;
}


void simple_calibrate(uint16_t sample_count, float* gyro_bias_out, float* accel_bias_out)
{
    float g_temp[3], dps_temp[3], ut_temp[3];
    float gyro_sum[3] = {0.0f, 0.0f, 0.0f};
    float accel_sum[3] = {0.0f, 0.0f, 0.0f};

    printf("简易校准开始，请保持设备水平静止...\n");
    printf("正在采集 %d 次样本...\n", sample_count);

    // 1. 连续读取数据并累加
    for (uint16_t i = 0; i < sample_count; i++)
    {
        if (mpu9250_basic_read(g_temp, dps_temp, ut_temp) == 0)
        {
            accel_sum[0] += g_temp[0];
            accel_sum[1] += g_temp[1];
            accel_sum[2] += g_temp[2];

            gyro_sum[0] += dps_temp[0];
            gyro_sum[1] += dps_temp[1];
            gyro_sum[2] += dps_temp[2];
        }
        mpu9250_interface_delay_ms(10); // 每次读取之间稍作延时，确保获取到新的样本
    }

    // 2. 计算平均值
    for (int i = 0; i < 3; i++)
    {
        accel_bias_out[i] = accel_sum[i] / sample_count;
        gyro_bias_out[i] = gyro_sum[i] / sample_count;
    }

    // 3. 移除加速度计Z轴的重力分量
    // 假设设备是水平放置的，Z轴的读数应该是 1g。这个值不是偏置，而是重力。
    // 真正的偏置是读数与1g之间的差值。
    accel_bias_out[2] = accel_bias_out[2] - 1.0f; 

    printf("简易校准完成。\n");
}

void ahrs_madgwick_init(MadgwickState* state) {
    if (state == NULL) return;
    state->q[0] = 1.0f;
    state->q[1] = 0.0f;
    state->q[2] = 0.0f;
    state->q[3] = 0.0f;
    state->beta = 0.1f; // 默认增益
}

void ahrs_madgwick_update(MadgwickState* state, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt)
{
    if (state == NULL) return;

    float q1 = state->q[0], q2 = state->q[1], q3 = state->q[2], q4 = state->q[3];
    float norm;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;
    float hx, hy;
    float _2q1mx, _2q1my, _2q1mz, _2q2mx;
    float _2bx, _2bz;
    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _2q4 = 2.0f * q4;
    float _2q1q3 = 2.0f * q1 * q3;
    float _2q3q4 = 2.0f * q3 * q4;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    norm = sqrtf(ax * ax + ay * ay + az * az);
    if (norm == 0.0f) return;
    norm = 1.0f / norm;
    ax *= norm;
    ay *= norm;
    az *= norm;

    norm = sqrtf(mx * mx + my * my + mz * mz);
    if (norm == 0.0f) return;
    norm = 1.0f / norm;
    mx *= norm;
    my *= norm;
    mz *= norm;

    _2q1mx = 2.0f * q1 * mx;
    _2q1my = 2.0f * q1 * my;
    _2q1mz = 2.0f * q1 * mz;
    _2q2mx = 2.0f * q2 * mx;
    hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
    hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
    _2bx = sqrtf(hx * hx + hy * hy);
    _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;

    s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - 2.0f * _2bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-2.0f * _2bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - 2.0f * _2bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-2.0f * _2bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    norm = sqrtf(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);
    norm = 1.0f/norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    s4 *= norm;

    qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - state->beta * s1;
    qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - state->beta * s2;
    qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - state->beta * s3;
    qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - state->beta * s4;

    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;
    q4 += qDot4 * dt;
    norm = sqrtf(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
    norm = 1.0f/norm;
    state->q[0] = q1 * norm;
    state->q[1] = q2 * norm;
    state->q[2] = q3 * norm;
    state->q[3] = q4 * norm;
}

void ahrs_mahony_init(MahonyState* state) {
    if (state == NULL) return;
    state->q[0] = 1.0f;
    state->q[1] = 0.0f;
    state->q[2] = 0.0f;
    state->q[3] = 0.0f;
    state->eInt[0] = 0.0f;
    state->eInt[1] = 0.0f;
    state->eInt[2] = 0.0f;
    state->Kp = 10.0f; // 默认Kp
    state->Ki = 0.0f;  // 默认Ki
}

void ahrs_mahony_update(MahonyState* state, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt)
{
    if (state == NULL) return;

    float q1 = state->q[0], q2 = state->q[1], q3 = state->q[2], q4 = state->q[3];
    float norm;
    float hx, hy, bx, bz;
    float vx, vy, vz, wx, wy, wz;
    float ex, ey, ez;
    float pa, pb, pc;

    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    norm = sqrtf(ax * ax + ay * ay + az * az);
    if (norm == 0.0f) return;
    norm = 1.0f / norm;
    ax *= norm;
    ay *= norm;
    az *= norm;

    norm = sqrtf(mx * mx + my * my + mz * mz);
    if (norm == 0.0f) return;
    norm = 1.0f / norm;
    mx *= norm;
    my *= norm;
    mz *= norm;

    hx = 2.0f * mx * (0.5f - q3q3 - q4q4) + 2.0f * my * (q2q3 - q1q4) + 2.0f * mz * (q2q4 + q1q3);
    hy = 2.0f * mx * (q2q3 + q1q4) + 2.0f * my * (0.5f - q2q2 - q4q4) + 2.0f * mz * (q3q4 - q1q2);
    bx = sqrtf(hx * hx + hy * hy);
    bz = 2.0f * mx * (q2q4 - q1q3) + 2.0f * my * (q3q4 + q1q2) + 2.0f * mz * (0.5f - q2q2 - q3q3);

    vx = 2.0f * (q2q4 - q1q3);
    vy = 2.0f * (q1q2 + q3q4);
    vz = q1q1 - q2q2 - q3q3 + q4q4;
    wx = 2.0f * bx * (0.5f - q3q3 - q4q4) + 2.0f * bz * (q2q4 - q1q3);
    wy = 2.0f * bx * (q2q3 - q1q4) + 2.0f * bz * (q1q2 + q3q4);
    wz = 2.0f * bx * (q1q3 + q2q4) + 2.0f * bz * (0.5f - q2q2 - q3q3);

    ex = (ay * vz - az * vy) + (my * wz - mz * wy);
    ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
    ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
    if (state->Ki > 0.0f)
    {
        state->eInt[0] += ex;
        state->eInt[1] += ey;
        state->eInt[2] += ez;
    }
    else
    {
        state->eInt[0] = 0.0f;
        state->eInt[1] = 0.0f;
        state->eInt[2] = 0.0f;
    }

    gx = gx + state->Kp * ex + state->Ki * state->eInt[0];
    gy = gy + state->Kp * ey + state->Ki * state->eInt[1];
    gz = gz + state->Kp * ez + state->Ki * state->eInt[2];

    pa = q2;
    pb = q3;
    pc = q4;
    q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * dt);
    q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * dt);
    q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * dt);
    q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * dt);

    norm = sqrtf(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
    norm = 1.0f / norm;
    state->q[0] = q1 * norm;
    state->q[1] = q2 * norm;
    state->q[2] = q3 * norm;
    state->q[3] = q4 * norm;
}