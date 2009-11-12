#ifndef _OPENGL_EVENTMANAGER_H_
#define _OPENGL_EVENTMANAGER_H_

#include <string>
#include "GameControls.h"
#include "gameloop.h"

class OpenGL_EventManager : public event_manager::EventManager
{
public:
    OpenGL_EventManager();
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // CycledComponent implementation
    virtual void cycle();
    virtual ios_fc::String getControlName(int controlType, bool alternate);
    virtual bool changeControl(int controlType, bool alternate, event_manager::GameControlEvent &event);
    virtual void saveControls();
private:
    //void translateMouseEvent(const SDL_Event &sdl_event, event_manager::GameControlEvent &controlEvent);
    int m_idleDx, m_idleDy;
    int m_mouseX, m_mouseY;
};

#endif // _OpenGL_EVENTMANAGER_H_


