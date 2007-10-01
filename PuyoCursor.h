#ifndef _PUYOCURSOR_H
#define _PUYOCURSOR_H

#include "gameloop.h"
#include "IosImgProcess.h"

class PuyoCursor : public DrawableComponent, public CycledComponent
{
public:
    PuyoCursor(const char *cursorImage);
    virtual ~PuyoCursor();
    virtual void onEvent(GameControlEvent *event);
    virtual void cycle();
protected:
    virtual void draw(SDL_Surface *screen);
private:
    void SetCursorPosition(int x, int y);
    
    IIM_Surface * cursorSurface;
    int blitX, blitY;
    int prevblitX, prevblitY;
    float blitAngle, tgtBlitAngle;
    int idleDx, idleDy;
};

#endif // _PUYOCURSOR_H

