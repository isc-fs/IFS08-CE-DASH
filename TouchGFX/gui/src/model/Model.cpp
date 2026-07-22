#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "app_config.h"
#include "main.h"
#include "memorymap.h"
#include "ui_buttons.h"

static uint16_t appsPercent(uint16_t raw, uint16_t min, uint16_t max)
{
    if (max <= min || raw <= min) return 0U;
    if (raw >= max) return 100U;
    return static_cast<uint16_t>((static_cast<uint32_t>(raw - min) * 100U) / (max - min));
}

Model::Model() : modelListener(0),
                 lastTelemetrySequence(0U),
                 hasTelemetry(0U),
                 infoPage(0U),
                 telemetry(),
                 paramAlertSettings{
                     {0U, 0U, VEHICLE_CONFIG_ALERT_0_MAX, 5U},
                     {0U, 0U, VEHICLE_CONFIG_ALERT_1_MAX, 5U},
                     {0U, 0U, VEHICLE_CONFIG_ALERT_2_MAX, 5U},
                     {0U, 0U, VEHICLE_CONFIG_ALERT_3_MAX, 5U},
                     {0U, 0U, VEHICLE_CONFIG_ALERT_4_MAX, 5U}},
                 sdLogSessionId{"NOLOG"}
{

}

uint16_t Model::getParamAlertValue(uint8_t index) const
{
    if (index >= PARAM_ALERT_COUNT)
    {
        return 0U;
    }

    return AppConfig_GetVehicleAlertThreshold(index);
}

uint16_t Model::getParamAlertMin(uint8_t index) const
{
    if (index >= PARAM_ALERT_COUNT)
    {
        return 0U;
    }

    return paramAlertSettings[index].minValue;
}

uint16_t Model::getParamAlertMax(uint8_t index) const
{
    if (index >= PARAM_ALERT_COUNT)
    {
        return 0U;
    }

    return paramAlertSettings[index].maxValue;
}

uint16_t Model::getParamAlertStep(uint8_t index) const
{
    if (index >= PARAM_ALERT_COUNT)
    {
        return 1U;
    }

    return paramAlertSettings[index].step;
}

void Model::setParamAlertValue(uint8_t index, uint16_t value)
{
    if (index >= PARAM_ALERT_COUNT)
    {
        return;
    }

    if (value < paramAlertSettings[index].minValue)
    {
        value = paramAlertSettings[index].minValue;
    }

    if (value > paramAlertSettings[index].maxValue)
    {
        value = paramAlertSettings[index].maxValue;
    }

    (void)AppConfig_SetVehicleAlertThreshold(index, value);
}

bool Model::isParamSdLoggingEnabled() const
{
    return (AppConfig_IsLocalSdLoggingEnabled() != 0U);
}

bool Model::isSdLogRecordingActive() const
{
    return (AppConfig_IsSdLogRecordingActive() != 0U);
}

const char* Model::getParamSdLogSessionId() const
{
    AppConfig_CopySdLogSessionId(sdLogSessionId, sizeof(sdLogSessionId));
    return sdLogSessionId;
}

void Model::setParamSdLoggingEnabled(bool enabled)
{
    const uint8_t requestedValue = enabled ? 1U : 0U;

    (void)AppConfig_SetLocalSdLoggingEnabled(requestedValue);
}

void Model::requestNewSdLogSession()
{
    AppConfig_RequestNewSdLogSession();
}

bool Model::hasDisplayTelemetry() const
{
    return (hasTelemetry != 0U);
}

const UiTelemetry& Model::getDisplayTelemetry() const
{
    return telemetry;
}

uint8_t Model::getInfoPage() const
{
    return infoPage;
}

void Model::setInfoPage(uint8_t page)
{
    infoPage = page;
}

