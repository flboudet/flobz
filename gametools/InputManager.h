#ifndef _INPUT_SW_MAN_H
#define _INPUT_SW_MAN_H

#include "glSDL.h"
#include <string.h>

const int JOYSTICK_THRESHOLD = 25000;

class InputSwitch
{
  public:

    InputSwitch(bool isup = false) : isup(isup) {}
    virtual ~InputSwitch() {}
    virtual const char *name() const = 0;
    virtual int id() const = 0;

    inline bool operator==(const InputSwitch &other) const {
      return id() == other.id();
    }

    inline bool isUp()   const { return isup; }
    inline bool isDown() const { return !isup; }

    /* Meta-Informations about the switch */
    virtual bool isArrowUp()   const { return false; }
    virtual bool isArrowDown() const { return false; }
    virtual bool isValidate()  const { return false; }
    virtual bool isCancel()    const { return false; }
    virtual bool isPause()     const { return false; }

  private:
    bool isup;
};

void initControllers();
void closeControllers();
  
InputSwitch *switchForEvent(SDL_Event *e);
InputSwitch *waitInputSwitch();
InputSwitch *checkInputSwitch();

class KeyInputSwitch : public InputSwitch
{
  public:
    KeyInputSwitch(int keysym, bool isup);
    const char *name() const;
    int id() const;

    virtual bool isArrowUp()   const;
    virtual bool isArrowDown() const;
    virtual bool isValidate()  const;
    virtual bool isCancel()    const;
    virtual bool isPause()     const;
    
  private:
    int keysym;
    mutable char keyName[256];
};

class JoystickSwitch : public InputSwitch
{
  private:
    int which;
    int button;
    mutable char keyName[256];

  public:
    JoystickSwitch(int which, int button, bool isup);
    const char *name() const;
    int id() const;

    virtual bool isValidate()  const;
    virtual bool isCancel()    const;
};

class JoystickAxisSwitch : public InputSwitch
{
  private:
    int which;
    int axis;
    bool maximum;
    mutable char keyName[256];

  public:
    JoystickAxisSwitch(int which, int axis, bool maximum, bool isup);
    const char *name() const;
    int id() const;

    virtual bool isArrowUp()   const;
    virtual bool isArrowDown() const;
};

class InputFromIDAndName : public InputSwitch
{
  int _id;
  char _name[256];
  public:
  InputFromIDAndName(int _id, const char *_name) : _id(_id) {strcpy(this->_name, _name);}
  const char *name() const { return _name; }
  int id() const           { return _id; }
};

#endif
