#include <gui/arranque_screen/ArranqueView.hpp>
#include <touchgfx/Color.hpp>

ArranqueView::ArranqueView()
{

}

void ArranqueView::setupScreen()
{
    ArranqueViewBase::setupScreen();
    applyTheme();
}

void ArranqueView::tearDownScreen()
{
    ArranqueViewBase::tearDownScreen();
}

void ArranqueView::applyTheme()
{
    const touchgfx::colortype backgroundColor = touchgfx::Color::getColorFromRGB(0, 0, 0);
    const touchgfx::colortype textColor = touchgfx::Color::getColorFromRGB(255, 255, 255);

    __background.setColor(backgroundColor);
    textArea1.setColor(textColor);
    __background.invalidate();
    textArea1.invalidate();
}
