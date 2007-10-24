#include <math.h>
#include "PuyoCursor.h"
#include "gameui.h"
using namespace gameui;

PuyoCursor::PuyoCursor(const char *cursorImage) : CycledComponent(0.01), idleDx(0), idleDy(0)
{
    cursorSurface = IIM_Load_Absolute_DisplayFormatAlpha (cursorImage);
    blitX = 0;
    blitY = 0;
    prevblitX = 0;
    prevblitY = 0;
    blitAngle = 0;
}

PuyoCursor::~PuyoCursor()
{
    IIM_Free(cursorSurface);
}

void PuyoCursor::draw(SDL_Surface *screen)
{
    if (moveToFront())
        return;
    IIM_BlitRotatedSurfaceCentered(cursorSurface, (-blitAngle) + 90, screen, blitX, blitY);
}

void PuyoCursor::onEvent(GameControlEvent *event)
{
    switch (event->sdl_event.type) {
    case SDL_MOUSEMOTION:
        setCursorPosition(event->sdl_event.motion.x,  event->sdl_event.motion.y);
        break;
    case SDL_JOYAXISMOTION:
        if (event->sdl_event.jaxis.axis == 2) {
            idleDy = event->sdl_event.jaxis.value / 5000;
        }
        else if (event->sdl_event.jaxis.axis == 3) {
            idleDx = event->sdl_event.jaxis.value / 5000;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (event->sdl_event.button.button == SDL_BUTTON_LEFT) {
            click(event->sdl_event.button.x, event->sdl_event.button.y);
        }
        break;
    default:
        break;
    }
}

void PuyoCursor::cycle()
{
    if ((idleDx * idleDx) + (idleDy * idleDy) < 1)
        return;
    setCursorPosition(blitX + idleDx, blitY + idleDy);
}

void PuyoCursor::setCursorPosition(int x, int y)
{
    // Push an SDL user event corresponding to the moving of our game cursor
    SDL_Event moveEvent;
    moveEvent.type = SDL_USEREVENT;
    moveEvent.user.code = PuyoCursor::MOVE;
    moveEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&moveEvent);
    
    blitX = x;
    blitY = y;
    int dx = prevblitX - blitX;
    int dy = prevblitY - blitY;
    if (dx*dx + dy*dy > 1000) {
        if ((dx >= 0) && (dy >= 0)) {
            tgtBlitAngle = (atan((float)dy / (float)dx))/ 3.1416 * 180.;
        }
        else if ((dx >= 0) && (dy < 0)) {
            tgtBlitAngle = 360. - (atan(- (float)dy / (float)dx))/ 3.1416 * 180.;
        }
        else if ((dx < 0) && (dy >= 0)) {
            tgtBlitAngle = 180. - (atan(- (float)dy / (float)dx))/ 3.1416 * 180.;
        }
        else {
            tgtBlitAngle = 180. + (atan((float)dy / (float)dx))/ 3.1416 * 180.;
        }
        prevblitX = blitX;
        prevblitY = blitY;
    }
    blitAngle += (tgtBlitAngle - blitAngle) / 3.;
}

void PuyoCursor::click(int x, int y)
{
    // Push an SDL user event corresponding to the click of our game cursor
    SDL_Event clickEvent;
    clickEvent.type = SDL_USEREVENT;
    clickEvent.user.code = PuyoCursor::CLICK;
    clickEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&clickEvent);
}