UiTelemetry Model::buildUiTelemetry(const DisplayTelemetry& snapshot)
{
    UiTelemetry uiTelemetry = {};

    uiTelemetry.tickMs = snapshot.tick_ms;
    uiTelemetry.sequence = snapshot.sequence;

    uiTelemetry.ecuFsmState = snapshot.ecu_fsm_state;
    uiTelemetry.ecuBotonArranque = snapshot.ecu_boton_arranque;
    uiTelemetry.ecuS1Aceleracion = appsPercent(snapshot.ecu_s1_aceleracion, 2490U, 3350U);
    uiTelemetry.ecuS2Aceleracion = appsPercent(snapshot.ecu_s2_aceleracion, 2345U, 3025U);
    uiTelemetry.ecuAceleracion = static_cast<uint16_t>(
        (static_cast<uint32_t>(uiTelemetry.ecuS1Aceleracion) + uiTelemetry.ecuS2Aceleracion) / 2U);
    uiTelemetry.ecuSFreno = snapshot.ecu_s_freno;
    uiTelemetry.ecuTorqueTotal = snapshot.ecu_torque_total;
    uiTelemetry.ecuFlagEv23 = snapshot.ecu_flag_ev_2_3;
    uiTelemetry.ecuFlagT1189 = snapshot.ecu_flag_t11_8_9;

    uiTelemetry.amsOkPrecarga = snapshot.ams_ok_precarga;
    uiTelemetry.amsState = snapshot.ams_state;
    uiTelemetry.amsVCeldaMin = snapshot.ams_v_celda_min;
    uiTelemetry.amsSoc = snapshot.ams_soc;
    for (uint8_t index = 0U; index < 5U; index++)
    {
        uiTelemetry.amsVminModulo[index] = snapshot.ams_vmin_modulo[index];
        uiTelemetry.amsVmaxModulo[index] = snapshot.ams_vmax_modulo[index];
        uiTelemetry.amsTempMaxModulo[index] = snapshot.ams_temp_max_modulo[index];
    }
    uiTelemetry.amsCorrienteAccu = snapshot.ams_corriente_accu;
    uiTelemetry.amsCorrienteDcdc = snapshot.ams_corriente_dcdc;
    uiTelemetry.amsTempDcdc = snapshot.ams_temp_dcdc;

    uiTelemetry.inverterInvState = snapshot.inverter_inv_state;
    uiTelemetry.inverterInvVdcReady = snapshot.inverter_inv_vdc_ready;
    uiTelemetry.inverterInvError = snapshot.inverter_inv_error;
    uiTelemetry.inverterInvDcBusVoltage = snapshot.inverter_inv_dc_bus_voltage;
    uiTelemetry.inverterInvMotorTemp = snapshot.inverter_inv_motor_temp - 50;
    uiTelemetry.inverterInvIgbtTemp = snapshot.inverter_inv_igbt_temp - 50;
    uiTelemetry.inverterInvAirTemp = snapshot.inverter_inv_air_temp - 50;
    uiTelemetry.inverterInvRpm = snapshot.inverter_inv_rpm;
    uiTelemetry.inverterInvSpeedActual = snapshot.inverter_inv_speed_actual;
    uiTelemetry.inverterInvCurrentActual = snapshot.inverter_inv_current_actual;
    uiTelemetry.inverterInvCurrentDRaw = snapshot.inverter_inv_current_d_raw;
    uiTelemetry.inverterInvCurrentQRaw = snapshot.inverter_inv_current_q_raw;
    uiTelemetry.inverterInvVoltModulusPermil = snapshot.inverter_inv_volt_modulus_permil;
    uiTelemetry.inverterInvMotor2Temp = snapshot.inverter_inv_motor2_temp_raw - 50;
    uiTelemetry.inverterInvDemCode = snapshot.inverter_inv_dem_code;
    uiTelemetry.inverterInvDemPresent = snapshot.inverter_inv_dem_present;
    uiTelemetry.inverterInvPwrstgBitState = snapshot.inverter_inv_pwrstg_bit_state;
    uiTelemetry.inverterInvFocBitState = snapshot.inverter_inv_foc_bit_state;
    uiTelemetry.inverterInvUptimeMs = snapshot.inverter_inv_uptime_ms;
    uiTelemetry.inverterInvCore0LoadPct = snapshot.inverter_inv_core0_load_pct;
    uiTelemetry.inverterInvCore1LoadPct = snapshot.inverter_inv_core1_load_pct;
    uiTelemetry.inverterInvKl30Mv = snapshot.inverter_inv_kl30_mV;
    uiTelemetry.inverterInvCmdSrc = snapshot.inverter_inv_cmd_src;
    uiTelemetry.inverterInvCtrlType = snapshot.inverter_inv_ctrl_type;
    uiTelemetry.inverterInvCtrlMode = snapshot.inverter_inv_ctrl_mode;
    uiTelemetry.inverterInvPosFbSrc = snapshot.inverter_inv_pos_fb_src;
    uiTelemetry.inverterInvAcBusPowerW = snapshot.inverter_inv_ac_bus_power_W;
    uiTelemetry.inverterInvTorqueMaxFeasNdm = snapshot.inverter_inv_torque_max_feas_Ndm;
    uiTelemetry.inverterInvTorqueEstNm = snapshot.inverter_inv_torque_est_Nm;
    uiTelemetry.inverterInvSetpointDRaw = snapshot.inverter_inv_setpoint_d_raw;
    uiTelemetry.inverterInvSetpointQRaw = snapshot.inverter_inv_setpoint_q_raw;

    uiTelemetry.gpsSpeed = snapshot.gps_speed;
    uiTelemetry.gpsCourseDeg = snapshot.gps_course_deg;
    uiTelemetry.gpsAltitude = snapshot.gps_altitude;
    uiTelemetry.gpsFixType = snapshot.gps_fix_type;
    uiTelemetry.gpsSatCount = snapshot.gps_sat_count;
    uiTelemetry.gpsHdop = snapshot.gps_hdop;
    uiTelemetry.gpsLatitude = snapshot.gps_latitude;
    uiTelemetry.gpsLongitude = snapshot.gps_longitude;

    return uiTelemetry;
}

void Model::tick()
{
    DisplayTelemetry snapshot = {};
    uint32_t sequence = 0U;
    uint32_t buttonMask = 0U;
    DisplayDiag_OnUiTick();

    if (modelListener != 0)
    {
        modelListener->onTick();
    }

    buttonMask = UIButtons_FetchPressedMask();
    if (modelListener != 0)
    {
        for (uint32_t buttonIndex = 0U; buttonIndex < UI_BUTTON_ID_COUNT; buttonIndex++)
        {
            if ((buttonMask & (1UL << buttonIndex)) != 0U)
            {
                modelListener->onUiButtonPressed(static_cast<UiButtonId>(buttonIndex));
            }
        }
    }

    if (DisplayTelemetry_GetSnapshot(&snapshot, &sequence) == 0U)
    {
        return;
    }

    if ((hasTelemetry != 0U) && (sequence == lastTelemetrySequence))
    {
        return;
    }

    telemetry = buildUiTelemetry(snapshot);
    lastTelemetrySequence = sequence;
    hasTelemetry = 1U;

    if (modelListener != 0)
    {
        DisplayDiag_OnUiTelemetryPush();
        modelListener->onDisplayTelemetryChanged(telemetry);
    }
}
