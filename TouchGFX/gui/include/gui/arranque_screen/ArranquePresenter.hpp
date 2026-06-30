#ifndef ARRANQUEPRESENTER_HPP
#define ARRANQUEPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class ArranqueView;

class ArranquePresenter : public touchgfx::Presenter, public ModelListener
{
public:
    ArranquePresenter(ArranqueView& v);

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

    virtual ~ArranquePresenter() {}

private:
    ArranquePresenter();

    ArranqueView& view;
};

#endif // ARRANQUEPRESENTER_HPP
