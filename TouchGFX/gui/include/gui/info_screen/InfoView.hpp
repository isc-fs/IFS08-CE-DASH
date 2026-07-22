#ifndef INFOVIEW_HPP
#define INFOVIEW_HPP

#include <gui_generated/info_screen/InfoViewBase.hpp>
#include <gui/info_screen/InfoPresenter.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

class InfoView : public InfoViewBase
{
public:
    InfoView();
    virtual ~InfoView() {}
    virtual void setupScreen();
    virtual void handleTickEvent();
    virtual void tearDownScreen();
    void setTelemetry(const UiTelemetry& telemetry);
    void setAlertThresholds(const uint16_t* values);
    void setSdLoggingStatus(bool enabled, const char* sessionId);
    void setPage(uint8_t page);
    uint8_t getPage() const;
    void nextPage();
    void previousPage();
    void dismissAlertsPopup();

protected:
    enum
    {
        PAGE_COUNT = 5
    };

    UiTelemetry currentTelemetry;
    bool hasTelemetry;
    bool hasFilteredDriveRpm;
    uint8_t currentPage;
    int32_t filteredDriveRpm;
    uint16_t alertThresholds[5];
    uint8_t activeAlertMask;
    uint8_t dismissedAlertMask;
    bool alertPopupVisible;

    void updatePageVisibility();
    void updateTabStyles();
    void setValueU32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint32_t value);
    void setValueS32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int32_t value);
    void setValueU16(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint16_t value);
    void setValueS16(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int16_t value);
    void setValueU8(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint8_t value);
    void setPairU32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint32_t first, uint32_t second);
    void setPairS32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int32_t first, int32_t second);
    void updateTelemetryRows();
    void updateAlertsPopup();
};

#endif // INFOVIEW_HPP
