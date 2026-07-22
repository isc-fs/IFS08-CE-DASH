#include <gui/info_screen/InfoView.hpp>
#include <touchgfx/Color.hpp>

namespace
{
constexpr uint8_t pageHome = 0U;
constexpr uint8_t pageDrive = 1U;
constexpr uint8_t pageBattery = 2U;
constexpr uint8_t pageThermal = 3U;
constexpr uint8_t pageInverter = 4U;
constexpr uint8_t alertMotorTemp = 1U << 0;
constexpr uint8_t alertInverterTemp = 1U << 1;
constexpr uint8_t alertAccuTemp = 1U << 2;
constexpr uint8_t alertSoc = 1U << 3;
constexpr uint8_t alertAccuVoltage = 1U << 4;

const touchgfx::colortype colorMuted = touchgfx::Color::getColorFromRGB(150, 150, 150);
const touchgfx::colortype colorWhite = touchgfx::Color::getColorFromRGB(255, 255, 255);
const touchgfx::colortype colorSdRecording = touchgfx::Color::getColorFromRGB(45, 190, 95);
const touchgfx::colortype colorSdStopped = touchgfx::Color::getColorFromRGB(214, 54, 56);

bool isNoLogSession(const char* sessionId)
{
    return (sessionId == nullptr) ||
           ((sessionId[0] == 'N') && (sessionId[1] == 'O') && (sessionId[2] == 'L') && (sessionId[3] == 'O') && (sessionId[4] == 'G'));
}

void setAsciiText(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, const char* a, const char* b)
{
    if (bufferSize == 0U)
    {
        return;
    }

    uint16_t pos = 0U;
    const char* parts[] = { a, b };
    for (uint8_t part = 0U; part < 2U; ++part)
    {
        for (const char* p = parts[part]; (p != nullptr) && (*p != '\0') && (pos < (bufferSize - 1U)); ++p)
        {
            buffer[pos++] = static_cast<touchgfx::Unicode::UnicodeChar>(*p);
        }
    }
    buffer[pos] = 0U;
}
}

InfoView::InfoView()
    : currentTelemetry(),
      hasTelemetry(false),
      hasFilteredDriveRpm(false),
      currentPage(pageHome),
      filteredDriveRpm(0),
      alertThresholds{0U},
      activeAlertMask(0U),
      dismissedAlertMask(0U),
      alertPopupVisible(false)
{
}

void InfoView::setupScreen()
{
    InfoViewBase::setupScreen();
    setSdLoggingStatus(false, nullptr);
    updatePageVisibility();
    updateTelemetryRows();
    updateAlertsPopup();
}

void InfoView::handleTickEvent()
{
}

void InfoView::tearDownScreen()
{
    InfoViewBase::tearDownScreen();
}

void InfoView::setTelemetry(const UiTelemetry& telemetry)
{
    currentTelemetry = telemetry;
    if (!hasFilteredDriveRpm)
    {
        filteredDriveRpm = telemetry.inverterInvRpm;
        hasFilteredDriveRpm = true;
    }
    else
    {
        filteredDriveRpm += (telemetry.inverterInvRpm - filteredDriveRpm) / 2;
    }
    hasTelemetry = true;
    updateTelemetryRows();
    updateAlertsPopup();
}

void InfoView::setAlertThresholds(const uint16_t* values)
{
    if (values == nullptr)
    {
        return;
    }

    for (uint8_t i = 0U; i < 5U; ++i)
    {
        alertThresholds[i] = values[i];
    }
    updateAlertsPopup();
}

void InfoView::setSdLoggingStatus(bool enabled, const char* sessionId)
{
    const bool noLog = isNoLogSession(sessionId);
    const char* name = noLog ? "NEW LOG" : sessionId;
    const char* prefix = noLog ? "SD NOLOG - " : (enabled ? "SD REC " : "SD STOP ");

    telemetry_footer_box.setColor((enabled && !noLog) ? colorSdRecording : colorSdStopped);
    setAsciiText(telemetry_footer_textBuffer,
                 TELEMETRY_FOOTER_TEXT_SIZE,
                 prefix,
                 name);
    telemetry_footer_text.invalidate();
    telemetry_footer_text.resizeToCurrentText();
    telemetry_footer_text.invalidate();
    telemetry_footer_box.invalidate();
}

void InfoView::nextPage()
{
    currentPage = static_cast<uint8_t>((currentPage + 1U) % PAGE_COUNT);
    updatePageVisibility();
}

void InfoView::previousPage()
{
    currentPage = static_cast<uint8_t>((currentPage == 0U) ? (PAGE_COUNT - 1U) : (currentPage - 1U));
    updatePageVisibility();
}

