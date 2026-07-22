#include "display_telemetry_can_config.h"

#include <assert.h>

int main(void)
{
  static const uint8_t expectedDlc[] = {8U, 6U, 6U, 6U, 8U, 4U};
  static const uint32_t expectedSignals[] = {4U, 4U, 3U, 5U, 3U, 2U};
  uint32_t count = 0U;

  (void)DisplayTelemetryCanConfig_GetAll(&count);
  assert(count == 24U);
  assert(DisplayTelemetryCanConfig_GetMinId() == 0x510U);
  assert(DisplayTelemetryCanConfig_GetMaxId() == 0x527U);
  assert(DisplayTelemetryCanConfig_GetSnapshotMaskAll() == 0x00FFFFFFUL);

  for (uint32_t index = 0U; index < 6U; index++)
  {
    const DisplayTelemetryCanMessageConfig *config =
        DisplayTelemetryCanConfig_FindById(0x522U + index);
    assert(config != 0);
    assert(config->minPayloadLength == expectedDlc[index]);
    assert(config->maxPayloadLength == expectedDlc[index]);
    assert(config->signalCount == expectedSignals[index]);
  }

  return 0;
}
