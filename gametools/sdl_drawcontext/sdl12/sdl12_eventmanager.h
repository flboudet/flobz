#ifndef _SDL12_EVENTMANAGER_H_
#define _SDL12_EVENTMANAGER_H_

#include <string>
#include "GameControls.h"
#include "gameloop.h"

#include <SDL/SDL.h>

class SDL12_EventManager : public event_manager::EventManager,
                           public CycledComponent
{
public:
    SDL12_EventManager();
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // Control settings handling
    virtual ios_fc::String getControlName(int controlType, bool alternate);
    virtual bool   changeControl(int controlType, bool alternate, event_manager::GameControlEvent &event);
    virtual void   saveControls();
    // CycledComponent implementation
    virtual void cycle();
private:
    void translateMouseEvent(const SDL_Event &sdl_event,
                             event_manager::GameControlEvent &controlEvent);
    int m_idleDx, m_idleDy;
    int m_mouseX, m_mouseY;
};

#endif // _SDL12_EVENTMANAGER_H_

