#include "gameui.h"

namespace gameui {

  GameUIEnum   GameUIDefaults::CONTAINER_POLICY = USE_MAX_SIZE;
  float        GameUIDefaults::SPACING          = 16.0f;
  SoFont      *GameUIDefaults::FONT           = SoFont_new();
  SoFont      *GameUIDefaults::FONT_INACTIVE  = SoFont_new();
  GameLoop    *GameUIDefaults::GAME_LOOP      = new GameLoop();
  ScreenStack *GameUIDefaults::SCREEN_STACK   = new ScreenStack();

  //
  // Widget
  // 
  Widget::Widget(WidgetContainer *parent)
    : parent(parent), preferedSize(0,0,0), size(0,0,0),
    position(0,0,0), hidden(false), focus(false), focusable(false)
    {
      for (int i = 0; i < GAMEUIENUM_LAST; ++i)
        actions[i] = NULL;
      requestDraw();
    }

  void Widget::draw(SDL_Surface *screen, bool force) const
  {
    if (/*(force || drawRequested()) && */ !hidden)
      draw(screen);
  }

  void Widget::eventOccured(GameControlEvent *event)
  {
    lostFocus();
  }

  void Widget::setFocusable(bool foc)
  {
    focusable = foc;
    if (parent && foc)
      parent->setFocusable(true);
  }

  //
  // WidgetContainer
  // 

  WidgetContainer::WidgetContainer(GameLoop *loop) : loop(loop) {
    if (loop == NULL) this->loop = GameUIDefaults::GAME_LOOP;
  }

  void WidgetContainer::add (Widget *child)    
  { 
    childs.add(child);
    child->setParent(this);
    if (child->getIdleComponent()) {
      loop->add(child->getIdleComponent());
    }
    arrangeWidgets();
  }

  void WidgetContainer::remove (Widget *child)
  {
    childs.remove(child);
    if (child->getIdleComponent()) {
      child->getIdleComponent()->kill();
    }
    arrangeWidgets();
  }

  void WidgetContainer::draw(SDL_Surface *surface) const 
  {
    for (int i = 0; i < getNumberOfChilds(); ++i) {
      getChild(i)->draw(surface);
    }
  }

  GameLoop *WidgetContainer::getGameLoop()
  {
    return loop;
  }

  bool WidgetContainer::bubbleSortZ_iteration(int itNumber)
  {
    // Remark: you may think that a bubble sort is a somehow slow sorting
    // algorithm, but in fact this the fastest possible on an already sorted array O(n)
    // In our case, the widget list will not change often: so bubble sort is a good choice.

    if (itNumber < 1) IOS_ERROR("Wrong bubbleSortZ_iteration itNumber\n");

    bool changed = false;

    for (int i = 0; i < childs.size() - itNumber; ++i) {
      Vec3 v1 = childs[i]->getPosition();
      Vec3 v2 = childs[i+1]->getPosition();
      if (v1.z > v2.z) {
        Widget *tmp = childs[i];
        childs[i] = childs[i+1];
        childs[i+1] = tmp;
        changed = true;
      }
    }
    return changed;
  }

  void WidgetContainer::sortWidgets()
  {
    int it = 1;
    // Sort widgets
    while (bubbleSortZ_iteration(it++));
  }

  void WidgetContainer::setSize(const Vec3 &v3)
  {
    Widget::setSize(v3);
    arrangeWidgets();
  }

  void WidgetContainer::setPosition(const Vec3 &v3)
  {
    Widget::setPosition(v3);
    arrangeWidgets();
  }

  // 
  // Box
  // 
  Box::Box(GameLoop *loop) : WidgetContainer(loop)
  {
    setPolicy(GameUIDefaults::CONTAINER_POLICY);
    activeWidget = -1;
  }

  void Box::setPolicy(GameUIEnum policy)
  {
    this->policy = policy;
    arrangeWidgets();
  }

