#ifndef _ROTATESCREENTRANSITION_H_
#define _ROTATESCREENTRANSITION_H_

#include "ScreenTransition.h"

/**
 * Default implementation (doom melt effect)
 */
class ZoomScreenTransitionWidget : public ScreenTransitionWidget, public IdleComponent {
public:
    ZoomScreenTransitionWidget(Screen &fromScreen);
    virtual ~ZoomScreenTransitionWidget();
    void idle(double currentTime);
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
private:
    double m_initialTime, m_duration, m_finalTime;
    double m_alpha;
    double m_zoom;
};

#endif // _ROTATESCREENTRANSITION_H_
