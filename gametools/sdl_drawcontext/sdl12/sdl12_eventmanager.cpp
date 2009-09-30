#include "sdl12_eventmanager.h"

bool SDL12_EventManager::pollEvent(GameControlEvent &controlEvent)
{
    SDL_Event e;
    bool result = SDL_PollEvent(&e);
    getControlEvent(e, &controlEvent);
    return result;
}


