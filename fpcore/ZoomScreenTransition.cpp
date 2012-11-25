#include "ZoomScreenTransition.h"
#include "ios_fc.h"
#include "GTLog.h"

ZoomScreenTransitionWidget::ZoomScreenTransitionWidget(Screen &fromScreen)
    : ScreenTransitionWidget(fromScreen, IMAGE_RGBA),
      m_duration(0.5), m_alpha(1.), m_zoom(1.)
{
    m_initialTime = ios_fc::getUnixTime();
    m_finalTime = m_initialTime + m_duration;
}

ZoomScreenTransitionWidget::~ZoomScreenTransitionWidget()
{
}

void ZoomScreenTransitionWidget::idle(double currentTime)
{
    if (currentTime >= m_finalTime) {
        releaseFromSurface();
        parentLoop->removeIdle(this);
        m_alpha = 0.;
    }
    else {
        m_alpha = (m_finalTime - currentTime) / m_duration;
        m_zoom = 1./m_alpha;
        //m_rotation = 3.1416 * m_alpha;
    }
}

void ZoomScreenTransitionWidget::draw(DrawTarget *dt)
{
    if (m_alpha == 0.)
        return;
    IosSurface *s = getFromSurface();
    IosSurface *ds = s->setAlpha(m_alpha);
    double h = dt->h * m_zoom;
    double w = dt->w * m_zoom;
    IosRect r = {(dt->w - w)/2., (dt->h - h)/2., w, h};
    dt->draw(ds, NULL, &r);
    delete ds;

}

