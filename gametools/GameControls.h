#ifndef _GAME_CONT_H
#define _GAME_CONT_H

#include <stdint.h>

#include "InputManager.h"

#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

/**
 * This class represents an SDL event transcripted into a game event.
 * It manages the mapping between SDL events and game events using
 * the user's preferences
 */
typedef struct GameControlEvent {
	GameControlEvent() : caught(false) {}
    /**
     * Corresponds to an event interpreted during game phase
     */
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
    /**
     * Corresponds to an event during GUI navigation
     */
    enum {
        kCursorNone,
        kUp,
        kDown,
        kLeft,
        kRight,
        kStart,
        kBack,
        kQuit,
        kGameMouseMoved,
        kGameMouseClicked,
        kCursorLastKey
    } cursorEvent;
    /**
     * Corresponds to an event during keyboard input
     */
    enum {
        kKeyboardNone,
        kKeyboardDown,
        kKeyboardUp
    } keyboardEvent;
    bool isUp;
    bool isJoystick;
	bool caught;
    int x, y;
    uint16_t unicodeKeySym;
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

class EventManager
{
public:
    virtual ~EventManager() {}
    virtual bool pollEvent(GameControlEvent &controlEvent) = 0;
};

#endif

