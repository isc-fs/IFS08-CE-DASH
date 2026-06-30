#ifndef PARAMVIEW_HPP
#define PARAMVIEW_HPP

#include <gui_generated/param_screen/ParamViewBase.hpp>
#include <gui/param_screen/ParamPresenter.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

class ParamView : public ParamViewBase
{
public:
    ParamView();
    virtual ~ParamView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    void showPage(uint8_t pageIndex, bool menuActive);
    void updateAlertValues(const uint16_t* values);
    void updateConfigValues(const uint16_t* values, const char* sessionId);
    void setAlertSelection(int selectedIndex, bool contentActive, bool editing);
    void setConfigSelection(int selectedIndex, bool contentActive, bool editing);
    void flashConfigIcon(uint8_t row);
protected:
    uint8_t blinkTickCounter;
    bool blinkVisible;
    bool blinkEnabled;
    int blinkingAlertRow;
    int blinkingConfigRow;
    bool blinkingConfigActive;
    uint8_t configFlashTicks;
    uint8_t configFlashRow;

    void applyMenuStyles(uint8_t pageIndex, bool menuActive);
    void applyAlertRowStyles(int selectedIndex, bool contentActive, bool editing);
    void applyConfigRowStyles(int selectedIndex, bool contentActive, bool editing);
    void setAlertRowColor(uint8_t index, touchgfx::colortype color);
    void setConfigRowColor(uint8_t index, touchgfx::colortype color);
    void setAlertIndexVisible(uint8_t index, bool visible);
    void setConfigIndexVisible(uint8_t index, bool visible);
    void setAlertRowVisible(uint8_t index, bool visible);
    void setConfigRowVisible(uint8_t index, bool visible);
    void setSdFooterStatus(bool enabled, const char* sessionId);
    void setAsciiValue(touchgfx::TextAreaWithOneWildcard& textArea,
                       touchgfx::Unicode::UnicodeChar* buffer,
                       uint16_t bufferSize,
                       const char* value);
    void setNumericValue(touchgfx::TextAreaWithOneWildcard& textArea,
                         touchgfx::Unicode::UnicodeChar* buffer,
                         uint16_t bufferSize,
                         uint16_t value);
};

#endif // PARAMVIEW_HPP
