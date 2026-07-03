#ifndef __DISPLAY_TELEMETRY_H__
#define __DISPLAY_TELEMETRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef struct
{
  /* Snapshot metadata */
  uint32_t tick_ms;
  uint16_t sequence;

  /* ECU */
  uint8_t ecu_fsm_state;
  uint8_t ecu_boton_arranque;
  uint16_t ecu_s1_aceleracion;
  uint16_t ecu_s2_aceleracion;
  uint16_t ecu_s_freno;
  uint16_t ecu_torque_total;
  uint8_t ecu_flag_ev_2_3;
  uint8_t ecu_flag_t11_8_9;

  /* AMS */
  uint8_t ams_ok_precarga;
  uint8_t ams_state;
  uint16_t ams_v_celda_min;
  uint8_t ams_soc;
  uint16_t ams_vmin_modulo[5];
  uint16_t ams_vmax_modulo[5];
  int16_t ams_corriente_accu;
  int16_t ams_corriente_dcdc;
  int16_t ams_temp_dcdc;
  int16_t ams_temp_max_modulo[5];

  /* Inverter */
  uint8_t inverter_inv_state;
  uint8_t inverter_inv_vdc_ready;
  uint8_t inverter_inv_error;
  uint16_t inverter_inv_dc_bus_voltage;
  int16_t inverter_inv_motor_temp;
  int16_t inverter_inv_igbt_temp;
  int16_t inverter_inv_air_temp;
  int32_t inverter_inv_rpm;
  int32_t inverter_inv_speed_actual;
  int32_t inverter_inv_current_actual;

  /* GPS */
  uint16_t gps_speed;
  uint16_t gps_course_deg;
  int32_t gps_altitude;
  uint8_t gps_fix_type;
  uint8_t gps_sat_count;
  uint16_t gps_hdop;
  int32_t gps_latitude;
  int32_t gps_longitude;
} VehicleTelemetry;

typedef VehicleTelemetry DisplayTelemetry;

void DisplayTelemetry_Reset(void);
void DisplayTelemetry_TaskInit(void);
void DisplayTelemetry_TaskStep(void);
uint8_t DisplayTelemetry_GetSnapshot(DisplayTelemetry *snapshot, uint32_t *sequence);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_TELEMETRY_H__ */