void InfoView::setPage(uint8_t page)
{
    currentPage = static_cast<uint8_t>(page % PAGE_COUNT);
    updatePageVisibility();
}

uint8_t InfoView::getPage() const
{
    return currentPage;
}

void InfoView::dismissAlertsPopup()
{
    if (activeAlertMask == 0U)
    {
        alertPopupVisible = false;
        alerts_popup.setVisible(false);
        alerts_popup.invalidate();
        return;
    }

    alerts_popup.invalidate();
    if (alertPopupVisible)
    {
        dismissedAlertMask |= activeAlertMask;
        alertPopupVisible = false;
    }
    else
    {
        dismissedAlertMask &= static_cast<uint8_t>(~activeAlertMask);
        alertPopupVisible = true;
    }
    alerts_popup.setVisible(alertPopupVisible);
    alerts_popup.invalidate();
}

void InfoView::updatePageVisibility()
{
    page_home.setVisible(currentPage == pageHome);
    page_drive.setVisible(currentPage == pageDrive);
    page_battery.setVisible(currentPage == pageBattery);
    page_thermal.setVisible(currentPage == pageThermal);
    page_inverter.setVisible(currentPage == pageInverter);

    page_home.invalidate();
    page_drive.invalidate();
    page_battery.invalidate();
    page_thermal.invalidate();
    page_inverter.invalidate();

    updateTabStyles();
}

void InfoView::updateTabStyles()
{
    tab_home.setColor((currentPage == pageHome) ? colorWhite : colorMuted);
    tab_drive.setColor((currentPage == pageDrive) ? colorWhite : colorMuted);
    tab_battery.setColor((currentPage == pageBattery) ? colorWhite : colorMuted);
    tab_thermal.setColor((currentPage == pageThermal) ? colorWhite : colorMuted);
    tab_inverter.setColor((currentPage == pageInverter) ? colorWhite : colorMuted);

    tab_home.invalidate();
    tab_drive.invalidate();
    tab_battery.invalidate();
    tab_thermal.invalidate();
    tab_inverter.invalidate();
}

