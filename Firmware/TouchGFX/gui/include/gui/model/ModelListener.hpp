#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include "ui_buttons.h"
class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    virtual void onTick() {}
    virtual void onPedalValueChanged(uint16_t value) {}
    virtual void onDisplayTelemetryChanged(const UiTelemetry& telemetry) {}
    virtual void onUiButtonPressed(UiButtonId button) {}

    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
