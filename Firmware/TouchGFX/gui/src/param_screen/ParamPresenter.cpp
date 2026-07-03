#include <gui/param_screen/ParamView.hpp>
#include <gui/param_screen/ParamPresenter.hpp>
#include <cstring>

namespace
{
uint8_t wrapSelection(int value, uint8_t count)
{
    if (count == 0U)
    {
        return 0U;
    }

    if (value < 0)
    {
        return static_cast<uint8_t>(count - 1U);
    }

    if (value >= static_cast<int>(count))
    {
        return 0U;
    }

    return static_cast<uint8_t>(value);
}
}

ParamPresenter::ParamPresenter(ParamView& v)
    : view(v),
      alertValues{0U},
      configValues{0U},
      currentPage(PAGE_AVISOS),
      avisosRow(0U),
      configuracionRow(0U),
      interactionState(STATE_MENU_SELECTION),
      editingValue(0U),
      editingOriginalValue(0U),
      lastSdEnabled(false),
      lastSdRecording(false),
      lastSdSessionId{0}
{

}

void ParamPresenter::activate()
{
    syncAlertValues();
    configValues[0] = model->isParamSdLoggingEnabled() ? 1U : 0U;
    interactionState = STATE_MENU_SELECTION;
    refreshView();
}

void ParamPresenter::deactivate()
{

}

void ParamPresenter::onTick()
{
    const bool sdEnabled = model->isParamSdLoggingEnabled();
    const bool sdRecording = model->isSdLogRecordingActive();
    const char* sessionId = model->getParamSdLogSessionId();
    bool alertChanged = false;

    for (uint8_t index = 0U; index < Model::PARAM_ALERT_COUNT; ++index)
    {
        alertChanged = alertChanged || (model->getParamAlertValue(index) != alertValues[index]);
    }

    if ((sdEnabled != lastSdEnabled) ||
        (sdRecording != lastSdRecording) ||
        (std::strncmp(lastSdSessionId, sessionId, sizeof(lastSdSessionId)) != 0) ||
        ((interactionState != STATE_EDIT_ALERT) && alertChanged))
    {
        refreshView();
    }
}

void ParamPresenter::onUiButtonPressed(UiButtonId button)
{
    switch (interactionState)
    {
    case STATE_MENU_SELECTION:
        if (button == UI_BUTTON_ID_UP)
        {
            currentPage = wrapSelection(static_cast<int>(currentPage) - 1, 2U);
            refreshView();
        }
        else if (button == UI_BUTTON_ID_DOWN)
        {
            currentPage = wrapSelection(static_cast<int>(currentPage) + 1, 2U);
            refreshView();
        }
        else if (button == UI_BUTTON_ID_SELECT)
        {
            interactionState = STATE_CONTENT_SELECTION;
            refreshView();
        }
        else if (button == UI_BUTTON_ID_MENU)
        {
            view.ChangeToScreenInfo();
        }
        break;

    case STATE_CONTENT_SELECTION:
        if (button == UI_BUTTON_ID_MENU)
        {
            interactionState = STATE_MENU_SELECTION;
            refreshView();
        }
        else if (button == UI_BUTTON_ID_UP)
        {
            if (currentPage == PAGE_AVISOS)
            {
                avisosRow = wrapSelection(static_cast<int>(avisosRow) - 1, Model::PARAM_ALERT_COUNT);
            }
            else
            {
                configuracionRow = wrapSelection(static_cast<int>(configuracionRow) - 1, Model::PARAM_CONFIG_COUNT);
            }
            refreshView();
        }
        else if (button == UI_BUTTON_ID_DOWN)
        {
            if (currentPage == PAGE_AVISOS)
            {
                avisosRow = wrapSelection(static_cast<int>(avisosRow) + 1, Model::PARAM_ALERT_COUNT);
            }
            else
            {
                configuracionRow = wrapSelection(static_cast<int>(configuracionRow) + 1, Model::PARAM_CONFIG_COUNT);
            }
            refreshView();
        }
        else if (button == UI_BUTTON_ID_SELECT)
        {
            if (currentPage == PAGE_AVISOS)
            {
                editingOriginalValue = model->getParamAlertValue(avisosRow);
                editingValue = editingOriginalValue;
                interactionState = STATE_EDIT_ALERT;
            }
            else
            {
                if (configuracionRow == 0U)
                {
                    if (model->isSdLogRecordingActive())
                    {
                        model->setParamSdLoggingEnabled(false);
                    }
                    else
                    {
                        model->setParamSdLoggingEnabled(true);
                        if (std::strncmp(model->getParamSdLogSessionId(), "NOLOG", 5U) == 0)
                        {
                            model->requestNewSdLogSession();
                        }
                    }
                    refreshView();
                    view.flashConfigIcon(configuracionRow);
                    return;
                }
                else
                {
                    model->setParamSdLoggingEnabled(true);
                    model->requestNewSdLogSession();
                    refreshView();
                    view.flashConfigIcon(configuracionRow);
                    return;
                }
            }
            refreshView();
        }
        break;

    case STATE_EDIT_ALERT:
        if (button == UI_BUTTON_ID_MENU)
        {
            editingValue = editingOriginalValue;
            interactionState = STATE_CONTENT_SELECTION;
            refreshView();
        }
        else if (button == UI_BUTTON_ID_SELECT)
        {
            model->setParamAlertValue(avisosRow, editingValue);
            syncAlertValues();
            interactionState = STATE_CONTENT_SELECTION;
            refreshView();
        }
        else if (button == UI_BUTTON_ID_UP)
        {
            adjustEditingValue(+1);
            refreshView();
        }
        else if (button == UI_BUTTON_ID_DOWN)
        {
            adjustEditingValue(-1);
            refreshView();
        }
        break;

    default:
        break;
    }
}