void InfoView::setValueU32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint32_t value)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%u" : "", static_cast<unsigned>(value));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setValueS32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int32_t value)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%d" : "", static_cast<int>(value));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setValueU16(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint16_t value)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%u" : "", static_cast<unsigned>(value));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setValueS16(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int16_t value)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%d" : "", static_cast<int>(value));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setValueU8(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint8_t value)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%u" : "", static_cast<unsigned>(value));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setPairU32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, uint32_t first, uint32_t second)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%u / %u" : "",
                               static_cast<unsigned>(first), static_cast<unsigned>(second));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::setPairS32(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize, touchgfx::TextAreaWithOneWildcard& textArea, int32_t first, int32_t second)
{
    textArea.invalidate();
    touchgfx::Unicode::snprintf(buffer, bufferSize, hasTelemetry ? "%d / %d" : "",
                               static_cast<int>(first), static_cast<int>(second));
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void InfoView::updateTelemetryRows()
{
    setValueU8(home_value_1Buffer, HOME_VALUE_1_SIZE, home_value_1, currentTelemetry.ecuFsmState);
    setValueU8(home_value_2Buffer, HOME_VALUE_2_SIZE, home_value_2, currentTelemetry.amsState);
    setValueU8(home_value_3Buffer, HOME_VALUE_3_SIZE, home_value_3, currentTelemetry.amsOkPrecarga);
    setValueU8(home_value_4Buffer, HOME_VALUE_4_SIZE, home_value_4, currentTelemetry.amsSoc);
    setValueU8(home_value_5Buffer, HOME_VALUE_5_SIZE, home_value_5, currentTelemetry.inverterInvState);
    setValueU8(home_value_6Buffer, HOME_VALUE_6_SIZE, home_value_6, currentTelemetry.inverterInvError);
    setValueU16(home_value_7Buffer, HOME_VALUE_7_SIZE, home_value_7, currentTelemetry.inverterInvDcBusVoltage);
    setValueU16(home_value_8Buffer, HOME_VALUE_8_SIZE, home_value_8, currentTelemetry.ecuAceleracion);
    setValueU16(home_value_9Buffer, HOME_VALUE_9_SIZE, home_value_9, currentTelemetry.ecuSFreno);
    setValueU16(home_value_10Buffer, HOME_VALUE_10_SIZE, home_value_10, currentTelemetry.inverterInvDemCode);
    setValueU8(home_value_11Buffer, HOME_VALUE_11_SIZE, home_value_11, currentTelemetry.inverterInvDemPresent);

    setValueS32(drive_value_1Buffer, DRIVE_VALUE_1_SIZE, drive_value_1, currentTelemetry.inverterInvSpeedActual);
    setValueS32(drive_value_2Buffer, DRIVE_VALUE_2_SIZE, drive_value_2, filteredDriveRpm);
    setValueU16(drive_value_3Buffer, DRIVE_VALUE_3_SIZE, drive_value_3, currentTelemetry.ecuTorqueTotal);
    setValueS32(drive_value_4Buffer, DRIVE_VALUE_4_SIZE, drive_value_4, currentTelemetry.inverterInvCurrentActual);
    setValueU16(drive_value_5Buffer, DRIVE_VALUE_5_SIZE, drive_value_5, currentTelemetry.ecuAceleracion);
    setValueU16(drive_value_6Buffer, DRIVE_VALUE_6_SIZE, drive_value_6, currentTelemetry.ecuSFreno);
    setValueU16(drive_value_7Buffer, DRIVE_VALUE_7_SIZE, drive_value_7, currentTelemetry.gpsSpeed);

    setValueU8(battery_value_1Buffer, BATTERY_VALUE_1_SIZE, battery_value_1, currentTelemetry.amsSoc);
    setValueU16(battery_value_2Buffer, BATTERY_VALUE_2_SIZE, battery_value_2, currentTelemetry.amsVCeldaMin);
    setValueS16(battery_value_3Buffer, BATTERY_VALUE_3_SIZE, battery_value_3, currentTelemetry.amsCorrienteAccu);
    setValueS16(battery_value_4Buffer, BATTERY_VALUE_4_SIZE, battery_value_4, currentTelemetry.amsCorrienteDcdc);
    setValueU16(battery_value_5Buffer, BATTERY_VALUE_5_SIZE, battery_value_5, currentTelemetry.inverterInvDcBusVoltage);
    setValueU16(battery_value_6Buffer, BATTERY_VALUE_6_SIZE, battery_value_6, currentTelemetry.amsVminModulo[0]);
    setValueU16(battery_value_7Buffer, BATTERY_VALUE_7_SIZE, battery_value_7, currentTelemetry.amsVminModulo[1]);
    setValueU16(battery_value_8Buffer, BATTERY_VALUE_8_SIZE, battery_value_8, currentTelemetry.amsVminModulo[2]);
    setValueU16(battery_value_9Buffer, BATTERY_VALUE_9_SIZE, battery_value_9, currentTelemetry.amsVminModulo[3]);
    setValueU16(battery_value_10Buffer, BATTERY_VALUE_10_SIZE, battery_value_10, currentTelemetry.amsVminModulo[4]);
    setValueU16(battery_value_11Buffer, BATTERY_VALUE_11_SIZE, battery_value_11, currentTelemetry.amsVmaxModulo[0]);
    setValueU16(battery_value_12Buffer, BATTERY_VALUE_12_SIZE, battery_value_12, currentTelemetry.amsVmaxModulo[1]);
    setValueU16(battery_value_13Buffer, BATTERY_VALUE_13_SIZE, battery_value_13, currentTelemetry.amsVmaxModulo[2]);
    setValueU16(battery_value_14Buffer, BATTERY_VALUE_14_SIZE, battery_value_14, currentTelemetry.amsVmaxModulo[3]);
    setValueU16(battery_value_15Buffer, BATTERY_VALUE_15_SIZE, battery_value_15, currentTelemetry.amsVmaxModulo[4]);

    setValueS16(thermal_value_1Buffer, THERMAL_VALUE_1_SIZE, thermal_value_1, currentTelemetry.inverterInvMotorTemp);
    setValueS16(thermal_value_2Buffer, THERMAL_VALUE_2_SIZE, thermal_value_2, currentTelemetry.inverterInvIgbtTemp);
    setValueS16(thermal_value_3Buffer, THERMAL_VALUE_3_SIZE, thermal_value_3, currentTelemetry.inverterInvAirTemp);
    setValueS16(thermal_value_4Buffer, THERMAL_VALUE_4_SIZE, thermal_value_4, currentTelemetry.amsTempDcdc);
    setValueS16(thermal_value_5Buffer, THERMAL_VALUE_5_SIZE, thermal_value_5, currentTelemetry.amsTempMaxModulo[0]);
    setValueS16(thermal_value_6Buffer, THERMAL_VALUE_6_SIZE, thermal_value_6, currentTelemetry.amsTempMaxModulo[1]);
    setValueS16(thermal_value_7Buffer, THERMAL_VALUE_7_SIZE, thermal_value_7, currentTelemetry.amsTempMaxModulo[2]);
    setValueS16(thermal_value_8Buffer, THERMAL_VALUE_8_SIZE, thermal_value_8, currentTelemetry.amsTempMaxModulo[3]);
    setValueS16(thermal_value_9Buffer, THERMAL_VALUE_9_SIZE, thermal_value_9, currentTelemetry.amsTempMaxModulo[4]);
    setValueS16(thermal_value_10Buffer, THERMAL_VALUE_10_SIZE, thermal_value_10, currentTelemetry.inverterInvMotor2Temp);

    setPairS32(inverter_value_1Buffer, INVERTER_VALUE_1_SIZE, inverter_value_1,
               currentTelemetry.inverterInvCurrentDRaw, currentTelemetry.inverterInvCurrentQRaw);
    setPairS32(inverter_value_2Buffer, INVERTER_VALUE_2_SIZE, inverter_value_2,
               currentTelemetry.inverterInvVoltModulusPermil, currentTelemetry.inverterInvAcBusPowerW);
    setPairS32(inverter_value_3Buffer, INVERTER_VALUE_3_SIZE, inverter_value_3,
               currentTelemetry.inverterInvTorqueMaxFeasNdm, currentTelemetry.inverterInvTorqueEstNm);
    setPairS32(inverter_value_4Buffer, INVERTER_VALUE_4_SIZE, inverter_value_4,
               currentTelemetry.inverterInvSetpointDRaw, currentTelemetry.inverterInvSetpointQRaw);
    setPairU32(inverter_value_5Buffer, INVERTER_VALUE_5_SIZE, inverter_value_5,
               currentTelemetry.inverterInvPwrstgBitState, currentTelemetry.inverterInvFocBitState);
    setPairU32(inverter_value_6Buffer, INVERTER_VALUE_6_SIZE, inverter_value_6,
               currentTelemetry.inverterInvUptimeMs, currentTelemetry.inverterInvKl30Mv);
    setPairU32(inverter_value_7Buffer, INVERTER_VALUE_7_SIZE, inverter_value_7,
               currentTelemetry.inverterInvCore0LoadPct, currentTelemetry.inverterInvCore1LoadPct);
    setPairU32(inverter_value_8Buffer, INVERTER_VALUE_8_SIZE, inverter_value_8,
               currentTelemetry.inverterInvCmdSrc, currentTelemetry.inverterInvCtrlType);
    setPairU32(inverter_value_9Buffer, INVERTER_VALUE_9_SIZE, inverter_value_9,
               currentTelemetry.inverterInvCtrlMode, currentTelemetry.inverterInvPosFbSrc);
}

void InfoView::updateAlertsPopup()
{
    uint16_t pos = 0U;
    uint8_t mask = 0U;

    auto append = [&](uint8_t bit, const char* text)
    {
        mask |= bit;
        if (pos != 0U && pos < (ALERTS_POPUP_TEXT_SIZE - 1U))
        {
            alerts_popup_textBuffer[pos++] = '\n';
        }
        for (const char* p = text; (*p != '\0') && (pos < (ALERTS_POPUP_TEXT_SIZE - 1U)); ++p)
        {
            alerts_popup_textBuffer[pos++] = static_cast<touchgfx::Unicode::UnicodeChar>(*p);
        }
    };

    alerts_popup.invalidate();
    if (hasTelemetry)
    {
        if ((alertThresholds[0] != 0U) && (currentTelemetry.inverterInvMotorTemp >= static_cast<int16_t>(alertThresholds[0])))
        {
            append(alertMotorTemp, "Motor temp limit");
        }
        if ((alertThresholds[1] != 0U) && (currentTelemetry.inverterInvIgbtTemp >= static_cast<int16_t>(alertThresholds[1])))
        {
            append(alertInverterTemp, "Inverter temp limit");
        }
        if (alertThresholds[2] != 0U)
        {
            for (uint8_t i = 0U; i < 5U; ++i)
            {
                if (currentTelemetry.amsTempMaxModulo[i] >= static_cast<int16_t>(alertThresholds[2]))
                {
                    append(alertAccuTemp, "Accu temp limit");
                    break;
                }
            }
        }
        if ((alertThresholds[3] != 0U) && (currentTelemetry.amsSoc <= alertThresholds[3]))
        {
            append(alertSoc, "SOC limit");
        }
        if ((alertThresholds[4] != 0U) && (currentTelemetry.amsVCeldaMin <= alertThresholds[4]))
        {
            append(alertAccuVoltage, "Accu voltage limit");
        }
    }

    alerts_popup_textBuffer[pos] = 0U;
    dismissedAlertMask &= mask;
    activeAlertMask = mask;
    alertPopupVisible = ((mask & ~dismissedAlertMask) != 0U);
    alerts_indicator_box.setVisible(mask != 0U);
    alerts_indicator_text.setVisible(mask != 0U);
    alerts_popup.setVisible(alertPopupVisible);
    alerts_popup_text.resizeToCurrentText();
    alerts_indicator_box.invalidate();
    alerts_indicator_text.invalidate();
    alerts_popup.invalidate();
}
