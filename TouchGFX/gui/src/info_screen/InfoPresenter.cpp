#include <gui/info_screen/InfoView.hpp>
#include <gui/info_screen/InfoPresenter.hpp>
#include <cstring>

InfoPresenter::InfoPresenter(InfoView& v)
    : view(v),
      lastAlertThresholds{0U},
      lastSdRecording(false),
      lastSdSessionId{0}
{

}

void InfoPresenter::activate()
{
    view.setPage(model->getInfoPage());
    syncAlertThresholds();
    if (model->hasDisplayTelemetry())
    {
        view.setTelemetry(model->getDisplayTelemetry());
    }
    syncSdStatus();
}

void InfoPresenter::deactivate()
{

}

void InfoPresenter::onTick()
{
    syncAlertThresholds();
    syncSdStatus();
}

void InfoPresenter::onDisplayTelemetryChanged(const UiTelemetry& telemetry)
{
    view.setTelemetry(telemetry);
    syncSdStatus();
}

void InfoPresenter::onUiButtonPressed(UiButtonId button)
{
    if (button == UI_BUTTON_ID_MENU)
    {
        view.ChangeToScreenParam();
    }
    else if (button == UI_BUTTON_ID_UP)
    {
        view.nextPage();
        model->setInfoPage(view.getPage());
    }
    else if (button == UI_BUTTON_ID_DOWN)
    {
        view.previousPage();
        model->setInfoPage(view.getPage());
    }
    else if (button == UI_BUTTON_ID_SELECT)
    {
        view.dismissAlertsPopup();
    }
}

void InfoPresenter::syncAlertThresholds()
{
    uint16_t thresholds[Model::PARAM_ALERT_COUNT] = {0U};
    bool changed = false;

    for (uint8_t i = 0U; i < Model::PARAM_ALERT_COUNT; ++i)
    {
        thresholds[i] = model->getParamAlertValue(i);
        changed = changed || (thresholds[i] != lastAlertThresholds[i]);
    }

    if (changed)
    {
        for (uint8_t i = 0U; i < Model::PARAM_ALERT_COUNT; ++i)
        {
            lastAlertThresholds[i] = thresholds[i];
        }
        view.setAlertThresholds(thresholds);
    }
}

void InfoPresenter::syncSdStatus()
{
    const bool recording = model->isParamSdLoggingEnabled() && model->isSdLogRecordingActive();
    const char* sessionId = model->getParamSdLogSessionId();

    if ((recording == lastSdRecording) &&
        (std::strncmp(lastSdSessionId, sessionId, sizeof(lastSdSessionId)) == 0))
    {
        return;
    }

    lastSdRecording = recording;
    std::strncpy(lastSdSessionId, sessionId, sizeof(lastSdSessionId) - 1U);
    lastSdSessionId[sizeof(lastSdSessionId) - 1U] = '\0';
    view.setSdLoggingStatus(recording, sessionId);
}
