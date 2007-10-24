#include <math.h>
#include "GameCursor.h"
#include "gameui.h"
using namespace gameui;

GameCursor::GameCursor(const char *cursorImage) : CycledComponent(0.01), idleDx(0), idleDy(0), visible(true), obscured(true)
{
    cursorSurface = IIM_Load_Absolute_DisplayFormatAlpha (cursorImage);
    blitX = 0;
    blitY = 0;
    prevblitX = 0;
    prevblitY = 0;
    blitAngle = 0;
}

GameCursor::~GameCursor()
{
    IIM_Free(cursorSurface);
}

void GameCursor::draw(SDL_Surface *screen)
{
    if ((obscured) || (!visible))
        return;
    if (moveToFront())
        return;
    float angle = (-blitAngle) + 90;
    float anglerad = angle * M_PI / 180.0;
    //float vy = -1.0;
    //float vx = 0.0;
    //float nvx =  sin(anglerad) * vy;
    //float nvy =  cos(anglerad) * vy;
    //nvx = (nvx * 0.5) * cursorSurface->w;
    //nvy = (nvy * 0.5) * cursorSurface->h;
    float nvx = (-sin(anglerad) * 0.5) * cursorSurface->w;
    float nvy = (-cos(anglerad) * 0.5) * cursorSurface->h;
    IIM_BlitRotatedSurfaceCentered(cursorSurface, angle, screen, blitX - nvx, blitY - nvy);
}

void GameCursor::onEvent(GameControlEvent *event)
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

void GameCursor::cycle()
{
    if ((idleDx * idleDx) + (idleDy * idleDy) < 1)
        return;
    setCursorPosition(blitX + idleDx, blitY + idleDy);
}

void GameCursor::setCursorPosition(int x, int y)
{
    obscured = false;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > this->DrawableComponent::parentLoop->getSurface()->w) x = this->DrawableComponent::parentLoop->getSurface()->w;
    if (y > this->DrawableComponent::parentLoop->getSurface()->h) y = this->DrawableComponent::parentLoop->getSurface()->h;
    // Push an SDL user event corresponding to the moving of our game cursor
    SDL_Event moveEvent;
    moveEvent.type = SDL_USEREVENT;
    moveEvent.user.code = GameCursor::MOVE;
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
    float deltaBlitAngle = tgtBlitAngle - blitAngle;
    if (deltaBlitAngle > 180.)
        deltaBlitAngle -= 360.;
    else if (deltaBlitAngle < -180.)
        deltaBlitAngle += 360.;
    blitAngle += (deltaBlitAngle) / 6.;
    if (blitAngle > 180.)
        blitAngle -= 360.;
    else if (blitAngle < -180.)
        blitAngle += 360.;
}

void GameCursor::click(int x, int y)
{
    // Push an SDL user event corresponding to the click of our game cursor
    SDL_Event clickEvent;
    clickEvent.type = SDL_USEREVENT;
    clickEvent.user.code = GameCursor::CLICK;
    clickEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&clickEvent);
}


