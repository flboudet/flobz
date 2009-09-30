#ifndef _SDL12_EVENTMANAGER_H_
#define _SDL12_EVENTMANAGER_H_

#include <string>
#include "GameControls.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class SDL12_EventManager : public EventManager
{
public:
    virtual bool pollEvent(GameControlEvent &controlEvent);
};

#endif // _SDL12_EVENTMANAGER_H_

