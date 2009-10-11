#ifndef _SDL13_EVENTMANAGER_H_
#define _SDL13_EVENTMANAGER_H_

#include <string>
#include "GameControls.h"
#include "gameloop.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class SDL13_EventManager : public event_manager::EventManager,
                           public CycledComponent
{
public:
    SDL13_EventManager();
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // CycledComponent implementation
    virtual void cycle();
private:
    void translateMouseEvent(const SDL_Event &sdl_event,
                             event_manager::GameControlEvent &controlEvent);
    int m_idleDx, m_idleDy;
    int m_mouseX, m_mouseY;
};

#endif // _SDL13_EVENTMANAGER_H_

