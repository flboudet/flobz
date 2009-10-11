#include "sdl13_eventmanager.h"

using namespace event_manager;

struct CursorEventArg {
    CursorEventArg(int x, int y) : x(x), y(y) {}
    int x, y;
};

SDL13_EventManager::SDL13_EventManager()
    : CycledComponent(0.01), m_idleDx(0), m_idleDy(0),
      m_mouseX(0), m_mouseY(0)
{}

bool SDL13_EventManager::pollEvent(GameControlEvent &controlEvent)
{
    SDL_Event e;
    bool result = SDL_PollEvent(&e);
    // Try to translate the SDL event into a game event
    // Try to translate the SDL event into a cursor event
    // Try to translate the SDL event into a keyboard event
    getControlEvent(e, &controlEvent);
    // Try to translate the SDL event into a mouse event
    translateMouseEvent(e, controlEvent);
    return result;
}

void SDL13_EventManager::pushMouseEvent(int x, int y,
                                        CursorEventType type)
{
    // Push an SDL user event
    SDL_Event mouseEvent;
    mouseEvent.type = SDL_USEREVENT;
    mouseEvent.user.code = type;
    mouseEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&mouseEvent);
}

void SDL13_EventManager::translateMouseEvent(const SDL_Event &sdl_event,
                                             GameControlEvent &controlEvent)
{
    switch (sdl_event.type) {
    case SDL_MOUSEMOTION:
        m_mouseX = sdl_event.motion.x;
        m_mouseY = sdl_event.motion.y;
        controlEvent.isUp = false;
        controlEvent.cursorEvent = kGameMouseMoved;
        controlEvent.x = m_mouseX;
        controlEvent.y = m_mouseY;
        break;
    case SDL_JOYAXISMOTION:
        if (sdl_event.jaxis.axis == 2) {
            m_idleDy = sdl_event.jaxis.value / 5000;
        }
        else if (sdl_event.jaxis.axis == 3) {
            m_idleDx = sdl_event.jaxis.value / 5000;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (sdl_event.button.button == SDL_BUTTON_LEFT) {
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseDown;
            controlEvent.x = sdl_event.button.x;
            controlEvent.y = sdl_event.button.y;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (sdl_event.button.button == SDL_BUTTON_LEFT) {
            controlEvent.isUp = true;
            controlEvent.cursorEvent = kGameMouseUp;
            controlEvent.x = sdl_event.button.x;
            controlEvent.y = sdl_event.button.y;
        }
        break;
    case SDL_USEREVENT: {
        switch (sdl_event.user.code) {
        case kGameMouseMoved:
        case kGameMouseDown:
        case kGameMouseUp: {
            CursorEventArg *arg = (CursorEventArg *)sdl_event.user.data1;
            controlEvent.x = arg->x;
            controlEvent.y = arg->y;
            delete arg;
        } break;
        default:
            break;
        }
        switch (sdl_event.user.code) {
        case kGameMouseMoved:
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseMoved;
            break;
        case kGameMouseDown:
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseDown;
            break;
        case kGameMouseUp:
            controlEvent.isUp = true;
            controlEvent.cursorEvent = kGameMouseUp;
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
}

void SDL13_EventManager::cycle()
{
    if ((m_idleDx * m_idleDx) + (m_idleDy * m_idleDy) < 1)
        return;
    m_mouseX += m_idleDx;
    m_mouseY += m_idleDy;
    pushMouseEvent(m_mouseX, m_mouseY, kGameMouseMoved);
}
