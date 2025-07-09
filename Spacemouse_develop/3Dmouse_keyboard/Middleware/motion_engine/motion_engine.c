#include "motion_engine.h"

// ����һЩģ��˽�еĳ���
#define DEG_TO_RAD 0.0174532925f
#define RAD_TO_DEG 57.295779513f

// �ֶ������ĵش�У׼��
const float MAG_OFFSET_X = -2.66f;
const float MAG_OFFSET_Y = 21.06f;
const float MAG_OFFSET_Z = 12.12f;
const float MAG_SENSITIVITY = 0.15f;


int MotionEngine_Init(MotionEngine_State* engine) {
    if (mpu9250_basic_init(MPU9250_INTERFACE_IIC, MPU9250_ADDRESS_AD0_LOW) != 0) {
        return 0; // ��ʼ��ʧ��
    }
    // ע�⣺���ﴫ���� engine �ڲ��� bias ����
    simple_calibrate(1000, engine->gyro_bias, engine->accel_bias); 
    
    // ��ʼ�� AHRS �㷨
    ahrs_madgwick_init(&engine->ahrs);
    engine->ahrs.beta = 0.1f;
    
    return 1; // ��ʼ���ɹ�
}

void MotionEngine_Update(MotionEngine_State* engine,float interval) {
    float dt = interval; 

    // 1. ��ȡ����������
    if (mpu9250_basic_read(engine->g, engine->dps, engine->ut) == 0) {
        
        // 2. Ӧ�����У׼
        engine->dps[0] -= engine->gyro_bias[0];
        engine->dps[1] -= engine->gyro_bias[1];
        engine->dps[2] -= engine->gyro_bias[2];
        
        engine->ut[0] -= MAG_OFFSET_X * MAG_SENSITIVITY;
        engine->ut[1] -= MAG_OFFSET_Y * MAG_SENSITIVITY;
        engine->ut[2] -= MAG_OFFSET_Z * MAG_SENSITIVITY;
        
        // 3. ���������ں��㷨
        ahrs_madgwick_update(&engine->ahrs, 
                             engine->dps[0] * DEG_TO_RAD, engine->dps[1] * DEG_TO_RAD, engine->dps[2] * DEG_TO_RAD,
                             engine->g[0], engine->g[1], engine->g[2],
                             -engine->ut[1], -engine->ut[0], -engine->ut[2],
                             dt);

        // 4. ����Ԫ������ŷ���ǲ��洢�ڽṹ����
        float qw = engine->ahrs.q[0], qx = engine->ahrs.q[1], qy = engine->ahrs.q[2], qz = engine->ahrs.q[3];
        engine->euler_angles[0] = atan2f(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy)) * RAD_TO_DEG; // Roll
        engine->euler_angles[1] = asinf(-2.0f * (qx * qz - qw * qy)) * RAD_TO_DEG;                                // Pitch
        engine->euler_angles[2] = atan2f(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz)) * RAD_TO_DEG; // Yaw
        engine->euler_angles[2] -= 6.0f; // ��ƫ��У��
    }
}

void MotionEngine_GetEulerAngles(MotionEngine_State* engine, float* angles) {
    angles[0] = engine->euler_angles[0];
    angles[1] = engine->euler_angles[1];
    angles[2] = engine->euler_angles[2];
}