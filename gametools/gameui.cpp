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
    position(0,0,0), hidden(false), focus(false), focusable(false), _drawRequested(true), receiveUp(false)
    {
      for (int i = 0; i < GAMEUIENUM_LAST; ++i)
        actions[i] = NULL;
      requestDraw();
    }

  void Widget::hide()   { hidden = true;                 }
  void Widget::show()
  {
    hidden = false;
    IdleComponent *idle = getIdleComponent();
    if (idle != NULL) {
      idle->setPause(false);
    }
    requestDraw();
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
  
  void Widget::requestDraw(bool fromParent)
  {
    _drawRequested = true;
    if (parent && !fromParent)
        parent->widgetMustRedraw(this);
  }

  void Widget::doDraw(SDL_Surface *screen)
  {
    if (_drawRequested) {
        draw(screen);
        _drawRequested = false;
    }
  }

  void Widget::addToGameLoop(GameLoop *loop)
  {
    IdleComponent *idle = getIdleComponent();
    if (idle != NULL) {
      idle->setPause(false);
      loop->add(idle);
    }
  }
  
  void Widget::removeFromGameLoop()
  {
    IdleComponent *idle = getIdleComponent();
    if (idle != NULL) {
      idle->remove();
//      idle->setPause(true);
    }
  }
  
  void Widget::removeFromGameLoopActive()
  {
    IdleComponent *idle = getIdleComponent();
    if (idle != NULL)
      GameUIDefaults::GAME_LOOP->remove(idle);
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
    child->addToGameLoop(loop);
    arrangeWidgets();
  }

  void WidgetContainer::remove (Widget *child)
  {
    childs.remove(child);
    child->removeFromGameLoop();
    arrangeWidgets();
  }

  void WidgetContainer::addToGameLoop(GameLoop *loop)
  {
    Widget::addToGameLoop(loop);
    for (int i = 0; i < getNumberOfChilds(); ++i)
      getChild(i)->addToGameLoop(loop);
  }
  
  void WidgetContainer::removeFromGameLoop()
  {
    Widget::removeFromGameLoop();
    for (int i = 0; i < getNumberOfChilds(); ++i)
      getChild(i)->removeFromGameLoop();
  }
    
  void WidgetContainer::removeFromGameLoopActive()
  {
    Widget::removeFromGameLoopActive();
    for (int i = 0; i < getNumberOfChilds(); ++i)
      getChild(i)->removeFromGameLoopActive();
  }

  void WidgetContainer::draw(SDL_Surface *surface) 
  {
    for (int i = 0; i < getNumberOfChilds(); ++i) {
      getChild(i)->doDraw(surface);
    }
  }
  
  void WidgetContainer::requestDraw(bool fromParent)
  {
      if (fromParent) {
          for (int i = 0; i < getNumberOfChilds(); ++i) {
              getChild(i)->requestDraw(true);
          }
      }
      Widget::requestDraw(fromParent);
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
  
  void Box::setFocusable(bool foc)
  {
    Widget::setFocusable(foc);
    if (foc) {
        for (int i = 0, j = getNumberOfChilds() ; i < j ; i++) {
            if (getChild(i)->isFocusable()) {
                activeWidget =  i;
                return;
            }
        }
    }
  }

  void Box::eventOccured(GameControlEvent *event)
  {
    if (activeWidget >= getNumberOfChilds())
      activeWidget = getNumberOfChilds() - 1;

    if (activeWidget < 0) {
      lostFocus();
      return;
    }

    Widget *child = getChild(activeWidget);
    if ((event->isUp) && (! child->receiveUpEvents()))
      return;
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
  // ZBox
  //
  bool ZBox::isPrevEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kUp;
  }
  bool ZBox::isNextEvent(GameControlEvent *event) const
  {
    return event->cursorEvent == GameControlEvent::kDown;
  }
  bool ZBox::isOtherDirection(GameControlEvent *event) const
  {
    return (event->cursorEvent == GameControlEvent::kRight)
      ||   (event->cursorEvent == GameControlEvent::kLeft);
  }
  void ZBox::widgetMustRedraw(Widget *wid)
  {
    /*for (int i = 0; i < getNumberOfChilds(); ++i) {
        getChild(i)->requestDraw(true);
    }*/
    requestDraw(true);
  }

  // SliderContainer

  SliderContainer::SliderContainer(GameLoop *loop)
    : ZBox(loop), contentWidget(NULL), previousWidget(NULL)
    , currentTime(0), slideStartTime(0)
    , sliding(false), bg(NULL)
    {
    }
  
  void SliderContainer::draw(SDL_Surface *screen)
  {
    if (bg != NULL)
    {
      IIM_Rect rect;
      rect.x = (Sint16)getPosition().x;
      rect.y = (Sint16)getPosition().y;
      IIM_BlitSurface(bg, NULL, screen, &rect);
    }
    ZBox::draw(screen);
  }

  void SliderContainer::transitionToContent(Widget *content)
  {
    if (sliding)
      return;
    previousWidget = contentWidget;
    slideStartTime = currentTime;
    if (contentWidget)
      contentWidget->lostFocus();
    contentWidget = content;
    add(contentWidget);
    sliding = true;

    if (previousWidget) previousWidget->removeFromGameLoop();
    if (contentWidget) contentWidget->removeFromGameLoop();
  }

  void SliderContainer::idle(double currentTime)
  {
    static const double slidingTime = .3;
    this->currentTime = currentTime;

    if (!sliding) return;
    double t = (currentTime - slideStartTime);
    
    if ((previousWidget == NULL) || (t > slidingTime) || (contentWidget == NULL) || (t < 0.))
    {
      if (previousWidget != NULL)
        ZBox::remove(previousWidget);
      if (contentWidget != NULL)
        contentWidget->addToGameLoop(getGameLoop());
      sliding = false;
      return;
    }

    double coef1 = t / slidingTime - 0.4;
    double coef2 = t / slidingTime + 0.4;

    if (coef1 < 0.) coef1 = 0.;
    if (coef2 > 1.) coef2 = 1.;

    Vec3 pos1 = getPosition();
    Vec3 siz1 = getSize();
    pos1.x += 5;
    pos1.y += 5;
    siz1.x -= 10;
    siz1.y -= 10;
    Vec3 pos2 = pos1;
    Vec3 siz2 = siz1;
    
    double shrink1 = coef1 * siz1.y;
    double shrink2 = coef2 * siz1.y;
    
    pos1.y = pos1.y + (siz1.y - shrink1) / 2.;
    pos2.y = pos2.y + shrink2 / 2.;
    siz1.y = shrink1;
    siz2.y = siz2.y - shrink2;

    contentWidget->setPosition(pos1);
    contentWidget->setSize(siz1);
    previousWidget->setPosition(pos2);
    previousWidget->setSize(siz2);
  }

  //
  // ScreenVBox
  //

  Screen::Screen(float x, float y, float width, float height, GameLoop *loop) : rootContainer(loop), bg(NULL)
  {
    rootContainer.setPosition(Vec3(x, y, 1.0f));
    rootContainer.setSize(Vec3(width, height, 1.0f));
    rootContainer.giveFocus();
  }

  void Screen::setBackground(IIM_Surface *bg)
  {
    this->bg = bg;
  }

  void Screen::draw(SDL_Surface *surface)
  {
    if (!isVisible()) return;
    /*if (bg) {
      SDL_Rect rect;
      rect.x = (Sint16)rootContainer.getPosition().x;
      rect.y = (Sint16)rootContainer.getPosition().y;
      rect.w = (Uint16)rootContainer.getSize().x;
      rect.h = (Uint16)rootContainer.getSize().y;
      SDL_BlitSurface(bg->surf, NULL, surface, &rect);
    }*/
    rootContainer.doDraw(surface);
  }
  
  void Screen::drawAnyway(SDL_Surface *surface)
  {
    rootContainer.requestDraw(true);
    rootContainer.doDraw(surface);
  }

  void Screen::onEvent(GameControlEvent *event)
  {
    if (!isVisible()) return;
    rootContainer.eventOccured(event);
    rootContainer.giveFocus();
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
      offsetX = offsetY = 0.;
    }

  void Text::setValue(String value)
  {
    label = value;
    requestDraw();
    setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
    if (parent)
      parent->arrangeWidgets();
  }

  void Text::draw(SDL_Surface *screen)
  {
    SoFont_PutString(font, screen, (int)(offsetX + getPosition().x), (int)(offsetY + getPosition().y), (const char*)label, NULL);
  }

  void Text::idle(double currentTime)
  {
    offsetX = 7. * (sin(getPosition().x) + sin(getPosition().y + currentTime));
    offsetY = 3. * (cos(getPosition().x+1) + sin(getPosition().y - currentTime * 1.1));
    requestDraw();
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
  // ToggleButton
  //
  
  ToggleButton::ToggleButton(const String &label, const String &offState, const String &onState, bool initialState, Action *action) : Button(label + "  " + (initialState ? onState : offState), action), unmodifiedLabel(label), onState(onState), offState(offState)
  {
  }
  
  void ToggleButton::setToggle(bool toggleValue)
  {
    setValue(unmodifiedLabel + "  " + (toggleValue ? onState : offState));
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
      stack.top()->removeFromGameLoop();
    }
    screen->show();
    screen->addToGameLoop(screen->getGameLoop());
    stack.push(screen);
    loop->add(screen);
  }

  void ScreenStack::pop() {
    stack.top()->remove();
    stack.top()->removeFromGameLoop();
    stack.top()->hide();
    stack.pop();
    stack.top()->show();
    stack.top()->addToGameLoop(stack.top()->getGameLoop());
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

