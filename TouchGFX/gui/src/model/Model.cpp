#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "app_config.h"
#include "main.h"
#include "memorymap.h"
#include "ui_buttons.h"

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
    uiTelemetry.ecuAceleracion = static_cast<uint16_t>(
        (static_cast<uint32_t>(snapshot.ecu_s1_aceleracion) + static_cast<uint32_t>(snapshot.ecu_s2_aceleracion)) / 2U);
    uiTelemetry.ecuS1Aceleracion = snapshot.ecu_s1_aceleracion;
    uiTelemetry.ecuS2Aceleracion = snapshot.ecu_s2_aceleracion;
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
    uiTelemetry.inverterInvMotorTemp = snapshot.inverter_inv_motor_temp;
    uiTelemetry.inverterInvIgbtTemp = snapshot.inverter_inv_igbt_temp;
    uiTelemetry.inverterInvAirTemp = snapshot.inverter_inv_air_temp;
    uiTelemetry.inverterInvRpm = snapshot.inverter_inv_rpm;
    uiTelemetry.inverterInvSpeedActual = snapshot.inverter_inv_speed_actual;
    uiTelemetry.inverterInvCurrentActual = snapshot.inverter_inv_current_actual;

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
