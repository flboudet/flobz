#include "sdl12_eventmanager.h"

using namespace event_manager;

bool SDL12_EventManager::pollEvent(GameControlEvent &controlEvent)
{
    SDL_Event e;
    bool result = SDL_PollEvent(&e);
    getControlEvent(e, &controlEvent);
    return result;
}

void SDL12_EventManager::pushMouseEvent(int x, int y,
                                        CursorEventType type)
{
}