void ParamPresenter::refreshView()
{
    uint16_t displayedAlertValues[Model::PARAM_ALERT_COUNT] = {0U};

    syncAlertValues();
    for (uint8_t index = 0U; index < Model::PARAM_ALERT_COUNT; index++)
    {
        displayedAlertValues[index] = alertValues[index];
    }

    if (interactionState == STATE_EDIT_ALERT)
    {
        displayedAlertValues[avisosRow] = editingValue;
    }

    configValues[0] = model->isSdLogRecordingActive() ? 1U : 0U;
    configValues[1] = 0U;

    view.showPage(currentPage, interactionState == STATE_MENU_SELECTION);
    view.updateAlertValues(displayedAlertValues);
    view.updateConfigValues(configValues, model->getParamSdLogSessionId());
    lastSdEnabled = model->isParamSdLoggingEnabled();
    lastSdRecording = model->isSdLogRecordingActive();
    std::strncpy(lastSdSessionId, model->getParamSdLogSessionId(), sizeof(lastSdSessionId) - 1U);
    lastSdSessionId[sizeof(lastSdSessionId) - 1U] = '\0';
    if (currentPage == PAGE_AVISOS)
    {
        view.setConfigSelection(static_cast<int>(configuracionRow),
                                false,
                                false);
        view.setAlertSelection(static_cast<int>(avisosRow),
                               (interactionState != STATE_MENU_SELECTION),
                               interactionState == STATE_EDIT_ALERT);
    }
    else
    {
        view.setAlertSelection(static_cast<int>(avisosRow),
                               false,
                               false);
        view.setConfigSelection(static_cast<int>(configuracionRow),
                                (interactionState != STATE_MENU_SELECTION),
                                false);
    }
}

void ParamPresenter::syncAlertValues()
{
    for (uint8_t index = 0U; index < Model::PARAM_ALERT_COUNT; index++)
    {
        alertValues[index] = model->getParamAlertValue(index);
    }
}

void ParamPresenter::adjustEditingValue(int direction)
{
    const uint16_t minValue = model->getParamAlertMin(avisosRow);
    const uint16_t maxValue = model->getParamAlertMax(avisosRow);
    const uint16_t step = model->getParamAlertStep(avisosRow);
    int32_t candidate = static_cast<int32_t>(editingValue);

    candidate += (direction > 0) ? static_cast<int32_t>(step) : -static_cast<int32_t>(step);

    if (candidate < static_cast<int32_t>(minValue))
    {
        candidate = static_cast<int32_t>(minValue);
    }

    if (candidate > static_cast<int32_t>(maxValue))
    {
        candidate = static_cast<int32_t>(maxValue);
    }

    editingValue = static_cast<uint16_t>(candidate);
}
