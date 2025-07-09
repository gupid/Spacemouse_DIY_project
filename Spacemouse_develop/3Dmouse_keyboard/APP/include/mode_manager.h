#ifndef __MODE_MANAGER_H__
#define __MODE_MANAGER_H__

#include "CH58x_common.h"
#include "main.h" 
typedef enum {
    MODE_NONE,
    MODE_USB,
    MODE_BLE,
    MODE_RF
} OperatingMode_t;
// º¯ÊýÉùÃ÷
void ModeManager_Init(void);
void ModeManager_MainLoop(void);
void ModeManager_RequestSwitchToUSB(void);
void ModeManager_RequestSwitchToBLE(void);
OperatingMode_t ModeManager_GetCurrentMode(void);
void ModeManager_SetUSBAttached(void);

#endif // __MODE_MANAGER_H__