#include "InputManager.h"

typedef struct SdlKeyName {
  int key;
  char name[50];
} SdlKeyName;

static const SdlKeyName sdlKeyDictionnary[] = {
  { SDLK_UNKNOWN,   "  "},
  { SDLK_BACKSPACE, "Backspace"  },
  { SDLK_TAB,       "Tab"        },
  { SDLK_CLEAR,     "Clear"      },
  { SDLK_RETURN,    "Return"     },
  { SDLK_PAUSE,     "Pause"      },
  { SDLK_ESCAPE,    "Escape"     },
  { SDLK_SPACE,     "Space"      },
  { SDLK_DELETE,    "Delete"     },
  { SDLK_KP0,       "KP 0"       },
  { SDLK_KP1,       "KP 1"       },
  { SDLK_KP2,       "KP 2"       },
  { SDLK_KP3,       "KP 3"       },
  { SDLK_KP4,       "KP 4"       },
  { SDLK_KP5,       "KP 5"       },
  { SDLK_KP6,       "KP 6"       },
  { SDLK_KP7,       "KP 7"       },
  { SDLK_KP8,       "KP 8"       },
  { SDLK_KP9,       "KP 9"       },
  { SDLK_UP,        "Up Arrow"   },
  { SDLK_DOWN,      "Down Arrow" },
  { SDLK_LEFT,      "Left Arrow" },
  { SDLK_RIGHT,     "Right Arrow"},
  { SDLK_INSERT,    "Insert"     },
  { SDLK_HOME,      "Home"       },
  { SDLK_END,       "End"        },
  { SDLK_PAGEUP,     "Page Up"   },
  { SDLK_PAGEDOWN,   "Page Down" },
  { SDLK_F1,         "F1"        },
  { SDLK_F2,         "F2"        },
  { SDLK_F3,         "F3"        },
  { SDLK_F4,         "F4"        },
  { SDLK_F5,         "F5"        },
  { SDLK_F6,         "F6"        },
  { SDLK_F7,         "F7"        },
  { SDLK_F8,         "F8"        },
  { SDLK_F9,         "F9"        },
  { SDLK_F10,        "F10"       },
  { SDLK_F11,        "F11"       },
  { SDLK_F12,        "F12"       },
  { SDLK_F13,        "F13"       },
  { SDLK_F14,        "F14"       },
  { SDLK_F15,        "F15"       },
  { SDLK_NUMLOCK,    "Num Lock"  },
  { SDLK_CAPSLOCK,   "Caps Lock" },
  { SDLK_SCROLLOCK,  "Scroll Lock"},
  { SDLK_RSHIFT,     "Right Shift"},
  { SDLK_LSHIFT,     "Left Shift" },
  { SDLK_RCTRL,     "Right Ctrl" },
  { SDLK_LCTRL,     "Left Ctrl" },
  { SDLK_RALT,     "Right Alt" },
  { SDLK_LALT,     "Left Alt" },
  { SDLK_RMETA,     "Right Meta" },
  { SDLK_LMETA,     "Left Meta" },
  { SDLK_RSUPER,     "Right Windows" },
  { SDLK_LSUPER,     "Left Windows" },
  { SDLK_MODE,     "Mode Shift" },
  { SDLK_HELP,     "Help" },
  { SDLK_PRINT,     "Print Screen" },
  { SDLK_SYSREQ,     "Sys Rq" },
  { SDLK_BREAK,     "Break" },
  { SDLK_MENU,     "Menu" },
  { SDLK_POWER,     "Power" },
  { SDLK_EURO,     "Euro" }
};

static const int sdlKeyDictionnarySize = sizeof(sdlKeyDictionnary) / sizeof(SdlKeyName);

static SDL_Joystick *joystick[16];
static int numJoysticks;
static int axisSave[16][16];

/* KEY Input */

KeyInputSwitch::KeyInputSwitch(int keysym, bool isup) : InputSwitch(isup), keysym(keysym) {
  keyName[0] = 0;
}

const char *KeyInputSwitch::name() const {

  if (keyName[0] == 0) {
    for (int i = 0 ; i < sdlKeyDictionnarySize ; i++) {
      if (sdlKeyDictionnary[i].key == keysym) {
        strcpy(keyName, sdlKeyDictionnary[i].name);
        break;
      }
    }
    if (keyName[0] == 0) {
      keyName[0] = (char)keysym;
      keyName[1] = 0;
    }
  }
  return keyName;
}

int KeyInputSwitch::id() const {
  return (1000 - keysym);
}

