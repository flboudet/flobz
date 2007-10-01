#ifndef _GAME_CONT_H
#define _GAME_CONT_H

#include "glSDL.h"
#include "InputManager.h"

/**
 * This class represents an SDL event transcripted into a game event.
 * It manages the mapping between SDL events and game events using
 * the user's preferences
 */
typedef struct GameControlEvent {
	GameControlEvent() : caught(false) {}
    enum {
        kGameNone,
        kPauseGame,
        kPlayer1Left,
        kPlayer1Right,
        kPlayer1TurnLeft,
        kPlayer1TurnRight,
        kPlayer1Down,
        kPlayer2Left,
        kPlayer2Right,
        kPlayer2TurnLeft,
        kPlayer2TurnRight,
        kPlayer2Down,
        kGameLastKey
    } gameEvent;
    enum {
        kCursorNone,
        kUp,
        kDown,
        kLeft,
        kRight,
        kStart,
        kBack,
        kQuit,
        kCursorLastKey
    } cursorEvent;
    bool isUp;
    SDL_Event sdl_event;
	bool caught;
	void setCaught() { caught = true; }
} GameControlEvent;


enum {
    kPlayer1LeftControl             = 0,
    kPlayer1RightControl            = 1,
    kPlayer1DownControl             = 2,
    kPlayer1ClockwiseControl        = 3,
    kPlayer1CounterclockwiseControl = 4,
    kPlayer2LeftControl             = 5,
    kPlayer2RightControl            = 6,
    kPlayer2DownControl             = 7,
    kPlayer2ClockwiseControl        = 8,
    kPlayer2CounterclockwiseControl = 9
};


void initGameControls();
void getControlEvent(SDL_Event e, GameControlEvent *result);

void getKeyName(int control, bool alternate, char *keyName);
bool tryChangeControl(int control, bool alternate, SDL_Event e, GameControlEvent *result);

void saveControls();
void loadControls();

#endif

