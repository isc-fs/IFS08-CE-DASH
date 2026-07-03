#ifndef INFOPRESENTER_HPP
#define INFOPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class InfoView;

class InfoPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    InfoPresenter(InfoView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();
    virtual void onTick();
    virtual void onDisplayTelemetryChanged(const UiTelemetry& telemetry);
    virtual void onUiButtonPressed(UiButtonId button);

    virtual ~InfoPresenter() {}

private:
    InfoPresenter();
    void syncAlertThresholds();
    void syncSdStatus();

    InfoView& view;
    uint16_t lastAlertThresholds[Model::PARAM_ALERT_COUNT];
    bool lastSdRecording;
    char lastSdSessionId[13];
};

#endif // INFOPRESENTER_HPP
