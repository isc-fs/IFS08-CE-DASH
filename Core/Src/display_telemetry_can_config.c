#include "display_telemetry_can_config.h"
#include <stddef.h>

static const DisplayTelemetrySignalConfig g_dash510Signals[] =
{
  {"inverter_inv_state", offsetof(DisplayTelemetry, inverter_inv_state), 1U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"ecu_torque_total", offsetof(DisplayTelemetry, ecu_torque_total), 2U, 1U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"ecu_flag_ev_2_3", offsetof(DisplayTelemetry, ecu_flag_ev_2_3), 1U, 2U, 8U, 8U, 0x01U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_BIT},
  {"ecu_flag_t11_8_9", offsetof(DisplayTelemetry, ecu_flag_t11_8_9), 1U, 2U, 8U, 8U, 0x02U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_BIT},
  {"ams_ok_precarga", offsetof(DisplayTelemetry, ams_ok_precarga), 1U, 3U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"ecu_boton_arranque", offsetof(DisplayTelemetry, ecu_boton_arranque), 1U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"sequence", offsetof(DisplayTelemetry, sequence), 2U, 6U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash511Signals[] =
{
  {"ecu_s1_aceleracion", offsetof(DisplayTelemetry, ecu_s1_aceleracion), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ecu_s2_aceleracion", offsetof(DisplayTelemetry, ecu_s2_aceleracion), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ecu_s_freno", offsetof(DisplayTelemetry, ecu_s_freno), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash512Signals[] =
{
  {"inverter_inv_dc_bus_voltage", offsetof(DisplayTelemetry, inverter_inv_dc_bus_voltage), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_v_celda_min", offsetof(DisplayTelemetry, ams_v_celda_min), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"inverter_inv_error", offsetof(DisplayTelemetry, inverter_inv_error), 1U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_vdc_ready", offsetof(DisplayTelemetry, inverter_inv_vdc_ready), 1U, 5U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8}
};

static const DisplayTelemetrySignalConfig g_dash513Signals[] =
{
  {"inverter_inv_motor_temp", offsetof(DisplayTelemetry, inverter_inv_motor_temp), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_igbt_temp", offsetof(DisplayTelemetry, inverter_inv_igbt_temp), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_air_temp", offsetof(DisplayTelemetry, inverter_inv_air_temp), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetrySignalConfig g_dash514Signals[] =
{
  {"inverter_inv_rpm", offsetof(DisplayTelemetry, inverter_inv_rpm), 4U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE}
};

static const DisplayTelemetrySignalConfig g_dash515Signals[] =
{
  {"inverter_inv_speed_actual", offsetof(DisplayTelemetry, inverter_inv_speed_actual), 4U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE}
};

static const DisplayTelemetrySignalConfig g_dash516Signals[] =
{
  {"inverter_inv_current_actual", offsetof(DisplayTelemetry, inverter_inv_current_actual), 4U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE}
};

static const DisplayTelemetrySignalConfig g_dash517Signals[] =
{
  {"ecu_fsm_state", offsetof(DisplayTelemetry, ecu_fsm_state), 1U, 0U, 2U, 2U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"ams_state", offsetof(DisplayTelemetry, ams_state), 1U, 1U, 2U, 2U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8}
};

static const DisplayTelemetrySignalConfig g_dash518Signals[] =
{
  {"ams_soc", offsetof(DisplayTelemetry, ams_soc), 1U, 0U, 7U, 7U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"ams_corriente_accu", offsetof(DisplayTelemetry, ams_corriente_accu), 2U, 1U, 7U, 7U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_corriente_dcdc", offsetof(DisplayTelemetry, ams_corriente_dcdc), 2U, 3U, 7U, 7U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_temp_dcdc", offsetof(DisplayTelemetry, ams_temp_dcdc), 2U, 5U, 7U, 7U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetrySignalConfig g_dash519Signals[] =
{
  {"gps_speed", offsetof(DisplayTelemetry, gps_speed), 2U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"gps_course_deg", offsetof(DisplayTelemetry, gps_course_deg), 2U, 2U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"gps_altitude", offsetof(DisplayTelemetry, gps_altitude), 4U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE}
};

static const DisplayTelemetrySignalConfig g_dash51ASignals[] =
{
  {"gps_fix_type", offsetof(DisplayTelemetry, gps_fix_type), 1U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"gps_sat_count", offsetof(DisplayTelemetry, gps_sat_count), 1U, 1U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"gps_hdop", offsetof(DisplayTelemetry, gps_hdop), 2U, 2U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"gps_latitude", offsetof(DisplayTelemetry, gps_latitude), 4U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE}
};

static const DisplayTelemetrySignalConfig g_dash51BSignals[] =
{
  {"gps_longitude", offsetof(DisplayTelemetry, gps_longitude), 4U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE},
  {"tick_ms", offsetof(DisplayTelemetry, tick_ms), 4U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U32_LE}
};

static const DisplayTelemetrySignalConfig g_dash51CSignals[] =
{
  {"ams_vmin_modulo0", offsetof(DisplayTelemetry, ams_vmin_modulo[0]), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmin_modulo1", offsetof(DisplayTelemetry, ams_vmin_modulo[1]), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmin_modulo2", offsetof(DisplayTelemetry, ams_vmin_modulo[2]), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash51DSignals[] =
{
  {"ams_vmin_modulo3", offsetof(DisplayTelemetry, ams_vmin_modulo[3]), 2U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmin_modulo4", offsetof(DisplayTelemetry, ams_vmin_modulo[4]), 2U, 2U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash51ESignals[] =
{
  {"ams_vmax_modulo0", offsetof(DisplayTelemetry, ams_vmax_modulo[0]), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmax_modulo1", offsetof(DisplayTelemetry, ams_vmax_modulo[1]), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmax_modulo2", offsetof(DisplayTelemetry, ams_vmax_modulo[2]), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash51FSignals[] =
{
  {"ams_vmax_modulo3", offsetof(DisplayTelemetry, ams_vmax_modulo[3]), 2U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"ams_vmax_modulo4", offsetof(DisplayTelemetry, ams_vmax_modulo[4]), 2U, 2U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash520Signals[] =
{
  {"ams_temp_max_modulo0", offsetof(DisplayTelemetry, ams_temp_max_modulo[0]), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_temp_max_modulo1", offsetof(DisplayTelemetry, ams_temp_max_modulo[1]), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_temp_max_modulo2", offsetof(DisplayTelemetry, ams_temp_max_modulo[2]), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetrySignalConfig g_dash521Signals[] =
{
  {"ams_temp_max_modulo3", offsetof(DisplayTelemetry, ams_temp_max_modulo[3]), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_temp_max_modulo4", offsetof(DisplayTelemetry, ams_temp_max_modulo[4]), 2U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"ams_temp_dcdc", offsetof(DisplayTelemetry, ams_temp_dcdc), 2U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetrySignalConfig g_dash522Signals[] =
{
  {"inverter_inv_current_d_raw", offsetof(DisplayTelemetry, inverter_inv_current_d_raw), 2U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_current_q_raw", offsetof(DisplayTelemetry, inverter_inv_current_q_raw), 2U, 2U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_volt_modulus_permil", offsetof(DisplayTelemetry, inverter_inv_volt_modulus_permil), 2U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"inverter_inv_motor2_temp_raw", offsetof(DisplayTelemetry, inverter_inv_motor2_temp_raw), 2U, 6U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE}
};

static const DisplayTelemetrySignalConfig g_dash523Signals[] =
{
  {"inverter_inv_dem_code", offsetof(DisplayTelemetry, inverter_inv_dem_code), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"inverter_inv_dem_present", offsetof(DisplayTelemetry, inverter_inv_dem_present), 1U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_pwrstg_bit_state", offsetof(DisplayTelemetry, inverter_inv_pwrstg_bit_state), 2U, 3U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"inverter_inv_foc_bit_state", offsetof(DisplayTelemetry, inverter_inv_foc_bit_state), 1U, 5U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8}
};

static const DisplayTelemetrySignalConfig g_dash524Signals[] =
{
  {"inverter_inv_uptime_ms", offsetof(DisplayTelemetry, inverter_inv_uptime_ms), 4U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U32_LE},
  {"inverter_inv_core0_load_pct", offsetof(DisplayTelemetry, inverter_inv_core0_load_pct), 1U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_core1_load_pct", offsetof(DisplayTelemetry, inverter_inv_core1_load_pct), 1U, 5U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8}
};

static const DisplayTelemetrySignalConfig g_dash525Signals[] =
{
  {"inverter_inv_kl30_mV", offsetof(DisplayTelemetry, inverter_inv_kl30_mV), 2U, 0U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U16_LE},
  {"inverter_inv_cmd_src", offsetof(DisplayTelemetry, inverter_inv_cmd_src), 1U, 2U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_ctrl_type", offsetof(DisplayTelemetry, inverter_inv_ctrl_type), 1U, 3U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_ctrl_mode", offsetof(DisplayTelemetry, inverter_inv_ctrl_mode), 1U, 4U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8},
  {"inverter_inv_pos_fb_src", offsetof(DisplayTelemetry, inverter_inv_pos_fb_src), 1U, 5U, 6U, 6U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_U8}
};

static const DisplayTelemetrySignalConfig g_dash526Signals[] =
{
  {"inverter_inv_ac_bus_power_W", offsetof(DisplayTelemetry, inverter_inv_ac_bus_power_W), 4U, 0U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S32_LE},
  {"inverter_inv_torque_max_feas_Ndm", offsetof(DisplayTelemetry, inverter_inv_torque_max_feas_Ndm), 2U, 4U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_torque_est_Nm", offsetof(DisplayTelemetry, inverter_inv_torque_est_Nm), 2U, 6U, 8U, 8U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetrySignalConfig g_dash527Signals[] =
{
  {"inverter_inv_setpoint_d_raw", offsetof(DisplayTelemetry, inverter_inv_setpoint_d_raw), 2U, 0U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE},
  {"inverter_inv_setpoint_q_raw", offsetof(DisplayTelemetry, inverter_inv_setpoint_q_raw), 2U, 2U, 4U, 4U, 0U, DISPLAY_TELEMETRY_SIGNAL_ENCODING_S16_LE}
};

static const DisplayTelemetryCanMessageConfig g_displayTelemetryCanConfigs[] =
{
  {"dash_510", 0x510U, (1UL << 0), 8U, 8U, g_dash510Signals, (uint32_t)(sizeof(g_dash510Signals) / sizeof(g_dash510Signals[0]))},
  {"dash_511", 0x511U, (1UL << 1), 6U, 6U, g_dash511Signals, (uint32_t)(sizeof(g_dash511Signals) / sizeof(g_dash511Signals[0]))},
  {"dash_512", 0x512U, (1UL << 2), 6U, 6U, g_dash512Signals, (uint32_t)(sizeof(g_dash512Signals) / sizeof(g_dash512Signals[0]))},
  {"dash_513", 0x513U, (1UL << 3), 6U, 6U, g_dash513Signals, (uint32_t)(sizeof(g_dash513Signals) / sizeof(g_dash513Signals[0]))},
  {"dash_514", 0x514U, (1UL << 4), 4U, 4U, g_dash514Signals, (uint32_t)(sizeof(g_dash514Signals) / sizeof(g_dash514Signals[0]))},
  {"dash_515", 0x515U, (1UL << 5), 4U, 4U, g_dash515Signals, (uint32_t)(sizeof(g_dash515Signals) / sizeof(g_dash515Signals[0]))},
  {"dash_516", 0x516U, (1UL << 6), 4U, 4U, g_dash516Signals, (uint32_t)(sizeof(g_dash516Signals) / sizeof(g_dash516Signals[0]))},
  {"dash_517", 0x517U, (1UL << 7), 2U, 2U, g_dash517Signals, (uint32_t)(sizeof(g_dash517Signals) / sizeof(g_dash517Signals[0]))},
  {"dash_518", 0x518U, (1UL << 8), 7U, 7U, g_dash518Signals, (uint32_t)(sizeof(g_dash518Signals) / sizeof(g_dash518Signals[0]))},
  {"dash_519", 0x519U, (1UL << 9), 8U, 8U, g_dash519Signals, (uint32_t)(sizeof(g_dash519Signals) / sizeof(g_dash519Signals[0]))},
  {"dash_51A", 0x51AU, (1UL << 10), 8U, 8U, g_dash51ASignals, (uint32_t)(sizeof(g_dash51ASignals) / sizeof(g_dash51ASignals[0]))},
  {"dash_51B", 0x51BU, (1UL << 11), 8U, 8U, g_dash51BSignals, (uint32_t)(sizeof(g_dash51BSignals) / sizeof(g_dash51BSignals[0]))},
  {"dash_51C", 0x51CU, (1UL << 12), 6U, 6U, g_dash51CSignals, (uint32_t)(sizeof(g_dash51CSignals) / sizeof(g_dash51CSignals[0]))},
  {"dash_51D", 0x51DU, (1UL << 13), 4U, 4U, g_dash51DSignals, (uint32_t)(sizeof(g_dash51DSignals) / sizeof(g_dash51DSignals[0]))},
  {"dash_51E", 0x51EU, (1UL << 14), 6U, 6U, g_dash51ESignals, (uint32_t)(sizeof(g_dash51ESignals) / sizeof(g_dash51ESignals[0]))},
  {"dash_51F", 0x51FU, (1UL << 15), 4U, 4U, g_dash51FSignals, (uint32_t)(sizeof(g_dash51FSignals) / sizeof(g_dash51FSignals[0]))},
  {"dash_520", 0x520U, (1UL << 16), 6U, 6U, g_dash520Signals, (uint32_t)(sizeof(g_dash520Signals) / sizeof(g_dash520Signals[0]))},
  {"dash_521", 0x521U, (1UL << 17), 6U, 6U, g_dash521Signals, (uint32_t)(sizeof(g_dash521Signals) / sizeof(g_dash521Signals[0]))},
  {"dash_522", 0x522U, (1UL << 18), 8U, 8U, g_dash522Signals, (uint32_t)(sizeof(g_dash522Signals) / sizeof(g_dash522Signals[0]))},
  {"dash_523", 0x523U, (1UL << 19), 6U, 6U, g_dash523Signals, (uint32_t)(sizeof(g_dash523Signals) / sizeof(g_dash523Signals[0]))},
  {"dash_524", 0x524U, (1UL << 20), 6U, 6U, g_dash524Signals, (uint32_t)(sizeof(g_dash524Signals) / sizeof(g_dash524Signals[0]))},
  {"dash_525", 0x525U, (1UL << 21), 6U, 6U, g_dash525Signals, (uint32_t)(sizeof(g_dash525Signals) / sizeof(g_dash525Signals[0]))},
  {"dash_526", 0x526U, (1UL << 22), 8U, 8U, g_dash526Signals, (uint32_t)(sizeof(g_dash526Signals) / sizeof(g_dash526Signals[0]))},
  {"dash_527", 0x527U, (1UL << 23), 4U, 4U, g_dash527Signals, (uint32_t)(sizeof(g_dash527Signals) / sizeof(g_dash527Signals[0]))}
};

const DisplayTelemetryCanMessageConfig *DisplayTelemetryCanConfig_GetAll(uint32_t *out_count)
{
  if (out_count != NULL)
  {
    *out_count = (uint32_t)(sizeof(g_displayTelemetryCanConfigs) / sizeof(g_displayTelemetryCanConfigs[0]));
  }

  return g_displayTelemetryCanConfigs;
}

const DisplayTelemetryCanMessageConfig *DisplayTelemetryCanConfig_FindById(uint32_t canId)
{
  uint32_t count = 0U;
  const DisplayTelemetryCanMessageConfig *configs = DisplayTelemetryCanConfig_GetAll(&count);

  for (uint32_t index = 0U; index < count; index++)
  {
    if (configs[index].canId == canId)
    {
      return &configs[index];
    }
  }

  return NULL;
}

uint32_t DisplayTelemetryCanConfig_GetMinId(void)
{
  return 0x510U;
}

uint32_t DisplayTelemetryCanConfig_GetMaxId(void)
{
  return 0x527U;
}

uint32_t DisplayTelemetryCanConfig_GetSnapshotMaskAll(void)
{
  uint32_t count = 0U;
  const DisplayTelemetryCanMessageConfig *configs = DisplayTelemetryCanConfig_GetAll(&count);
  uint32_t mask = 0U;

  for (uint32_t index = 0U; index < count; index++)
  {
    mask |= configs[index].snapshotMask;
  }

  return mask;
}
