#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>
#include "display_telemetry.h"

class ModelListener;

struct UiTelemetry
{
    uint32_t tickMs;
    uint16_t sequence;

    uint8_t ecuFsmState;
    uint8_t ecuBotonArranque;
    uint16_t ecuAceleracion;
    uint16_t ecuS1Aceleracion;
    uint16_t ecuS2Aceleracion;
    uint16_t ecuSFreno;
    uint16_t ecuTorqueTotal;
    uint8_t ecuFlagEv23;
    uint8_t ecuFlagT1189;

    uint8_t amsOkPrecarga;
    uint8_t amsState;
    uint16_t amsVCeldaMin;
    uint8_t amsSoc;
    uint16_t amsVminModulo[5];
    uint16_t amsVmaxModulo[5];
    int16_t amsCorrienteAccu;
    int16_t amsCorrienteDcdc;
    int16_t amsTempDcdc;
    int16_t amsTempMaxModulo[5];

    uint8_t inverterInvState;
    uint8_t inverterInvVdcReady;
    uint8_t inverterInvError;
    uint16_t inverterInvDcBusVoltage;
    int16_t inverterInvMotorTemp;
    int16_t inverterInvIgbtTemp;
    int16_t inverterInvAirTemp;
    int32_t inverterInvRpm;
    int32_t inverterInvSpeedActual;
    int32_t inverterInvCurrentActual;

    uint16_t gpsSpeed;
    uint16_t gpsCourseDeg;
    int32_t gpsAltitude;
    uint8_t gpsFixType;
    uint8_t gpsSatCount;
    uint16_t gpsHdop;
    int32_t gpsLatitude;
    int32_t gpsLongitude;
};

class Model
{
public:
    enum
    {
        PARAM_ALERT_COUNT = 5,
        PARAM_CONFIG_COUNT = 2
    };

    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    uint16_t getParamAlertValue(uint8_t index) const;
    uint16_t getParamAlertMin(uint8_t index) const;
    uint16_t getParamAlertMax(uint8_t index) const;
    uint16_t getParamAlertStep(uint8_t index) const;
    void setParamAlertValue(uint8_t index, uint16_t value);

    bool isParamSdLoggingEnabled() const;
    bool isSdLogRecordingActive() const;
    const char* getParamSdLogSessionId() const;
    void setParamSdLoggingEnabled(bool enabled);
    void requestNewSdLogSession();
    bool hasDisplayTelemetry() const;
    const UiTelemetry& getDisplayTelemetry() const;
    uint8_t getInfoPage() const;
    void setInfoPage(uint8_t page);
private:
    struct ParamAlertSetting
    {
        uint16_t value;
        uint16_t minValue;
        uint16_t maxValue;
        uint16_t step;
    };

    static UiTelemetry buildUiTelemetry(const DisplayTelemetry& snapshot);

    ModelListener* modelListener;
    uint32_t lastTelemetrySequence;
    uint8_t hasTelemetry;
    uint8_t infoPage;
    UiTelemetry telemetry;
    ParamAlertSetting paramAlertSettings[PARAM_ALERT_COUNT];
    mutable char sdLogSessionId[13];
};

#endif // MODEL_HPP
