#ifndef __DISPLAY_TELEMETRY_CAN_CONFIG_H__
#define __DISPLAY_TELEMETRY_CAN_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "display_telemetry.h"
#include <stddef.h>
#include <stdint.h>

typedef enum
{
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8 = 0,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_BIT,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_BE,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_BE,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_U32_LE,
  DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE
} DisplayTelemetrySignalEncoding;

typedef struct
{
  const char *parameterName;
  size_t telemetryOffset;
  uint8_t telemetrySize;
  uint8_t payloadOffset;
  uint8_t minPayloadLength;
  uint8_t maxPayloadLength;
  uint8_t bitMask;
  DisplayTelemetrySignalEncoding encoding;
} DisplayTelemetrySignalConfig;

typedef struct
{
  const char *name;
  uint32_t canId;
  uint32_t snapshotMask;
  uint8_t minPayloadLength;
  uint8_t maxPayloadLength;
  const DisplayTelemetrySignalConfig *signals;
  uint32_t signalCount;
} DisplayTelemetryCanMessageConfig;

const DisplayTelemetryCanMessageConfig *DisplayTelemetryCanConfig_GetAll(uint32_t *out_count);
const DisplayTelemetryCanMessageConfig *DisplayTelemetryCanConfig_FindById(uint32_t canId);
uint32_t DisplayTelemetryCanConfig_GetMinId(void);
uint32_t DisplayTelemetryCanConfig_GetMaxId(void);
uint32_t DisplayTelemetryCanConfig_GetSnapshotMaskAll(void);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_TELEMETRY_CAN_CONFIG_H__ */
