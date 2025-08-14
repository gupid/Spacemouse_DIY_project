// rf_middleware.h

#ifndef __RF_MIDDLEWARE_H__
#define __RF_MIDDLEWARE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

// 定义最大支持的数据负载长度
#define RF_MW_MAX_PAYLOAD_SIZE      64

/**
 * @brief RF数据接收回调函数类型定义
 * * @param data      接收到的数据指针
 * @param length    数据长度
 * @param rssi      接收信号强度指示 (Received Signal Strength Indication)
 */
typedef void (*rf_mw_data_callback_t)(const uint8_t *data, uint8_t length, int8_t rssi);

/**
 * @brief 初始化RF中间件
 *
 * @param data_cb   数据接收回调函数。当模块接收到有效数据时，将调用此函数。
 */
void RF_MW_Init(rf_mw_data_callback_t data_cb);

/**
 * @brief 发送数据
 * @note  这是一个非阻塞函数。函数会立即返回。发送完成后，模块会自动切换回接收模式。
 *
 * @param data      要发送的数据缓冲区指针
 * @param length    要发送的数据长度 (不应超过 RF_MW_MAX_PAYLOAD_SIZE)
 *
 * @return  true    - 数据已成功提交给RF驱动进行发送。
 * @return  false   - 发送失败，可能是因为当前有正在进行的发送操作或长度错误。
 */
bool RF_MW_SendData(const uint8_t *data, uint8_t length);
void RF_MW_Deinit(void);
#ifdef __cplusplus
}
#endif

#endif // __RF_MIDDLEWARE_H__