bool KeyInputSwitch::isArrowUp()   const {
  return keysym == SDLK_UP;
}
bool KeyInputSwitch::isArrowDown() const {
  return keysym == SDLK_DOWN;
}
bool KeyInputSwitch::isValidate()  const {
  return keysym == SDLK_RETURN;
}
bool KeyInputSwitch::isCancel()    const {
  return keysym == SDLK_ESCAPE;
}
bool KeyInputSwitch::isPause()    const {
  return keysym == SDLK_p;
}


/* JOY BUTTON */

JoystickSwitch::JoystickSwitch(int which, int button, bool isup)
: InputSwitch(isup),which(which),button(button)
{
  keyName[0] = 0;
}

const char *JoystickSwitch::name() const {

  if (keyName[0] == 0)
    sprintf(keyName, "JOY%d - Button %d", which, button);
  return keyName;
}

int JoystickSwitch::id() const {
  return 2000 + which * 50 + button;
}

bool JoystickSwitch::isValidate()  const {
  return button == 0;
}
bool JoystickSwitch::isCancel()    const {
  return button == 2;
}

/* JOY AXIS SWITCH */

JoystickAxisSwitch::JoystickAxisSwitch(int which, int axis, bool maximum, bool isup)
  : InputSwitch(isup),which(which),axis(axis),maximum(maximum)
{
  keyName[0] = 0;
}

const char *JoystickAxisSwitch::name() const {

  if (keyName[0] == 0)
    sprintf(keyName, "JOY%d - Axis %d - %s", which, axis, (maximum?"Max":"Min"));
  return keyName;
}

int JoystickAxisSwitch::id() const {
  return 3000 + which * 50 + axis + (maximum?1:0);
}

bool JoystickAxisSwitch::isArrowUp()   const {
  return (axis == 1) && (!maximum);
}
bool JoystickAxisSwitch::isArrowDown() const {
  return (axis == 1) && (maximum);
}
/** EVENT HANDLERS */

InputSwitch *waitInputSwitch()
{
  SDL_Event e;
  if (!SDL_WaitEvent(&e))
    return NULL;

  return switchForEvent(&e);
}

InputSwitch *checkInputSwitch()
{
  SDL_Event e;
  if (!SDL_PollEvent(&e))
    return NULL;

  return switchForEvent(&e);
}



InputSwitch *switchForEvent(SDL_Event *e)
{
  int prevaxis;
  switch (e->type)
  {
    case SDL_JOYBUTTONDOWN:
      return new JoystickSwitch(e->jbutton.which, e->jbutton.button, false);

    case SDL_JOYBUTTONUP:
      return new JoystickSwitch(e->jbutton.which, e->jbutton.button, true);
      
    case SDL_JOYAXISMOTION:
      prevaxis = axisSave[e->jaxis.which][e->jaxis.axis];
      axisSave[e->jaxis.which][e->jaxis.axis] = e->jaxis.value;

      if ((e->jaxis.value > JOYSTICK_THRESHOLD) && (prevaxis <= JOYSTICK_THRESHOLD))
          return new JoystickAxisSwitch(e->jaxis.which, e->jaxis.axis, true, false);
      if ((e->jaxis.value <= JOYSTICK_THRESHOLD) && (prevaxis > JOYSTICK_THRESHOLD))
          return new JoystickAxisSwitch(e->jaxis.which, e->jaxis.axis, true, true);
      if ((e->jaxis.value < -JOYSTICK_THRESHOLD) && (prevaxis >= -JOYSTICK_THRESHOLD))
          return new JoystickAxisSwitch(e->jaxis.which, e->jaxis.axis, false, false);
      if ((e->jaxis.value >= -JOYSTICK_THRESHOLD) && (prevaxis < -JOYSTICK_THRESHOLD))
          return new JoystickAxisSwitch(e->jaxis.which, e->jaxis.axis, false, true);

      return NULL;
      
    case SDL_KEYDOWN:
      return new KeyInputSwitch(e->key.keysym.sym, false);
    case SDL_KEYUP:
      return new KeyInputSwitch(e->key.keysym.sym, true);
    default:
      return NULL;
  }
}


void initControllers()
{
  numJoysticks = SDL_NumJoysticks();
  for( int i=0 ; i < numJoysticks ; i++ ) 
  {
    joystick[numJoysticks - i - 1] = SDL_JoystickOpen(i);
  }
  SDL_JoystickEventState(SDL_ENABLE);

  for (int i = 0 ; i < 16 ; i++)
    for (int j = 0 ; j < 16 ; j++)
      axisSave[i][j] = 0;
}

void closeControllers()
{
  for( int i=0 ; i < numJoysticks ; i++ ) 
  {
    SDL_JoystickClose(joystick[i]);
  }
}
