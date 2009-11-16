#include "sdl13_eventmanager.h"
#include "common/SDL_InputSwitch.h"

using namespace event_manager;

#define NB_CONTROLS 10
static InputSwitch *keyControls[NB_CONTROLS] =
{
    new KeyInputSwitch(SDLK_s,true),
    new KeyInputSwitch(SDLK_f,true),
    new KeyInputSwitch(SDLK_d,true),
    new KeyInputSwitch(SDLK_UNKNOWN,true),
    new KeyInputSwitch(SDLK_e,true),
    new KeyInputSwitch(SDLK_LEFT,true),
    new KeyInputSwitch(SDLK_RIGHT,true),
    new KeyInputSwitch(SDLK_DOWN,true),
    new KeyInputSwitch(SDLK_UNKNOWN,true),
    new KeyInputSwitch(SDLK_UP,true)
};

static InputSwitch *keyAlternateControls[NB_CONTROLS] =
{
    new JoystickAxisSwitch(0, 0, false, false),
    new JoystickAxisSwitch(0, 0, true, false),
    new JoystickAxisSwitch(0, 1, true, false),
    new JoystickSwitch(0, 1, false),
    new JoystickSwitch(0, 0, false),
    new JoystickAxisSwitch(1, 0, false, false),
    new JoystickAxisSwitch(1, 0, true, false),
    new JoystickAxisSwitch(1, 1, true, false),
    new JoystickSwitch(1, 1, false),
    new JoystickSwitch(1, 0, false)
};

static void getControlEvent(SDL_Event e, InputSwitch *input, GameControlEvent *result)
{
    result->gameEvent     = kGameNone;
    result->cursorEvent   = kCursorNone;
    result->keyboardEvent = kKeyboardNone;
    result->isUp   = true;
    result->caught = false;
    result->isJoystick = false;
    result->unicodeKeySym = 0;

    switch (e.type) {
        case SDL_QUIT:
            result->cursorEvent = kQuit;
            break;
        case SDL_KEYDOWN:
            result->keyboardEvent = kKeyboardDown;
            result->unicodeKeySym = e.key.keysym.unicode;
            break;
        case SDL_KEYUP:
            result->keyboardEvent = kKeyboardUp;
            result->unicodeKeySym = e.key.keysym.unicode;
            break;
        default:
            break;
    }
    // Game event handling
    if (input == NULL)
        return;

    result->isJoystick = input->isJoystick();

    if (input->isQuit())
        result->cursorEvent = kQuit;

    if (input->isValidate())
        result->cursorEvent = kStart;

    if (input->isCancel())
        result->cursorEvent = kBack;

    if (input->isArrowDown())
        result->cursorEvent = kDown;

    if (input->isArrowUp())
        result->cursorEvent = kUp;

    if (input->isArrowLeft())
        result->cursorEvent = kLeft;

    if (input->isArrowRight())
        result->cursorEvent = kRight;

    if (input->isPause())
        result->gameEvent = kPauseGame;

    result->isUp = input->isUp();

    if ((*input == *keyControls[kPlayer1LeftControl]) || (*input == *keyAlternateControls[kPlayer1LeftControl]))
        result->gameEvent = kPlayer1Left;
    if ((*input == *keyControls[kPlayer1RightControl]) || (*input == *keyAlternateControls[kPlayer1RightControl]))
        result->gameEvent = kPlayer1Right;
    if ((*input == *keyControls[kPlayer1ClockwiseControl]) || (*input == *keyAlternateControls[kPlayer1ClockwiseControl]))
        result->gameEvent = kPlayer1TurnRight;
    if ((*input == *keyControls[kPlayer1CounterclockwiseControl]) || (*input == *keyAlternateControls[kPlayer1CounterclockwiseControl]))
        result->gameEvent = kPlayer1TurnLeft;
    if ((*input == *keyControls[kPlayer1DownControl]) || (*input == *keyAlternateControls[kPlayer1DownControl]))
        result->gameEvent = kPlayer1Down;

    if ((*input == *keyControls[kPlayer2LeftControl]) || (*input == *keyAlternateControls[kPlayer2LeftControl]))
        result->gameEvent = kPlayer2Left;
    if ((*input == *keyControls[kPlayer2RightControl]) || (*input == *keyAlternateControls[kPlayer2RightControl]))
        result->gameEvent = kPlayer2Right;
    if ((*input == *keyControls[kPlayer2ClockwiseControl]) || (*input == *keyAlternateControls[kPlayer2ClockwiseControl]))
        result->gameEvent = kPlayer2TurnRight;
    if ((*input == *keyControls[kPlayer2CounterclockwiseControl]) || (*input == *keyAlternateControls[kPlayer2CounterclockwiseControl]))
        result->gameEvent = kPlayer2TurnLeft;
    if ((*input == *keyControls[kPlayer2DownControl]) || (*input == *keyAlternateControls[kPlayer2DownControl]))
        result->gameEvent = kPlayer2Down;
}

