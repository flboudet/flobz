#include "GameControls.h"
#include "preferences.h"
#include "GameCursor.h"

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

void getKeyName(int gameEvent, bool alternate, char *keyName)
{
  strcpy(keyName,"           ");
  if (alternate) {
    if (keyAlternateControls[gameEvent])
        strcpy(keyName,keyAlternateControls[gameEvent]->name());
  }
  else {
    if (keyControls[gameEvent])
        strcpy(keyName,keyControls[gameEvent]->name());
  }
}

void getControlEvent(SDL_Event e, InputSwitch *input, GameControlEvent *result)
{
  result->gameEvent     = GameControlEvent::kGameNone;
  result->cursorEvent   = GameControlEvent::kCursorNone;
  result->keyboardEvent = GameControlEvent::kKeyboardNone;
  result->isUp   = true;
  result->caught = false;
  result->isJoystick = false;
  result->unicodeKeySym = 0;

  switch (e.type) {
  case SDL_QUIT:
    result->cursorEvent = GameControlEvent::kQuit;
    break;
  // Mouse handling
  case SDL_USEREVENT: {
    switch (e.user.code) {
    case Cursor::MOVE: {
        result->isUp = false;
        result->cursorEvent = GameControlEvent::kGameMouseMoved;
        Cursor::CursorEventArg *arg = (Cursor::CursorEventArg *)e.user.data1;
        result->x = arg->x;
        result->y = arg->y;
        delete arg;
        } break;
    case Cursor::MOUSE_DOWN: {
        result->isUp = false;
        result->cursorEvent = GameControlEvent::kGameMouseClicked;
        Cursor::CursorEventArg *arg = (Cursor::CursorEventArg *)e.user.data1;
        result->x = arg->x;
        result->y = arg->y;
        delete arg;
        } break;
    case Cursor::MOUSE_UP: {
        result->isUp = true;
        result->cursorEvent = GameControlEvent::kGameMouseClicked;
        Cursor::CursorEventArg *arg = (Cursor::CursorEventArg *)e.user.data1;
        result->x = arg->x;
        result->y = arg->y;
        delete arg;
        } break;
    default:
        break;
    }
  }
  case SDL_KEYDOWN:
      result->keyboardEvent = GameControlEvent::kKeyboardDown;
      result->unicodeKeySym = e.key.keysym.unicode;
      break;
  case SDL_KEYUP:
      result->keyboardEvent = GameControlEvent::kKeyboardUp;
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
    result->cursorEvent = GameControlEvent::kQuit;

  if (input->isValidate())
    result->cursorEvent = GameControlEvent::kStart;

  if (input->isCancel())
    result->cursorEvent = GameControlEvent::kBack;

  if (input->isArrowDown())
    result->cursorEvent = GameControlEvent::kDown;

  if (input->isArrowUp())
    result->cursorEvent = GameControlEvent::kUp;

  if (input->isArrowLeft())
    result->cursorEvent = GameControlEvent::kLeft;

  if (input->isArrowRight())
    result->cursorEvent = GameControlEvent::kRight;

  if (input->isPause())
    result->gameEvent = GameControlEvent::kPauseGame;

  result->isUp = input->isUp();

  if ((*input == *keyControls[kPlayer1LeftControl]) || (*input == *keyAlternateControls[kPlayer1LeftControl]))
    result->gameEvent = GameControlEvent::kPlayer1Left;
  if ((*input == *keyControls[kPlayer1RightControl]) || (*input == *keyAlternateControls[kPlayer1RightControl]))
    result->gameEvent = GameControlEvent::kPlayer1Right;
  if ((*input == *keyControls[kPlayer1ClockwiseControl]) || (*input == *keyAlternateControls[kPlayer1ClockwiseControl]))
    result->gameEvent = GameControlEvent::kPlayer1TurnRight;
  if ((*input == *keyControls[kPlayer1CounterclockwiseControl]) || (*input == *keyAlternateControls[kPlayer1CounterclockwiseControl]))
    result->gameEvent = GameControlEvent::kPlayer1TurnLeft;
  if ((*input == *keyControls[kPlayer1DownControl]) || (*input == *keyAlternateControls[kPlayer1DownControl]))
    result->gameEvent = GameControlEvent::kPlayer1Down;

  if ((*input == *keyControls[kPlayer2LeftControl]) || (*input == *keyAlternateControls[kPlayer2LeftControl]))
    result->gameEvent = GameControlEvent::kPlayer2Left;
  if ((*input == *keyControls[kPlayer2RightControl]) || (*input == *keyAlternateControls[kPlayer2RightControl]))
    result->gameEvent = GameControlEvent::kPlayer2Right;
  if ((*input == *keyControls[kPlayer2ClockwiseControl]) || (*input == *keyAlternateControls[kPlayer2ClockwiseControl]))
    result->gameEvent = GameControlEvent::kPlayer2TurnRight;
  if ((*input == *keyControls[kPlayer2CounterclockwiseControl]) || (*input == *keyAlternateControls[kPlayer2CounterclockwiseControl]))
    result->gameEvent = GameControlEvent::kPlayer2TurnLeft;
  if ((*input == *keyControls[kPlayer2DownControl]) || (*input == *keyAlternateControls[kPlayer2DownControl]))
    result->gameEvent = GameControlEvent::kPlayer2Down;
}

void getControlEvent(SDL_Event e, GameControlEvent *result)
{
  InputSwitch *input  = switchForEvent(&e);
  getControlEvent(e, input, result);

  if (input)
    delete input;
}

void initGameControls()
{
  initControllers();
  loadControls();
}

bool tryChangeControl(int control, bool alternate, SDL_Event e, GameControlEvent *result)
{
  printf("switchForEvent?\n");
  InputSwitch *input = switchForEvent(&e);
  if (input == NULL) {
    printf("input is null\n");
    return false;
  }
  printf("switchForEvent!\n");

  getControlEvent(e,input,result);

  if (result->cursorEvent == GameControlEvent::kQuit)
    goto ret_false;

  if (input->isUp())
    goto ret_false;

  if (alternate == false) {
    if (keyControls[control] != NULL)
        delete keyControls[control];
    keyControls[control] = input;
  }
  else {
    if (keyAlternateControls[control] != NULL)
        delete keyAlternateControls[control];
    keyAlternateControls[control] = input;
  }
  return true;

ret_false:
  delete input;
  return false;
}

void saveControls()
{
    SetIntPreference("v50_P1Left",             keyControls[kPlayer1LeftControl]->id());
    SetIntPreference("v50_P1Right",            keyControls[kPlayer1RightControl]->id());
    SetIntPreference("v50_P1Down",             keyControls[kPlayer1DownControl]->id());
    SetIntPreference("v50_P1Clockwise",        keyControls[kPlayer1ClockwiseControl]->id());
    SetIntPreference("v50_P1Counterclockwise", keyControls[kPlayer1CounterclockwiseControl]->id());

    SetStrPreference("v50_P1LeftS",             keyControls[kPlayer1LeftControl]->name());
    SetStrPreference("v50_P1RightS",            keyControls[kPlayer1RightControl]->name());
    SetStrPreference("v50_P1DownS",             keyControls[kPlayer1DownControl]->name());
    SetStrPreference("v50_P1ClockwiseS",        keyControls[kPlayer1ClockwiseControl]->name());
    SetStrPreference("v50_P1CounterclockwiseS", keyControls[kPlayer1CounterclockwiseControl]->name());

    SetIntPreference("v50_AltP1Left",             keyAlternateControls[kPlayer1LeftControl]->id());
    SetIntPreference("v50_AltP1Right",            keyAlternateControls[kPlayer1RightControl]->id());
    SetIntPreference("v50_AltP1Down",             keyAlternateControls[kPlayer1DownControl]->id());
    SetIntPreference("v50_AltP1Clockwise",        keyAlternateControls[kPlayer1ClockwiseControl]->id());
    SetIntPreference("v50_AltP1Counterclockwise", keyAlternateControls[kPlayer1CounterclockwiseControl]->id());

    SetStrPreference("v50_AltP1LeftS",             keyAlternateControls[kPlayer1LeftControl]->name());
    SetStrPreference("v50_AltP1RightS",            keyAlternateControls[kPlayer1RightControl]->name());
    SetStrPreference("v50_AltP1DownS",             keyAlternateControls[kPlayer1DownControl]->name());
    SetStrPreference("v50_AltP1ClockwiseS",        keyAlternateControls[kPlayer1ClockwiseControl]->name());
    SetStrPreference("v50_AltP1CounterclockwiseS", keyAlternateControls[kPlayer1CounterclockwiseControl]->name());

    SetIntPreference("v50_P2Left",             keyControls[kPlayer2LeftControl]->id());
    SetIntPreference("v50_P2Right",            keyControls[kPlayer2RightControl]->id());
    SetIntPreference("v50_P2Down",             keyControls[kPlayer2DownControl]->id());
    SetIntPreference("v50_P2Clockwise",        keyControls[kPlayer2ClockwiseControl]->id());
    SetIntPreference("v50_P2Counterclockwise", keyControls[kPlayer2CounterclockwiseControl]->id());

    SetStrPreference("v50_P2LeftS",             keyControls[kPlayer2LeftControl]->name());
    SetStrPreference("v50_P2RightS",            keyControls[kPlayer2RightControl]->name());
    SetStrPreference("v50_P2DownS",             keyControls[kPlayer2DownControl]->name());
    SetStrPreference("v50_P2ClockwiseS",        keyControls[kPlayer2ClockwiseControl]->name());
    SetStrPreference("v50_P2CounterclockwiseS", keyControls[kPlayer2CounterclockwiseControl]->name());

    SetIntPreference("v50_AltP2Left",             keyAlternateControls[kPlayer2LeftControl]->id());
    SetIntPreference("v50_AltP2Right",            keyAlternateControls[kPlayer2RightControl]->id());
    SetIntPreference("v50_AltP2Down",             keyAlternateControls[kPlayer2DownControl]->id());
    SetIntPreference("v50_AltP2Clockwise",        keyAlternateControls[kPlayer2ClockwiseControl]->id());
    SetIntPreference("v50_AltP2Counterclockwise", keyAlternateControls[kPlayer2CounterclockwiseControl]->id());

    SetStrPreference("v50_AltP2LeftS",             keyAlternateControls[kPlayer2LeftControl]->name());
    SetStrPreference("v50_AltP2RightS",            keyAlternateControls[kPlayer2RightControl]->name());
    SetStrPreference("v50_AltP2DownS",             keyAlternateControls[kPlayer2DownControl]->name());
    SetStrPreference("v50_AltP2ClockwiseS",        keyAlternateControls[kPlayer2ClockwiseControl]->name());
    SetStrPreference("v50_AltP2CounterclockwiseS", keyAlternateControls[kPlayer2CounterclockwiseControl]->name());
}

void loadControls()
{
   char name[256];
   int id;

   GetStrPreference("v50_P1LeftS", name, keyControls[kPlayer1LeftControl]->name());
   id = GetIntPreference("v50_P1Left",  keyControls[kPlayer1LeftControl]->id());
   keyControls[kPlayer1LeftControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P1RightS", name, keyControls[kPlayer1RightControl]->name());
   id = GetIntPreference("v50_P1Right",  keyControls[kPlayer1RightControl]->id());
   keyControls[kPlayer1RightControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P1DownS", name, keyControls[kPlayer1DownControl]->name());
   id = GetIntPreference("v50_P1Down",  keyControls[kPlayer1DownControl]->id());
   keyControls[kPlayer1DownControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P1ClockwiseS", name, keyControls[kPlayer1ClockwiseControl]->name());
   id = GetIntPreference("v50_P1Clockwise",   keyControls[kPlayer1ClockwiseControl]->id());
   keyControls[kPlayer1ClockwiseControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P1CounterclockwiseS", name, keyControls[kPlayer1CounterclockwiseControl]->name());
   id = GetIntPreference("v50_P1Counterclockwise",  keyControls[kPlayer1CounterclockwiseControl]->id());
   keyControls[kPlayer1CounterclockwiseControl] = new InputFromIDAndName(id, name);

   GetStrPreference("v50_AltP1LeftS", name, keyAlternateControls[kPlayer1LeftControl]->name());
   id = GetIntPreference("v50_AltP1Left",  keyAlternateControls[kPlayer1LeftControl]->id());
   keyAlternateControls[kPlayer1LeftControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP1RightS", name, keyAlternateControls[kPlayer1RightControl]->name());
   id = GetIntPreference("v50_AltP1Right",  keyAlternateControls[kPlayer1RightControl]->id());
   keyAlternateControls[kPlayer1RightControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP1DownS", name, keyAlternateControls[kPlayer1DownControl]->name());
   id = GetIntPreference("v50_AltP1Down",  keyAlternateControls[kPlayer1DownControl]->id());
   keyAlternateControls[kPlayer1DownControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP1ClockwiseS", name, keyAlternateControls[kPlayer1ClockwiseControl]->name());
   id = GetIntPreference("v50_AltP1Clockwise",   keyAlternateControls[kPlayer1ClockwiseControl]->id());
   keyAlternateControls[kPlayer1ClockwiseControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP1CounterclockwiseS", name, keyAlternateControls[kPlayer1CounterclockwiseControl]->name());
   id = GetIntPreference("v50_AltP1Counterclockwise",  keyAlternateControls[kPlayer1CounterclockwiseControl]->id());
   keyAlternateControls[kPlayer1CounterclockwiseControl] = new InputFromIDAndName(id, name);

   GetStrPreference("v50_P2LeftS", name, keyControls[kPlayer2LeftControl]->name());
   id = GetIntPreference("v50_P2Left",  keyControls[kPlayer2LeftControl]->id());
   keyControls[kPlayer2LeftControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P2RightS", name, keyControls[kPlayer2RightControl]->name());
   id = GetIntPreference("v50_P2Right",  keyControls[kPlayer2RightControl]->id());
   keyControls[kPlayer2RightControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P2DownS", name, keyControls[kPlayer2DownControl]->name());
   id = GetIntPreference("v50_P2Down",  keyControls[kPlayer2DownControl]->id());
   keyControls[kPlayer2DownControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P2ClockwiseS", name, keyControls[kPlayer2ClockwiseControl]->name());
   id = GetIntPreference("v50_P2Clockwise",   keyControls[kPlayer2ClockwiseControl]->id());
   keyControls[kPlayer2ClockwiseControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_P2CounterclockwiseS", name, keyControls[kPlayer2CounterclockwiseControl]->name());
   id = GetIntPreference("v50_P2Counterclockwise",  keyControls[kPlayer2CounterclockwiseControl]->id());
   keyControls[kPlayer2CounterclockwiseControl] = new InputFromIDAndName(id, name);

   GetStrPreference("v50_AltP2LeftS", name, keyAlternateControls[kPlayer2LeftControl]->name());
   id = GetIntPreference("v50_AltP2Left",  keyAlternateControls[kPlayer2LeftControl]->id());
   keyAlternateControls[kPlayer2LeftControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP2RightS", name, keyAlternateControls[kPlayer2RightControl]->name());
   id = GetIntPreference("v50_AltP2Right",  keyAlternateControls[kPlayer2RightControl]->id());
   keyAlternateControls[kPlayer2RightControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP2DownS", name, keyAlternateControls[kPlayer2DownControl]->name());
   id = GetIntPreference("v50_AltP2Down",  keyAlternateControls[kPlayer2DownControl]->id());
   keyAlternateControls[kPlayer2DownControl]  = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP2ClockwiseS", name, keyAlternateControls[kPlayer2ClockwiseControl]->name());
   id = GetIntPreference("v50_AltP2Clockwise",   keyAlternateControls[kPlayer2ClockwiseControl]->id());
   keyAlternateControls[kPlayer2ClockwiseControl] = new InputFromIDAndName(id, name);
   GetStrPreference("v50_AltP2CounterclockwiseS", name, keyAlternateControls[kPlayer2CounterclockwiseControl]->name());
   id = GetIntPreference("v50_AltP2Counterclockwise",  keyAlternateControls[kPlayer2CounterclockwiseControl]->id());
   keyAlternateControls[kPlayer2CounterclockwiseControl] = new InputFromIDAndName(id, name);
}