  void Box::arrangeWidgets()
  {
    if (getNumberOfChilds() == 0) return;
    requestDraw();
    switch (policy)
    {
      case USE_MAX_SIZE:
        {
          float height = getSortingAxe(getSize());
          float heightPerChild = (height + GameUIDefaults::SPACING) / getNumberOfChilds()
            - GameUIDefaults::SPACING;
          Vec3 size     = getSize();
          setSortingAxe(size, heightPerChild);
          Vec3 position = getPosition();
          float axePos  = getSortingAxe(position);

          for (int i = 0; i < getNumberOfChilds(); ++i) {
            Widget *child = getChild(i);
            if (!child->getPreferedSize().is_zero()) {
              // center the widget if we know its size
              Vec3 csize = size - child->getPreferedSize();
              Vec3 cpos  = position + csize / 2.0;
              child->setSize(child->getPreferedSize());
              child->setPosition(cpos);
            }
            else {
              // else give him all the space he want.
              child->setSize(size);
              child->setPosition(position);
            }
            axePos += heightPerChild + GameUIDefaults::SPACING;
            setSortingAxe(position, axePos);
          }
        }
        break;
      case USE_MIN_SIZE:
        {}
        break;
      default:
        {}
    }
  }

  void Box::add (Widget *child)
  {
    WidgetContainer::add(child);
    if (child->isFocusable() && (activeWidget < 0)) {
      activeWidget = getNumberOfChilds() - 1;
      setFocusable(true);
    }
  }

  void Box::eventOccured(GameControlEvent *event)
  {
    if (event->isUp)
      return;

    if (activeWidget >= getNumberOfChilds())
      activeWidget = getNumberOfChilds() - 1;

    if (activeWidget < 0) {
      lostFocus();
      return;
    }

    Widget *child = getChild(activeWidget);
    child->giveFocus();
    child->eventOccured(event);

    if (child->haveFocus()) return;
    int direction = 0;

    if (isPrevEvent(event))
      direction = -1;
    else if (isNextEvent(event))
      direction = 1;
    else {
      if (isOtherDirection(event))
        lostFocus();
      return;
    }

    bool childHaveFocus;    
    int saveActive = activeWidget;
    do {
      activeWidget += direction;
      if (activeWidget < 0) {
        setActiveWidget(saveActive);
        lostFocus();
        return;
      }
      if (activeWidget >= getNumberOfChilds()) {
        setActiveWidget(saveActive);
        lostFocus();
        return;
      }
      childHaveFocus = giveFocusToActiveWidget();
    }
    while (!childHaveFocus);
  }

  void Box::setActiveWidget(int i)
  {
    activeWidget = i;
    if (activeWidget < 0)
      activeWidget = 0;
    if (activeWidget >= getNumberOfChilds())
      activeWidget = getNumberOfChilds() - 1;
    giveFocusToActiveWidget();
  }

  bool Box::giveFocusToActiveWidget()
  {
    GameControlEvent ev;
    ev.cursorEvent = GameControlEvent::kCursorNone;
    ev.gameEvent   = GameControlEvent::kGameNone;
    ev.isUp        = false;

    Widget *child = getChild(activeWidget);
    child->giveFocus();
    child->eventOccured(&ev);
    return child->haveFocus();
  }

  void Box::giveFocus()
  {
    WidgetContainer::giveFocus();
    if (activeWidget >= 0) {
      Widget *child = getChild(activeWidget);
      child->giveFocus();
    }
  }

  void Box::lostFocus()
  {
    WidgetContainer::lostFocus();
    if (activeWidget >= 0) {
      Widget *child = getChild(activeWidget);
      child->lostFocus();
    }
  }


