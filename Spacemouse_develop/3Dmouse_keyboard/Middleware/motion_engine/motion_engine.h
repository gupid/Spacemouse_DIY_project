#ifndef MOTIONENGINE
#define MOTIONENGINE
#include "driver_mpu9250_basic.h"
#include "ahrs_cal_fuse.h"
#include "main.h"
#include "CH58x_common.h"
#include <math.h>
#include <stdio.h>


// 定义 MotionEngine 状态结构体
typedef struct {
    // 传感器数据
    float g[3];
    float dps[3];
    float ut[3];
    
    // 传感器校准值
    float gyro_bias[3];
    float accel_bias[3];

    // AHRS 算法状态
    MadgwickState ahrs;

    // 最终计算出的欧拉角
    float euler_angles[3];

} MotionEngine_State;

// 初始化 MotionEngine 模块
// 返回0表示失败，1表示成功
int MotionEngine_Init(MotionEngine_State* engine);

// 更新姿态数据（这是核心处理函数）
void MotionEngine_Update(MotionEngine_State* engine, float interval);

// 获取最新的欧拉角
void MotionEngine_GetEulerAngles(MotionEngine_State* engine, float* angles);


#endif