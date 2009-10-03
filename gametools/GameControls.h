#ifndef _GAME_CONT_H
#define _GAME_CONT_H

#include <stdint.h>

#include "InputManager.h"

namespace event_manager
{

/**
 * Corresponds to an event interpreted during game phase
 */
enum GameEventType {
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
};

/**
 * Corresponds to an event during GUI navigation
 */
enum CursorEventType {
    kCursorNone,
    kUp,
    kDown,
    kLeft,
    kRight,
    kStart,
    kBack,
    kQuit,
    kGameMouseMoved,
    kGameMouseDown,
    kGameMouseUp,
    kCursorLastKey
};

/**
 * Corresponds to an event during keyboard input
 */
enum KeyboardEventType {
    kKeyboardNone,
    kKeyboardDown,
    kKeyboardUp,
    kKeyboardLastKey
};

/**
 * Representation of any kind of user input event happening
 * in the game. Depending on the scope of the object monitoring
 * the event, it can be either/both:
 *  - a game event
 *  - a cursor event (navigation in the GUI, including mouse handling)
 *  - a keyboard event (for keyboard input)
 * GameControlEvent is independant of the underlying
 * event handling backend.
 */
typedef struct GameControlEvent {
	GameControlEvent() : caught(false) {}
    GameEventType     gameEvent;
    CursorEventType   cursorEvent;
    KeyboardEventType keyboardEvent;
    bool isUp;
    bool isJoystick;
	bool caught;
    int x, y;
    uint16_t unicodeKeySym;
	void setCaught() { caught = true; }
} GameControlEvent;

/**
 * Abstract interface to an event handling backend
 */
class EventManager
{
public:
    virtual ~EventManager() {}
    virtual bool pollEvent(GameControlEvent &controlEvent) = 0;
    virtual void pushMouseEvent(int x, int y, CursorEventType type) = 0;
};

// TODO: move all that crap elsewhere

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

} // namespace

#endif

