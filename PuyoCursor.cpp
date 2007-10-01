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
        SetCursorPosition(event->sdl_event.motion.x,  event->sdl_event.motion.y);
        break;
    case SDL_JOYAXISMOTION:
        if (event->sdl_event.jaxis.axis == 2) {
            idleDy = event->sdl_event.jaxis.value / 5000;
        }
        else if (event->sdl_event.jaxis.axis == 3) {
            idleDx = event->sdl_event.jaxis.value / 5000;
        }
        break;
    default:
        break;
    }
}

void PuyoCursor::cycle()
{
    if ((blitX * blitX) + (blitY * blitY) < 2)
        return;
    SetCursorPosition(blitX + idleDx, blitY + idleDy);
    blitAngle += (tgtBlitAngle - blitAngle) / 3.;
}

void PuyoCursor::SetCursorPosition(int x, int y)
{
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
}

