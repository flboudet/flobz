#include "GameControls.h"
#include "preferences.h"

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

void getKeyName(int gameEvent, char *keyName)
{
  strcpy(keyName,"           ");
  if (keyControls[gameEvent])
    strcpy(keyName,keyControls[gameEvent]->name());
}

void getControlEvent(SDL_Event e, InputSwitch *input, GameControlEvent *result)
{
  result->gameEvent   = GameControlEvent::kGameNone;
  result->cursorEvent = GameControlEvent::kCursorNone;
  result->isUp = true;
    
  if (e.type == SDL_QUIT)
    result->cursorEvent = GameControlEvent::kQuit;
  
  if (input == NULL)
    return;

  if (input->isValidate() && input->isDown())
    result->cursorEvent = GameControlEvent::kStart;

  if (input->isCancel() && input->isDown())
    result->cursorEvent = GameControlEvent::kBack;

  if (input->isArrowDown() && input->isDown())
    result->cursorEvent = GameControlEvent::kDown;

  if (input->isArrowUp() && input->isDown())
    result->cursorEvent = GameControlEvent::kUp;

  if (input->isPause() && input->isDown())
    result->gameEvent = GameControlEvent::kPauseGame;

  result->isUp = input->isUp();

  if (*input == *keyControls[kPlayer1LeftControl])
    result->gameEvent = GameControlEvent::kPlayer1Left;
  if (*input == *keyControls[kPlayer1RightControl])
    result->gameEvent = GameControlEvent::kPlayer1Right;
  if (*input == *keyControls[kPlayer1ClockwiseControl])
    result->gameEvent = GameControlEvent::kPlayer1TurnRight;
  if (*input == *keyControls[kPlayer1CounterclockwiseControl])
    result->gameEvent = GameControlEvent::kPlayer1TurnLeft;
  if (*input == *keyControls[kPlayer1DownControl])
    result->gameEvent = GameControlEvent::kPlayer1Down;

  if (*input == *keyControls[kPlayer2LeftControl])
    result->gameEvent = GameControlEvent::kPlayer2Left;
  if (*input == *keyControls[kPlayer2RightControl])
    result->gameEvent = GameControlEvent::kPlayer2Right;
  if (*input == *keyControls[kPlayer2ClockwiseControl])
    result->gameEvent = GameControlEvent::kPlayer2TurnRight;
  if (*input == *keyControls[kPlayer2CounterclockwiseControl])
    result->gameEvent = GameControlEvent::kPlayer2TurnLeft;
  if (*input == *keyControls[kPlayer2DownControl])
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

bool tryChangeControl(int control, SDL_Event e, GameControlEvent *result)
{
  InputSwitch *input  = switchForEvent(&e);
  if (input == NULL)
    return false;

  getControlEvent(e,input,result);
  
/*
  if (result->cursorEvent == GameControlEvent::kBack)
    goto ret_false;

  if (result->cursorEvent == GameControlEvent::kStart)
    goto ret_false;
*/

  if (result->cursorEvent == GameControlEvent::kQuit)
    goto ret_false;

  if (input->isUp())
    goto ret_false;

  if (keyControls[control] != NULL)
    delete keyControls[control];

  keyControls[control] = input;
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
}
