#ifndef _SDL12_EVENTMANAGER_H_
#define _SDL12_EVENTMANAGER_H_

#include <string>
#include "GameControls.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class SDL12_EventManager : public event_manager::EventManager
{
public:

    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
};

#endif // _SDL12_EVENTMANAGER_H_

