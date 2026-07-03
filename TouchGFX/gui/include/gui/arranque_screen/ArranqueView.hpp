#ifndef ARRANQUEVIEW_HPP
#define ARRANQUEVIEW_HPP

#include <gui_generated/arranque_screen/ArranqueViewBase.hpp>
#include <gui/arranque_screen/ArranquePresenter.hpp>

class ArranqueView : public ArranqueViewBase
{
public:
    ArranqueView();
    virtual ~ArranqueView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
    void applyTheme();
};

#endif // ARRANQUEVIEW_HPP
