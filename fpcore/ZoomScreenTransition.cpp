#include "ZoomScreenTransition.h"
#include "ios_fc.h"
#include "GTLog.h"

ZoomScreenTransitionWidget::ZoomScreenTransitionWidget(Screen &fromScreen)
    : ScreenTransitionWidget(fromScreen, IMAGE_RGBA),
      _duration(0.5), _alpha(1.), _zoom(1.)
{
    _initialTime = ios_fc::getUnixTime();
    _finalTime = _initialTime + _duration;
}

ZoomScreenTransitionWidget::~ZoomScreenTransitionWidget()
{
}

void ZoomScreenTransitionWidget::idle(double currentTime)
{
    if (currentTime >= _finalTime) {
        releaseFromSurface();
        parentLoop->removeIdle(this);
        _alpha = 0.;
    }
    else {
        _alpha = (_finalTime - currentTime) / _duration;
        _zoom = 1./_alpha;
        //_rotation = 3.1416 * _alpha;
    }
}

void ZoomScreenTransitionWidget::draw(DrawTarget *dt)
{
    if (_alpha == 0.)
        return;
    IosSurface *s = getFromSurface();
    IosSurface *ds = s->setAlpha(_alpha);
    double h = dt->h * _zoom;
    double w = dt->w * _zoom;
    IosRect r = {(int)((dt->w - w)/2.), (int)((dt->h - h)/2.), (int)w, (int)h};
    dt->draw(ds, NULL, &r);
    delete ds;

}
