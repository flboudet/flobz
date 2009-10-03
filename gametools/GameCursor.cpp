#include <math.h>
#include "GameCursor.h"
#include "gameui.h"
using namespace gameui;
using namespace event_manager;
using namespace Cursor;

GameCursor::GameCursor(const char *cursorImage) : CycledComponent(0.01), idleDx(0), idleDy(0), visible(true), obscured(true)
{
    IIMLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getIIMLibrary();
    cursorSurface = iimLib.load_Absolute_DisplayFormatAlpha(cursorImage);
    blitX = 0;
    blitY = 0;
    prevblitX = 0;
    prevblitY = 0;
    blitAngle = 0;
}

GameCursor::~GameCursor()
{
    delete cursorSurface;
}

void GameCursor::draw(DrawTarget *dt)
{
    if ((obscured) || (!visible))
        return;
    if (moveToFront())
        return;
    float angle = (-blitAngle) + 90;
    float anglerad = angle * M_PI / 180.0;
    float nvx = (-sin(anglerad) * 0.5) * cursorSurface->w;
    float nvy = (-cos(anglerad) * 0.5) * cursorSurface->h;
    dt->renderRotatedCentered(cursorSurface, (int)angle, (int)(blitX - nvx), (int)(blitY - nvy));
}

void GameCursor::onEvent(GameControlEvent *event)
{
#ifdef TODO
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
            pushMouseEvent(event->sdl_event.button.x, event->sdl_event.button.y, MOUSE_DOWN);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (event->sdl_event.button.button == SDL_BUTTON_LEFT) {
            pushMouseEvent(event->sdl_event.button.x, event->sdl_event.button.y, MOUSE_UP);
        }
        break;
    default:
        break;
    }
#endif
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
    if (x > this->DrawableComponent::parentLoop->getDrawContext()->getWidth())
        x = this->DrawableComponent::parentLoop->getDrawContext()->getWidth();
    if (y > this->DrawableComponent::parentLoop->getDrawContext()->getHeight())
        y = this->DrawableComponent::parentLoop->getDrawContext()->getHeight();
    // Push an SDL user event corresponding to the moving of our game cursor
    SDL_Event moveEvent;
    moveEvent.type = SDL_USEREVENT;
    moveEvent.user.code = MOVE;
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

void GameCursor::pushMouseEvent(int x, int y, int eventType)
{
    // Push an SDL user event corresponding to the click of our game cursor
    SDL_Event mouseEvent;
    mouseEvent.type = SDL_USEREVENT;
    mouseEvent.user.code = eventType;
    mouseEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&mouseEvent);
}


