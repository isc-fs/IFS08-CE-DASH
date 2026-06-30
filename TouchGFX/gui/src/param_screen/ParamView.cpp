#include <gui/param_screen/ParamView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/Unicode.hpp>

namespace
{
constexpr uint8_t paramPageAvisos = 0U;
constexpr uint8_t paramPageConfiguracion = 1U;
constexpr uint8_t blinkPeriodTicks = 20U;
constexpr touchgfx::TypedTextId textFinishId = T_PARAM_FINISH;
constexpr touchgfx::TypedTextId textStartId = T_PARAM_START;

const touchgfx::colortype colorWhite = touchgfx::Color::getColorFromRGB(255, 255, 255);
const touchgfx::colortype colorBlack = touchgfx::Color::getColorFromRGB(0, 0, 0);
const touchgfx::colortype colorDarkPanel = touchgfx::Color::getColorFromRGB(18, 18, 18);
const touchgfx::colortype colorDarkPanelAlt = touchgfx::Color::getColorFromRGB(28, 28, 28);
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

ParamView::ParamView()
    : blinkTickCounter(0U),
      blinkVisible(true),
      blinkEnabled(false),
      blinkingAlertRow(-1),
      blinkingConfigRow(-1),
      blinkingConfigActive(false),
      configFlashTicks(0U),
      configFlashRow(0U)
{
}

void ParamView::setupScreen()
{
    ParamViewBase::setupScreen();
    blinkTickCounter = 0U;
    blinkVisible = true;
    blinkEnabled = false;
    blinkingAlertRow = -1;
    blinkingConfigRow = -1;
    blinkingConfigActive = false;
    configFlashTicks = 0U;
    configFlashRow = 0U;

    __background.setColor(colorBlack);
    box1.setColor(colorBlack);
    box2.setColor(colorDarkPanel);
    box2_1.setColor(colorDarkPanelAlt);
    textArea1.setColor(colorWhite);
    textArea1_2.setColor(colorWhite);
    __background.invalidate();
    box1.invalidate();
    box2.invalidate();
    box2_1.invalidate();
    textArea1.invalidate();
    textArea1_2.invalidate();

    Temperatura_limite_Inversor_1_1.invalidate();

    showPage(paramPageAvisos, true);
    setAlertSelection(-1, false, false);
    setConfigSelection(-1, false, false);
    setSdFooterStatus(false, nullptr);

    scalableImage1.invalidate();
    scalableImage1_1.invalidate();
    IndiceTmotor.invalidate();
    IndiceTacumulador.invalidate();
    IndiceSOC.invalidate();
    IndiceVminAcumulador.invalidate();
    IndiceTinversor.invalidate();
    IndiceTinversor_1.invalidate();
    IndiceNewLog.invalidate();
}

void ParamView::handleTickEvent()
{
    if (configFlashTicks > 0U)
    {
        configFlashTicks--;
        setConfigIndexVisible(configFlashRow, configFlashTicks == 0U);
    }

    if (!blinkEnabled)
    {
        return;
    }

    blinkTickCounter++;
    if (blinkTickCounter < blinkPeriodTicks)
    {
        return;
    }

    blinkTickCounter = 0U;
    blinkVisible = !blinkVisible;
    if (blinkingConfigActive)
    {
        applyConfigRowStyles(blinkingConfigRow, true, true);
    }
    else
    {
        applyAlertRowStyles(blinkingAlertRow, true, true);
    }
}

void ParamView::tearDownScreen()
{
    ParamViewBase::tearDownScreen();
}

void ParamView::showPage(uint8_t pageIndex, bool menuActive)
{
    avisos_container.setVisible(pageIndex == paramPageAvisos);
    Configuracion_container.setVisible(pageIndex == paramPageConfiguracion);
    avisos_container.invalidate();
    Configuracion_container.invalidate();
    applyMenuStyles(pageIndex, menuActive);

    scalableImage1.invalidate();
    scalableImage1_1.invalidate();

    if (pageIndex == paramPageAvisos)
    {
        IndiceTmotor.invalidate();
        IndiceTacumulador.invalidate();
        IndiceSOC.invalidate();
        IndiceVminAcumulador.invalidate();
        IndiceTinversor.invalidate();
    }
    else
    {
        IndiceTinversor_1.invalidate();
        IndiceNewLog.invalidate();
    }
}

void ParamView::updateAlertValues(const uint16_t* values)
{
    if (values == 0)
    {
        return;
    }

    setNumericValue(TLMSET, TLMSETBuffer, TLMSET_SIZE, values[0]);
    setNumericValue(TLISET, TLISETBuffer, TLISET_SIZE, values[1]);
    setNumericValue(TLASET, TLASETBuffer, TLASET_SIZE, values[2]);
    setNumericValue(SOCSET, SOCSETBuffer, SOCSET_SIZE, values[3]);
    setNumericValue(VASET, VASETBuffer, VASET_SIZE, values[4]);
}

void ParamView::updateConfigValues(const uint16_t* values, const char* sessionId)
{
    if (values == 0)
    {
        return;
    }

    Temperatura_limite_Inversor_1_1.setTypedText(touchgfx::TypedText((values[0] != 0U) ? textFinishId : textStartId));
    Temperatura_limite_Inversor_1_1.invalidate();
    setSdFooterStatus(values[0] != 0U, sessionId);
    setAsciiValue(ParamSessionValue,
                  ParamSessionValueBuffer,
                  PARAMSESSIONVALUE_SIZE,
                  sessionId);
}

void ParamView::setAlertSelection(int selectedIndex, bool contentActive, bool editing)
{
    applyAlertRowStyles(selectedIndex, contentActive, editing);
}

void ParamView::setConfigSelection(int selectedIndex, bool contentActive, bool editing)
{
    applyConfigRowStyles(selectedIndex, contentActive, editing);
}

void ParamView::flashConfigIcon(uint8_t row)
{
    configFlashRow = row;
    configFlashTicks = blinkPeriodTicks;
    setConfigIndexVisible(configFlashRow, false);
}

void ParamView::applyMenuStyles(uint8_t pageIndex, bool menuActive)
{
    scalableImage1.setVisible(menuActive && (pageIndex == paramPageAvisos));
    scalableImage1_1.setVisible(menuActive && (pageIndex == paramPageConfiguracion));

    scalableImage1.invalidate();
    scalableImage1_1.invalidate();
}

void ParamView::applyAlertRowStyles(int selectedIndex, bool contentActive, bool editing)
{
    const touchgfx::colortype textColor = colorWhite;

    blinkEnabled = editing && contentActive && (selectedIndex >= 0);
    blinkingAlertRow = blinkEnabled ? selectedIndex : -1;
    blinkingConfigRow = -1;
    blinkingConfigActive = false;

    if (!blinkEnabled)
    {
        blinkTickCounter = 0U;
        blinkVisible = true;
    }

    for (uint8_t row = 0U; row < 5U; row++)
    {
        const bool isSelected = contentActive && (selectedIndex == static_cast<int>(row));

        setAlertRowColor(row, textColor);
        setAlertRowVisible(row, true);
        setAlertIndexVisible(row, isSelected && (!editing || blinkVisible));
    }
}

void ParamView::applyConfigRowStyles(int selectedIndex, bool contentActive, bool editing)
{
    const touchgfx::colortype textColor = colorWhite;

    blinkEnabled = editing && contentActive && (selectedIndex >= 0);
    blinkingConfigRow = blinkEnabled ? selectedIndex : -1;
    blinkingConfigActive = blinkEnabled;
    blinkingAlertRow = -1;

    if (!blinkEnabled)
    {
        blinkTickCounter = 0U;
        blinkVisible = true;
    }

    for (uint8_t row = 0U; row < 2U; row++)
    {
        const bool isSelected = contentActive && (selectedIndex == static_cast<int>(row));
        setConfigRowColor(row, textColor);
        setConfigRowVisible(row, true);
        setConfigIndexVisible(row, isSelected && (!editing || blinkVisible));
    }
}

void ParamView::setAlertRowColor(uint8_t index, touchgfx::colortype color)
{
    switch (index)
    {
    case 0U:
        Temperatura_limite_Motor.setColor(color);
        TLMSET.setColor(color);
        Temperatura_limite_Motor.invalidate();
        TLMSET.invalidate();
        break;
    case 1U:
        Temperatura_limite_Inversor.setColor(color);
        TLISET.setColor(color);
        Temperatura_limite_Inversor.invalidate();
        TLISET.invalidate();
        break;
    case 2U:
        Temperatura_limite_Acumulador.setColor(color);
        TLASET.setColor(color);
        Temperatura_limite_Acumulador.invalidate();
        TLASET.invalidate();
        break;
    case 3U:
        Estado_de_carga.setColor(color);
        SOCSET.setColor(color);
        Estado_de_carga.invalidate();
        SOCSET.invalidate();
        break;
    case 4U:
        Tension_minima_Acumulador.setColor(color);
        VASET.setColor(color);
        Tension_minima_Acumulador.invalidate();
        VASET.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setConfigRowColor(uint8_t index, touchgfx::colortype color)
{
    switch (index)
    {
    case 0U:
        Temperatura_limite_Inversor_1.setColor(color);
        Temperatura_limite_Inversor_1_1.setColor(color);
        Temperatura_limite_Inversor_1.invalidate();
        Temperatura_limite_Inversor_1_1.invalidate();
        break;
    case 1U:
        ParamNewLogLabel.setColor(color);
        ParamNewLogLabel.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setAlertIndexVisible(uint8_t index, bool visible)
{
    switch (index)
    {
    case 0U:
        IndiceTmotor.setVisible(visible);
        IndiceTmotor.invalidate();
        break;
    case 1U:
        IndiceTinversor.setVisible(visible);
        IndiceTinversor.invalidate();
        break;
    case 2U:
        IndiceTacumulador.setVisible(visible);
        IndiceTacumulador.invalidate();
        break;
    case 3U:
        IndiceSOC.setVisible(visible);
        IndiceSOC.invalidate();
        break;
    case 4U:
        IndiceVminAcumulador.setVisible(visible);
        IndiceVminAcumulador.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setConfigIndexVisible(uint8_t index, bool visible)
{
    switch (index)
    {
    case 0U:
        IndiceTinversor_1.setVisible(visible);
        IndiceTinversor_1.invalidate();
        break;
    case 1U:
        IndiceNewLog.setVisible(visible);
        IndiceNewLog.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setAlertRowVisible(uint8_t index, bool visible)
{
    switch (index)
    {
    case 0U:
        Temperatura_limite_Motor.setVisible(visible);
        TLMSET.setVisible(visible);
        Temperatura_limite_Motor.invalidate();
        TLMSET.invalidate();
        break;
    case 1U:
        Temperatura_limite_Inversor.setVisible(visible);
        TLISET.setVisible(visible);
        Temperatura_limite_Inversor.invalidate();
        TLISET.invalidate();
        break;
    case 2U:
        Temperatura_limite_Acumulador.setVisible(visible);
        TLASET.setVisible(visible);
        Temperatura_limite_Acumulador.invalidate();
        TLASET.invalidate();
        break;
    case 3U:
        Estado_de_carga.setVisible(visible);
        SOCSET.setVisible(visible);
        Estado_de_carga.invalidate();
        SOCSET.invalidate();
        break;
    case 4U:
        Tension_minima_Acumulador.setVisible(visible);
        VASET.setVisible(visible);
        Tension_minima_Acumulador.invalidate();
        VASET.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setConfigRowVisible(uint8_t index, bool visible)
{
    switch (index)
    {
    case 0U:
        Temperatura_limite_Inversor_1.setVisible(visible);
        Temperatura_limite_Inversor_1_1.setVisible(visible);
        Temperatura_limite_Inversor_1.invalidate();
        Temperatura_limite_Inversor_1_1.invalidate();
        break;
    case 1U:
        ParamNewLogLabel.setVisible(visible);
        ParamNewLogLabel.invalidate();
        break;
    default:
        break;
    }
}

void ParamView::setSdFooterStatus(bool enabled, const char* sessionId)
{
    const bool noLog = isNoLogSession(sessionId);
    const char* name = noLog ? "NEW LOG" : sessionId;
    const char* prefix = noLog ? "SD NOLOG - " : (enabled ? "SD REC " : "SD STOP ");

    param_footer_box.setColor((enabled && !noLog) ? colorSdRecording : colorSdStopped);
    setAsciiText(param_footer_textBuffer, PARAM_FOOTER_TEXT_SIZE, prefix, name);
    param_footer_text.invalidate();
    param_footer_text.resizeToCurrentText();
    param_footer_text.invalidate();
    param_footer_box.invalidate();
}

void ParamView::setAsciiValue(touchgfx::TextAreaWithOneWildcard& textArea,
                              touchgfx::Unicode::UnicodeChar* buffer,
                              uint16_t bufferSize,
                              const char* value)
{
    uint16_t pos = 0U;
    const char* src = (value != 0) ? value : "NOLOG";

    if (bufferSize == 0U)
    {
        return;
    }

    textArea.invalidate();
    while ((*src != '\0') && (pos < (bufferSize - 1U)))
    {
        buffer[pos++] = static_cast<touchgfx::Unicode::UnicodeChar>(*src++);
    }
    buffer[pos] = 0U;
    textArea.resizeToCurrentText();
    textArea.invalidate();
}

void ParamView::setNumericValue(touchgfx::TextAreaWithOneWildcard& textArea,
                                touchgfx::Unicode::UnicodeChar* buffer,
                                uint16_t bufferSize,
                                uint16_t value)
{
    Unicode::snprintf(buffer, bufferSize, "%u", value);
    textArea.resizeToCurrentText();
    textArea.invalidate();
}
