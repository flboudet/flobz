#ifndef _INPUT_SW_MAN_H
#define _INPUT_SW_MAN_H

#include <string>
#include <string.h>
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

const int JOYSTICK_THRESHOLD = 28000;

class InputSwitch
{
  public:

    InputSwitch(bool isup = false) : isup(isup) {}
    virtual ~InputSwitch() {}
    static InputSwitch *createFromString(std::string str);

    virtual const char *name() const = 0;
    virtual int id() const = 0;

    inline bool operator==(const InputSwitch &other) const {
      return id() == other.id();
    }

    inline bool isUp()   const { return isup; }
    inline bool isDown() const { return !isup; }

    /* Meta-Informations about the switch */
    virtual bool isArrowUp()    const { return false; }
    virtual bool isArrowDown()  const { return false; }
    virtual bool isArrowLeft()  const { return false; }
    virtual bool isArrowRight() const { return false; }
    virtual bool isValidate()   const { return false; }
    virtual bool isCancel()     const { return false; }
    virtual bool isPause()      const { return false; }
    virtual bool isQuit()       const { return false; }

    virtual bool isJoystick()   const { return false; }

    virtual std::string toString() const = 0;
  protected:
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
#if SDL_VERSION_ATLEAST(1, 3, 0)
    KeyInputSwitch(int keysym, bool isup, Uint16 keymod = KMOD_NONE);
#else
    KeyInputSwitch(int keysym, bool isup, SDLMod keymod = KMOD_NONE);
#endif
    KeyInputSwitch(std::string str);
    const char *name() const;
    int id() const;

    virtual bool isArrowUp()   const;
    virtual bool isArrowDown() const;
    virtual bool isArrowLeft()  const;
    virtual bool isArrowRight() const;
    virtual bool isValidate()  const;
    virtual bool isCancel()    const;
    virtual bool isPause()     const;
    virtual bool isQuit()      const;

    virtual std::string toString() const;

  private:
    int keysym;
#if SDL_VERSION_ATLEAST(2, 0, 0)
    Uint16 keymod;
#else
    SDLMod keymod;
#endif
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
    JoystickSwitch(std::string str);
    const char *name() const;
    int id() const;

    virtual bool isValidate()  const;
    virtual bool isCancel()    const;
    virtual bool isJoystick()  const { return true; }

    virtual std::string toString() const;
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
    JoystickAxisSwitch(std::string str);
    const char *name() const;
    int id() const;

    virtual bool isArrowUp()   const;
    virtual bool isArrowDown() const;
    virtual bool isArrowLeft()  const;
    virtual bool isArrowRight() const;
    virtual bool isJoystick()  const { return true; }

    virtual std::string toString() const;
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
