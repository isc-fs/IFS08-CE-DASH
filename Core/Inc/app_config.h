#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
  APP_CONFIG_KEY_SD_LOG_ENABLE = 2U,
  APP_CONFIG_KEY_VEHICLE_ALERT_0 = 16U,
  APP_CONFIG_KEY_VEHICLE_ALERT_1 = 17U,
  APP_CONFIG_KEY_VEHICLE_ALERT_2 = 18U,
  APP_CONFIG_KEY_VEHICLE_ALERT_3 = 19U,
  APP_CONFIG_KEY_VEHICLE_ALERT_4 = 20U
} AppConfigKey;

enum
{
  VEHICLE_CONFIG_ALERT_PARAM_COUNT = 5U,
  VEHICLE_CONFIG_ALERT_0_MAX = 180U,
  VEHICLE_CONFIG_ALERT_1_MAX = 160U,
  VEHICLE_CONFIG_ALERT_2_MAX = 100U,
  VEHICLE_CONFIG_ALERT_3_MAX = 100U,
  VEHICLE_CONFIG_ALERT_4_MAX = 450U
};

typedef struct
{
  uint16_t alertThresholds[VEHICLE_CONFIG_ALERT_PARAM_COUNT];
} VehicleConfigParams;

uint8_t AppConfig_IsLocalSdLoggingEnabled(void);
uint8_t AppConfig_IsSdLogRecordingActive(void);
uint8_t AppConfig_SetLocalSdLoggingEnabled(uint8_t enabled);
void AppConfig_RequestNewSdLogSession(void);
void AppConfig_CopySdLogSessionId(char *buffer, uint32_t bufferSize);
void AppConfig_GetVehicleConfigParams(VehicleConfigParams *params);
uint16_t AppConfig_GetVehicleAlertThreshold(uint8_t index);
uint8_t AppConfig_SetVehicleAlertThreshold(uint8_t index, uint16_t value);
uint8_t AppConfig_SendCanConfigU8(uint8_t key, uint8_t value);
uint8_t AppConfig_SendCanConfigU16(uint8_t key, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
