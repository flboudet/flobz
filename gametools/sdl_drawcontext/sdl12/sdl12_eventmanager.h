#ifndef _SDL12_EVENTMANAGER_H_
#define _SDL12_EVENTMANAGER_H_

#include <memory>
#include <string>
#include "GameControls.h"
#include "gameloop.h"
#include "PreferencesManager.h"

#include <SDL/SDL.h>

class InputSwitch;

class SDL_GameControlEvent : public event_manager::GameControlEvent
{
public:
    SDL_GameControlEvent();
    virtual GameControlEvent *clone();
    std::auto_ptr<InputSwitch> m_inputSwitch;
};

class SDL12_EventManager : public event_manager::EventManager,
                           public CycledComponent
{
public:
    SDL12_EventManager(PreferencesManager *prefMgr);
    virtual event_manager::GameControlEvent *createGameControlEvent() const;
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // Control settings handling
    virtual ios_fc::String getControlName(int controlType, bool alternate);
    virtual bool   changeControl(int controlType, bool alternate, event_manager::GameControlEvent &event);
    virtual void   saveControls();
    virtual void setEnableJoyMouseEmulation(bool enable);
    // CycledComponent implementation
    virtual void cycle();
private:
    void translateMouseEvent(const SDL_Event &sdl_event,
                             event_manager::GameControlEvent &controlEvent);
    PreferencesManager *m_prefMgr;
    int m_idleDx, m_idleDy;
    int m_mouseX, m_mouseY;
    bool m_disableJoyMouseEmulation;
};

#endif // _SDL12_EVENTMANAGER_H_