  //
  // HBox
  // 
  bool HBox::isPrevEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kLeft;
  }

  bool HBox::isNextEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kRight;
  }

  bool HBox::isOtherDirection(GameControlEvent *event) const
  {
    return (event->cursorEvent == GameControlEvent::kUp)
      ||   (event->cursorEvent == GameControlEvent::kDown);
  }


  //
  // VBox
  //
  bool VBox::isPrevEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kUp;
  }
  bool VBox::isNextEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kDown;
  }
  bool VBox::isOtherDirection(GameControlEvent *event) const
  {
    return (event->cursorEvent == GameControlEvent::kRight)
      ||   (event->cursorEvent == GameControlEvent::kLeft);
  }

  //
  // ScreenVBox
  //

  Screen::Screen(float x, float y, float width, float height, GameLoop *loop) : VBox(loop), bg(NULL)
  {
    setPosition(Vec3(x, y, 1.0f));
    setSize(Vec3(width, height, 1.0f));
    giveFocus();
  }

  void Screen::setBackground(IIM_Surface *bg)
  {
    this->bg = bg;
  }

  void Screen::draw(SDL_Surface *surface) const
  {
    if (bg) {
      SDL_Rect rect;
      rect.x = (Sint16)getPosition().x;
      rect.y = (Sint16)getPosition().y;
      rect.w = (Uint16)getSize().x;
      rect.h = (Uint16)getSize().y;
      SDL_BlitSurface(bg->surf, NULL, surface, &rect);
    }
    VBox::draw(surface);
  }

  void Screen::onEvent(GameControlEvent *event)
  {
    if (!isVisible()) return;
    VBox::eventOccured(event);
    VBox::giveFocus();
    requestDraw();
  }

  //
  // Text
  // 

  Text::Text(const String &label, SoFont *font)
    : font(font), label(label)
    {
      if (font == NULL) this->font = GameUIDefaults::FONT;
      setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
    }

  void Text::setValue(String value)
  {
    label = value;
    requestDraw();
    setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
    if (parent)
      parent->arrangeWidgets();
  }

  void Text::draw(SDL_Surface *screen) const
  {
    SoFont_PutString(font, screen, (int)getPosition().x, (int)getPosition().y, (const char*)label, NULL);
  }

  //
  // Button
  // 

  void Button::init(SoFont *fontActive, SoFont *fontInactive)
  {
    if (fontInactive == NULL) fontInactive = GameUIDefaults::FONT_INACTIVE;
    if (fontActive == NULL)   fontActive = GameUIDefaults::FONT;

    this->fontActive   = fontActive;
    this->fontInactive = fontInactive;

    font = fontInactive;
    setFocusable(true);
  }

  Button::Button(const String &label, SoFont *fontActive, SoFont *fontInactive)
    : Text(label, fontInactive)
    {
      init(fontActive, fontInactive);
    }

  Button::Button(const String &label, Action *action)
    : Text(label, NULL)
    {
      init(NULL,NULL);
      setAction(ON_START, action);
    }

  static bool isDirectionEvent(GameControlEvent *event)
  {
    if (event->cursorEvent == GameControlEvent::kUp)
      return true;
    if (event->cursorEvent == GameControlEvent::kRight)
      return true;
    if (event->cursorEvent == GameControlEvent::kLeft)
      return true;
    if (event->cursorEvent == GameControlEvent::kDown)
      return true;
    return false;
  }

  void Button::eventOccured(GameControlEvent *event)
  {
    if (event->isUp)
      return;

    if (isDirectionEvent(event))
      lostFocus();

    if (event->cursorEvent == GameControlEvent::kStart) {
      Action *action = getAction(ON_START);
      if (action)
        action->action();
    }
  }

  void Button::lostFocus() {
    Text::lostFocus();
    font = fontInactive;
    requestDraw();
  }


  void Button::giveFocus() {
    Text::giveFocus();
    font = fontActive;
    requestDraw();
  }

  //
  // EditField
  //

  void EditField::init(SoFont *fontActive, SoFont *fontInactive)
  {
    if (fontInactive == NULL) fontInactive = GameUIDefaults::FONT_INACTIVE;
    if (fontActive == NULL)   fontActive = GameUIDefaults::FONT;

    this->fontActive   = fontActive;
    this->fontInactive = fontInactive;

    font = fontInactive;
    editionMode = false;

    setFocusable(true);
  }

  EditField::EditField(const String &defaultText,  Action *action)
    : Text(defaultText, NULL)
    {
      init(NULL,NULL);
      if (action != NULL)
        setAction(ON_START, action);
    }

  void EditField::eventOccured(GameControlEvent *event)
  {
    if (event->isUp)
      return;

    if (event->cursorEvent == GameControlEvent::kStart) {
      editionMode = !editionMode;
      if (editionMode == true) {
        previousValue = getValue();
        setValue("_");
      }
      else {
        setValue(getValue().substring(0, getValue().length() - 1));
        Action *action = getAction(ON_START);
        if (action)
            action->action();
      }
    }
    else if (editionMode == true) {
      if (event->cursorEvent == GameControlEvent::kBack) {
        setValue(previousValue);
        editionMode = false;
      }
      else if (event->sdl_event.type == SDL_KEYDOWN) {
        SDL_Event e = event->sdl_event;
        char ch = 0;
        if (e.key.keysym.sym == SDLK_PERIOD)
          ch = e.key.keysym.sym;
        if (e.key.keysym.sym == SDLK_SLASH)
          ch = e.key.keysym.sym;
        if (e.key.keysym.sym == SDLK_MINUS)
          ch = e.key.keysym.sym;
        if (e.key.keysym.sym == SDLK_COLON)
          ch = e.key.keysym.sym;

        if ((e.key.keysym.sym >= SDLK_KP0) && (e.key.keysym.sym <= SDLK_KP9))
          ch = e.key.keysym.sym - SDLK_KP0 + '0';
        if (e.key.keysym.sym == SDLK_KP_PERIOD)
          ch = '.';

        if ((e.key.keysym.sym >= SDLK_0) && (e.key.keysym.sym <= SDLK_9))
          ch = e.key.keysym.sym;

        if ((e.key.keysym.sym >= SDLK_a) && (e.key.keysym.sym <= SDLK_z))
          ch = e.key.keysym.sym;

        if (e.key.keysym.sym == SDLK_SPACE)
          ch = ' ';

        if (e.key.keysym.sym == SDLK_BACKSPACE && (getValue().length() > 1)) {
          String newValue = getValue().substring(0, getValue().length() - 2);
          newValue += "_";
          setValue(newValue);
        }

        if (ch) {
          String newValue = getValue();
          newValue[newValue.length() - 1] = ch;
          newValue += "_";
          setValue(newValue);
        }
      }
    }
    else {
      if (isDirectionEvent(event))
        lostFocus();
    }
  }

  void EditField::lostFocus() {
    Text::lostFocus();
    font = fontInactive;
    requestDraw();
  }

  void EditField::giveFocus() {
    Text::giveFocus();
    font = fontActive;
    requestDraw();
  }


  //
  // EditFieldWithLabel
  //

  EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, Action *action)
  {
    add(new Text(label));
    editField = new EditField(defaultValue, action);
    add(editField);
  }

  //
  // Separator
  //

  Separator::Separator(float width, float height)
  {
    setPreferedSize(Vec3(width, height, 1.0));
  }

  //
  // ScreenStack
  //

  void ScreenStack::checkLoop() {
    if (loop == NULL)
      loop = GameUIDefaults::GAME_LOOP;
  }

  ScreenStack::ScreenStack(GameLoop *loop)
  {
    this->loop = loop;
  }

  void ScreenStack::push(Screen *screen) {
    checkLoop();
    if (stack.size() > 0) {
      stack.top()->hide();
    }
    screen->show();
    stack.push(screen);
    loop->add(screen);
  }

  void ScreenStack::pop() {
    stack.top()->remove();
    stack.top()->hide();
    stack.pop();
    stack.top()->show();
  }

  //
  // PushScreenAction
  //
  PushScreenAction::PushScreenAction(Screen *screen, ScreenStack *stack)
  {
    if (stack == NULL) stack = GameUIDefaults::SCREEN_STACK;
    this->stack = stack;
    this->screen = screen;
  }

  void PushScreenAction::action() {
    stack->push(screen);
  }

  //
  // PopScreenAction
  //
  PopScreenAction::PopScreenAction(ScreenStack *stack)
  {
    if (stack == NULL) stack = GameUIDefaults::SCREEN_STACK;
    this->stack = stack;
  }

  void PopScreenAction::action()
  {
    stack->pop();
  }

};

