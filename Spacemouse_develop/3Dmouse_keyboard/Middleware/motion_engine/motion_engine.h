#ifndef MOTIONENGINE
#define MOTIONENGINE
#include "driver_mpu9250_basic.h"
#include "ahrs_cal_fuse.h"
#include "main.h"
#include "CH58x_common.h"
#include <math.h>
#include <stdio.h>


// ���� MotionEngine ״̬�ṹ��
typedef struct {
    // ����������
    float g[3];
    float dps[3];
    float ut[3];
    
    // ������У׼ֵ
    float gyro_bias[3];
    float accel_bias[3];

    // AHRS �㷨״̬
    MadgwickState ahrs;

    // ���ռ������ŷ����
    float euler_angles[3];

} MotionEngine_State;

// ��ʼ�� MotionEngine ģ��
// ����0��ʾʧ�ܣ�1��ʾ�ɹ�
int MotionEngine_Init(MotionEngine_State* engine);

// ������̬���ݣ����Ǻ��Ĵ�������
void MotionEngine_Update(MotionEngine_State* engine, float interval);

// ��ȡ���µ�ŷ����
void MotionEngine_GetEulerAngles(MotionEngine_State* engine, float* angles);


#endif