static void getControlEvent(SDL_Event e, GameControlEvent *result)
{
    InputSwitch *input  = switchForEvent(&e);
    getControlEvent(e, input, result);

    if (input)
        delete input;
}

ios_fc::String SDL13_EventManager::getControlName(int controlType, bool alternate)
{
    ios_fc::String controlName("           ");
    if (alternate) {
        if (keyAlternateControls[controlType])
            controlName = keyAlternateControls[controlType]->name();
    }
    else {
        if (keyControls[controlType])
            controlName = keyControls[controlType]->name();
    }
    return controlName;
}

bool SDL13_EventManager::changeControl(int controlType, bool alternate, GameControlEvent &event)
{
    return true;
}

void SDL13_EventManager::saveControls()
{
}

struct CursorEventArg {
    CursorEventArg(int x, int y) : x(x), y(y) {}
    int x, y;
};

SDL13_EventManager::SDL13_EventManager()
    : CycledComponent(0.01), m_idleDx(0), m_idleDy(0),
      m_mouseX(0), m_mouseY(0)
{
	SDL_EnableUNICODE(1);
}

bool SDL13_EventManager::pollEvent(GameControlEvent &controlEvent)
{
    SDL_Event e;
    bool result = SDL_PollEvent(&e);
    // Try to translate the SDL event into a game event
    // Try to translate the SDL event into a cursor event
    // Try to translate the SDL event into a keyboard event
    getControlEvent(e, &controlEvent);
    // Try to translate the SDL event into a mouse event
    translateMouseEvent(e, controlEvent);
    return result;
}

void SDL13_EventManager::pushMouseEvent(int x, int y,
                                        CursorEventType type)
{
    // Push an SDL user event
    SDL_Event mouseEvent;
    mouseEvent.type = SDL_USEREVENT;
    mouseEvent.user.code = type;
    mouseEvent.user.data1 = new CursorEventArg(x, y);
    SDL_PushEvent(&mouseEvent);
}

void SDL13_EventManager::translateMouseEvent(const SDL_Event &sdl_event,
                                             GameControlEvent &controlEvent)
{
    switch (sdl_event.type) {
    case SDL_MOUSEMOTION:
        m_mouseX = sdl_event.motion.x;
        m_mouseY = sdl_event.motion.y;
        controlEvent.isUp = false;
        controlEvent.cursorEvent = kGameMouseMoved;
        controlEvent.x = m_mouseX;
        controlEvent.y = m_mouseY;
        break;
    case SDL_JOYAXISMOTION:
        if (sdl_event.jaxis.axis == 2) {
            m_idleDy = sdl_event.jaxis.value / 5000;
        }
        else if (sdl_event.jaxis.axis == 3) {
            m_idleDx = sdl_event.jaxis.value / 5000;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (sdl_event.button.button == SDL_BUTTON_LEFT) {
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseDown;
            controlEvent.x = sdl_event.button.x;
            controlEvent.y = sdl_event.button.y;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (sdl_event.button.button == SDL_BUTTON_LEFT) {
            controlEvent.isUp = true;
            controlEvent.cursorEvent = kGameMouseUp;
            controlEvent.x = sdl_event.button.x;
            controlEvent.y = sdl_event.button.y;
        }
        break;
    case SDL_USEREVENT: {
        switch (sdl_event.user.code) {
        case kGameMouseMoved:
        case kGameMouseDown:
        case kGameMouseUp: {
            CursorEventArg *arg = (CursorEventArg *)sdl_event.user.data1;
            controlEvent.x = arg->x;
            controlEvent.y = arg->y;
            delete arg;
        } break;
        default:
            break;
        }
        switch (sdl_event.user.code) {
        case kGameMouseMoved:
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseMoved;
            break;
        case kGameMouseDown:
            controlEvent.isUp = false;
            controlEvent.cursorEvent = kGameMouseDown;
            break;
        case kGameMouseUp:
            controlEvent.isUp = true;
            controlEvent.cursorEvent = kGameMouseUp;
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
}

void SDL13_EventManager::cycle()
{
    if ((m_idleDx * m_idleDx) + (m_idleDy * m_idleDy) < 1)
        return;
    m_mouseX += m_idleDx;
    m_mouseY += m_idleDy;
    pushMouseEvent(m_mouseX, m_mouseY, kGameMouseMoved);
}
