#ifndef __RF_MIDDLEWARE_H
#define __RF_MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief RF数据接收回调函数类型定义
 *
 * @param data   接收到的数据缓冲区指针
 * @param length 数据长度
 * @param rssi   信号强度指示 (RSSI)
 */
typedef void (*rf_mw_data_callback_t)(const uint8_t *data, uint8_t length, int8_t rssi);

/**
 * @brief 初始化RF中间件
 *
 * @param accessAddress  RF通信的接入地址 (通信双方必须一致)
 * @param channel        RF通信频点 (0-83)
 * @param data_cb        数据接收回调函数
 *
 * @note 此函数应在系统和HAL初始化之后，主循环之前调用。
 */
void RF_MW_Init(uint32_t accessAddress, uint8_t channel, rf_mw_data_callback_t data_cb);

/**
 * @brief 通过RF发送数据
 *
 * @param data    要发送的数据缓冲区指针
 * @param length  要发送的数据长度 (注意：长度不应超过硬件支持的最大长度)
 *
 * @return 成功返回 true, 失败 (如中间件正忙) 返回 false.
 */
bool RF_MW_SendData(const uint8_t *data, uint8_t length);

/**
 * @brief 让设备进入或退出接收状态
 *
 * @param enable  true: 启动接收; false: 停止接收 (用于功耗管理)
 */
void RF_MW_EnableReceiver(bool enable);



#ifdef __cplusplus
}
#endif

#endif // __RF_MIDDLEWARE_H