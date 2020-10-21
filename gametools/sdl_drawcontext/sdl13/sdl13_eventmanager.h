#ifndef _SDL13_EVENTMANAGER_H_
#define _SDL13_EVENTMANAGER_H_

#include <string>
#include <memory>
#include "GameControls.h"
#include "gameloop.h"

#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class InputSwitch;

class SDL_GameControlEvent : public event_manager::GameControlEvent
{
public:
    SDL_GameControlEvent();
    virtual GameControlEvent *clone();
    std::auto_ptr<InputSwitch> m_inputSwitch;
};

class SDL13_EventManager : public event_manager::EventManager,
                           public CycledComponent
{
public:
    SDL13_EventManager();
    virtual event_manager::GameControlEvent *createGameControlEvent() const;
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // Control settings handling
    virtual ios_fc::String getControlName(int controlType, bool alternate);
    virtual bool   changeControl(int controlType, bool alternate, event_manager::GameControlEvent &event);
    virtual void   saveControls();
    virtual void setEnableJoyMouseEmulation(bool enabled);
    // CycledComponent implementation
    virtual void cycle();
private:
    void translateMouseEvent(const SDL_Event &sdl_event,
                             event_manager::GameControlEvent &controlEvent);
    int m_idleDx, m_idleDy;
    int m_mouseX, m_mouseY;
};

#endif // _SDL13_EVENTMANAGER_H_
