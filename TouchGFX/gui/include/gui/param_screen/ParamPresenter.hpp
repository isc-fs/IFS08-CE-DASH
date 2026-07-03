#ifndef PARAMPRESENTER_HPP
#define PARAMPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class ParamView;

class ParamPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    ParamPresenter(ParamView& v);

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
    virtual void onUiButtonPressed(UiButtonId button);

    virtual ~ParamPresenter() {}

private:
    enum
    {
        PAGE_AVISOS = 0,
        PAGE_CONFIGURACION = 1
    };

    enum InteractionState
    {
        STATE_MENU_SELECTION = 0,
        STATE_CONTENT_SELECTION,
        STATE_EDIT_ALERT
    };

    ParamPresenter();
    void refreshView();
    void syncAlertValues();
    void adjustEditingValue(int direction);

    ParamView& view;
    uint16_t alertValues[Model::PARAM_ALERT_COUNT];
    uint16_t configValues[Model::PARAM_CONFIG_COUNT];
    uint8_t currentPage;
    uint8_t avisosRow;
    uint8_t configuracionRow;
    uint8_t interactionState;
    uint16_t editingValue;
    uint16_t editingOriginalValue;
    bool lastSdEnabled;
    bool lastSdRecording;
    char lastSdSessionId[13];
};

#endif // PARAMPRESENTER_HPP
