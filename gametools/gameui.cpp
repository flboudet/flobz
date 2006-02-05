#include "gameui.h"
#include "../audio.h"

namespace gameui {

  GameUIEnum   GameUIDefaults::CONTAINER_POLICY = USE_MAX_SIZE;
  float        GameUIDefaults::SPACING          = 16.0f;
  SoFont      *GameUIDefaults::FONT             = SoFont_new();
  SoFont      *GameUIDefaults::FONT_TEXT        = SoFont_new();
  SoFont      *GameUIDefaults::FONT_INACTIVE    = SoFont_new();
  SoFont      *GameUIDefaults::FONT_SMALL_ACTIVE= SoFont_new();
  SoFont      *GameUIDefaults::FONT_SMALL_INFO  = SoFont_new();
  GameLoop    *GameUIDefaults::GAME_LOOP        = new GameLoop();
  ScreenStack *GameUIDefaults::SCREEN_STACK     = new ScreenStack();

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
    
  Widget::~Widget()
  {
    if (parent != NULL) {
        parent->remove(this);
    }
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
    checkFocus();
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
      loop->addIdle(idle);
    }
  }
  
  void Widget::removeFromGameLoopActive()
  {
    IdleComponent *idle = getIdleComponent();
    if (idle != NULL)
      GameUIDefaults::GAME_LOOP->removeIdle(idle);
  }
  
  //
  // WidgetContainer
  // 

  WidgetContainer::WidgetContainer(GameLoop *loop) : loop(loop), addedToGameLoop(false) {
    if (loop == NULL) this->loop = GameUIDefaults::GAME_LOOP;
  }

  WidgetContainer::~WidgetContainer()
  {
    for (int i = childs.size()-1 ; i >= 0 ; i--) {
      remove(childs[i]);
    }
  }

  void WidgetContainer::add (Widget *child)    
  { 
    childs.add(child);
    child->setParent(this);
    if (addedToGameLoop)
        child->addToGameLoop(loop);
    arrangeWidgets();
  }

  void WidgetContainer::remove (Widget *child)
  {
    childs.remove(child);
    child->setParent(NULL);
    child->removeFromGameLoopActive();
    arrangeWidgets();
  }

  void WidgetContainer::changeChild(int i, Widget *w)
  {
    Widget *tmp = getChild(i);
    if (tmp)
      tmp->removeFromGameLoopActive();
    childs[i] = w;
    w->setParent(this);
    if (addedToGameLoop)
      w->addToGameLoop(loop);
    arrangeWidgets();
  }

  void WidgetContainer::addToGameLoop(GameLoop *loop)
  {
    addedToGameLoop = true;
    Widget::addToGameLoop(loop);
    for (int i = 0; i < getNumberOfChilds(); ++i)
      getChild(i)->addToGameLoop(loop);
  }
  
  void WidgetContainer::removeFromGameLoopActive()
  {
    addedToGameLoop = false;
    Widget::removeFromGameLoopActive();
    for (int i = 0; i < getNumberOfChilds(); ++i)
      getChild(i)->removeFromGameLoopActive();
  }

  bool WidgetContainer::hasWidget(Widget *wid)
  {
    for (int i = 0, j = getNumberOfChilds() ; i < j ; i++) {
      if (getChild(i) == wid)
	return true;
    }
    return false;
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
    checkFocus();
    switch (policy)
    {
      case USE_MAX_SIZE:
        {
          float height = getSortingAxe(getSize());
 
            float heightOfKnownChilds = 0.0f;
          for (int i = 0; i < getNumberOfChilds(); ++i) {
            Widget *child = getChild(i);
            if (!child->getPreferedSize().is_zero()) {
                //numberOfknownChilds++;
                heightOfKnownChilds += getSortingAxe(child->getPreferedSize());
            }
          }
          //if (getNumberOfChilds() != 0) heightOfKnownChilds /= getNumberOfChilds();

         float heightPerChild = (height - heightOfKnownChilds) / (getNumberOfChilds());
         float heightToRemove = 0.0;
         if (heightPerChild < GameUIDefaults::SPACING)
         {
            if (height >= GameUIDefaults::SPACING * getNumberOfChilds())
            {
                heightToRemove = heightOfKnownChilds - (height - GameUIDefaults::SPACING * getNumberOfChilds());
                heightPerChild = GameUIDefaults::SPACING;
            }
            else
            {
                heightPerChild = (GameUIDefaults::SPACING * getNumberOfChilds() - height) / getNumberOfChilds();
                heightToRemove = heightOfKnownChilds - (height - heightPerChild * getNumberOfChilds());
            }
         }
         
          Vec3 size     = getSize();
          setSortingAxe(size, heightPerChild);
          Vec3 position = getPosition();
          setSortingAxe(position, getSortingAxe(position)-heightPerChild/2.0);
          float axePos  = getSortingAxe(position) ;


         for (int i = 0; i < getNumberOfChilds(); ++i) {
            Widget *child = getChild(i);
            if (!child->getPreferedSize().is_zero()) {
              // center the widget if we know its size
              float coeff = getSortingAxe(child->getPreferedSize()) / heightOfKnownChilds;
              Vec3 csize = size - child->getPreferedSize();
              Vec3 offset(0.0,0.0,0.0);
              setSortingAxe(offset, heightPerChild - getSortingAxe(csize / 2.0));
              Vec3 sizeOffset(0.0,0.0,0.0);
              setSortingAxe(sizeOffset,  heightToRemove*coeff);
              Vec3 cpos  = position + csize / 2.0 + offset;
              child->setSize(child->getPreferedSize()-sizeOffset);
              child->setPosition(cpos);
              axePos += getSortingAxe(child->getSize());
            }
            else {
              // else give him all the space he want.
              child->setSize(size);
              Vec3 offset(0.0,0.0,0.0);
              setSortingAxe(offset, heightPerChild - getSortingAxe(size / 2.0));
              child->setPosition(position+ offset);
            }
            axePos += heightPerChild;
            setSortingAxe(position, axePos);
          }
        }
        break;
      case USE_MIN_SIZE:
        {
          /* TODO to be continued
          float y       = GameUIDefaults::SPACING;
          float height  = getSortingAxe(getSize());
          Vec3 size     = getSize();
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
          }*/
        }
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
        checkFocus();
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
    // Handle the case where the widget has self-destroyed
    if (!hasWidget(child)) {
      return; // there are probably wiser things to do
    }

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
    do {
      activeWidget += direction;
      if (activeWidget < 0) {
        setActiveWidget(getNumberOfChilds()-1);
        lostFocus();
      }
      if (activeWidget >= getNumberOfChilds()) {
        setActiveWidget(0);
        lostFocus();
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

  void Box::focus(Widget *widget)
  {
    for (int i = 0; i < getNumberOfChilds(); ++i) {
      if (getChild(i) == widget) {
        setActiveWidget(i);
        return;
      }
    }
  }

  bool Box::giveFocusToActiveWidget()
  {
    GameControlEvent ev;
    ev.cursorEvent = GameControlEvent::kCursorNone;
    ev.gameEvent   = GameControlEvent::kGameNone;
    ev.isUp        = false;
    ev.sdl_event.type = SDL_NOEVENT;

    Widget *child = getChild(activeWidget);
    child->giveFocus();
    child->eventOccured(&ev);
    return child->haveFocus();
  }

  void Box::giveFocus()
  {
    WidgetContainer::giveFocus();
    if ((activeWidget >= 0) && (getChild(activeWidget)->isFocusable() && (activeWidget < getNumberOfChilds()))) {
      Widget *child = getChild(activeWidget);
      child->giveFocus();
    }
    else {
      checkFocus();
    }
  }

  void Box::checkFocus()
  {
    if (!haveFocus()) return;
    
    GameControlEvent ev;
    ev.cursorEvent = GameControlEvent::kCursorNone;
    ev.gameEvent   = GameControlEvent::kGameNone;
    ev.isUp        = false;
    ev.sdl_event.type = SDL_NOEVENT;

    eventOccured(&ev);

    if (activeWidget<0) return;
    if (activeWidget>=getNumberOfChilds()) return;

    giveFocusToActiveWidget();
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
    return false;
//    return event->cursorEvent == GameControlEvent::kUp;
  }
  bool ZBox::isNextEvent(GameControlEvent *event) const
  {
    return false;
//    return event->cursorEvent == GameControlEvent::kDown;
  }
  bool ZBox::isOtherDirection(GameControlEvent *event) const
  {
    return (event->cursorEvent == GameControlEvent::kRight)
      ||   (event->cursorEvent == GameControlEvent::kLeft)
      ||   (event->cursorEvent == GameControlEvent::kUp)
      ||   (event->cursorEvent == GameControlEvent::kDown);
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
    : IdleComponent()
    , ZBox(loop)
    , contentWidget(NULL), previousWidget(NULL)
    , currentTime(0.0), slideStartTime(0.0), slidingOffset(0.0)
    , sliding(false), bg(NULL)
    {
      slidingOffset = getPosition().x;
    }
  
  void SliderContainer::eventOccured(GameControlEvent *event)
  {
    if ( !(sliding && slideout)) ZBox::eventOccured(event);
  }
  
  void SliderContainer::draw(SDL_Surface *screen)
  {
    if (bg != NULL)
    {
      IIM_Rect rect;
      rect.x = (Sint16)(getPosition().x+slidingOffset);
      rect.y = (Sint16)getPosition().y;
      IIM_BlitSurface(bg, NULL, screen, &rect);
    }
    ZBox::draw(screen);
  }

  void SliderContainer::transitionToContent(Widget *content)
  {
    if (sliding)
    {
      if (slideout)
      {
        previousWidget->lostFocus();
        previousWidget->removeFromGameLoopActive();
        remove(previousWidget);          
        if (contentWidget != NULL)
        {
          add(contentWidget);
          contentWidget->addToGameLoop(getGameLoop());
        }
      }
    }
      
    slideStartTime = currentTime;
    previousWidget = contentWidget;
    contentWidget = content;
    if (previousWidget != NULL)
    {
      sliding = true;
      slideout= true;
      previousWidget->lostFocus();
      AudioManager::playSound("whop.wav", .1);
      //previousWidget->removeFromGameLoopActive();
    }
    else
    {
      if (contentWidget != NULL)
      {
        sliding = true;
        slideout= false;
        add(contentWidget);
        contentWidget->addToGameLoop(getGameLoop());
        AudioManager::playSound("whip.wav", .1);
      }
      else
      {
        sliding = false;
        /* Maybe should move myself out here... */
      }
    }
  }

  void SliderContainer::idle(double currentTime)
  {
    static const double slidingTime = .4;
    double oldtime = this->currentTime - slideStartTime;
    
    this->currentTime = currentTime;
    
    if (!sliding) return;
    
    double t = (currentTime - slideStartTime);
    
    if (t > slidingTime) // At end of sliding period
    {
      if (slideout) // If the previous widget has gone
      {
        // Then remove it once for all
        remove(previousWidget);
        /* Maybe should move myself out here... */

        if (contentWidget != NULL) // there is a new widget to show
        {
          // Then slide in the new widget
          t = 0;
          slideStartTime = currentTime;
          slideout = false;
          add(contentWidget);
          contentWidget->addToGameLoop(getGameLoop());
          AudioManager::playSound("whip.wav", .1);
        }
        else // Stop here
        {
          sliding = false;
          /* Maybe should move myself out here... */
          return;
        }
      }
      else // The new widget is here
      {
        sliding = false;
        /* Maybe should move myself in here... */
        return;
      }
    }
    
    //previousWidget->removeFromGameLoopActive();
    //contentWidget->addToGameLoop(getGameLoop());
    //giveFocusToActiveWidget();
    
    //Widget * slidingWidget = slideout ? previousWidget : contentWidget;
    
    Vec3 pos1 = getPosition();
    Vec3 pos2 = pos1;
    
    double distance = getSize().x;
    IdleComponent * idle = NULL;
        
    if (slideout)
    {
      double stime = t*t;
      double shtime = slidingTime*slidingTime;
      slidingOffset = distance*stime/shtime;
      pos1.x += distance;
      pos2.x += slidingOffset;
      //if (previousWidget != NULL) idle = previousWidget->getIdleComponent();
    }
    else
    {
      double stime = (slidingTime-t)*(slidingTime-t);
      double shtime = slidingTime*slidingTime;
      slidingOffset = distance*stime/shtime;
      pos2.x += distance;
      pos1.x += slidingOffset;
      //idle = contentWidget->getIdleComponent();
    }
    
    requestDraw();
    if (contentWidget  != NULL) contentWidget->setPosition(pos1);
    if (previousWidget != NULL) previousWidget->setPosition(pos2);
    if (idle != NULL) idle->idle(currentTime);
  }
  
  //
  // HScrollList
  //
#define kHScrollListNbAddedItemsToEachSide 1

  HScrollList::HScrollList(GameLoop *loop)
  {
  	lastvisible = firstvisible = -1;
  }
  int HScrollList::getNumberOfVisibleChilds() {
  	if (activeWidget==-1) return 0;
  	if (getNumberOfChilds()<(1+2*kHScrollListNbAddedItemsToEachSide)) return getNumberOfChilds();
  	else return 1+2*kHScrollListNbAddedItemsToEachSide;
  }
  
  bool HScrollList::isItemVisible(int i)
  {
  	if (firstvisible <= lastvisible)
  	{
  		return ((i>=firstvisible) && (i<=lastvisible));
  	}
  	else
  	{
  		return !((i>lastvisible) && (i<firstvisible));
  	}
  }
  
  Widget* HScrollList::getVisibleChild(int i) { return getChild((i+firstvisible)%getNumberOfChilds());}
  void HScrollList::updateShownWidgets(void)
  {
  	if ((activeWidget==-1) || (getNumberOfChilds() == 0))
  	{
  		firstvisible = lastvisible = -1;
  		return;
  	}
  	if (getNumberOfVisibleChilds()>=1+2*kHScrollListNbAddedItemsToEachSide)
  	{
  	  firstvisible = (getNumberOfChilds() + activeWidget - kHScrollListNbAddedItemsToEachSide)%getNumberOfChilds();
  	  lastvisible = (activeWidget + kHScrollListNbAddedItemsToEachSide)%getNumberOfChilds();
  	}
  	else
  	{
  		firstvisible = (activeWidget + getNumberOfChilds() - (getNumberOfChilds()-1)/2) % getNumberOfChilds();
  		lastvisible = (firstvisible + getNumberOfChilds() - 1) % getNumberOfChilds();
  	}
  }
  
  void HScrollList::eventOccured(GameControlEvent *event)
  {
  	int old = activeWidget;
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
    do {
      activeWidget += direction;
      if (activeWidget < 0) {
        setActiveWidget(getNumberOfChilds()-1);
        lostFocus();
      }
      if (activeWidget >= getNumberOfChilds()) {
        setActiveWidget(0);
        lostFocus();
      }
      childHaveFocus = giveFocusToActiveWidget();
    }
    while (!childHaveFocus);
    arrangeWidgets();
    GameControlEvent ev;
    ev.cursorEvent = GameControlEvent::kStart;
    ev.gameEvent   = GameControlEvent::kGameNone;
    ev.isUp        = false;
    getChild(activeWidget)->eventOccured(&ev);
  }

  void HScrollList::arrangeWidgets()
  {
  	updateShownWidgets();
    if (getNumberOfChilds() == 0) return;
    
    requestDraw();
    checkFocus();
    
    float height = getSortingAxe(getSize());
    float heightOfKnownChilds = 0.0f;
    for (int i = 0; i < getNumberOfVisibleChilds(); ++i) {
      Widget *child = getVisibleChild(i);
      if (!child->getPreferedSize().is_zero()) {
        heightOfKnownChilds += getSortingAxe(child->getPreferedSize());
      }
    }
    float heightPerChild = (height - heightOfKnownChilds) / (getNumberOfVisibleChilds());
    float heightToRemove = 0.0;
    if (heightPerChild < GameUIDefaults::SPACING)
    {
      if (height >= GameUIDefaults::SPACING * getNumberOfVisibleChilds())
      {
        heightToRemove = heightOfKnownChilds - (height - GameUIDefaults::SPACING * getNumberOfVisibleChilds());
        heightPerChild = GameUIDefaults::SPACING;
      }
      else
      {
        heightPerChild = (GameUIDefaults::SPACING * getNumberOfVisibleChilds() - height) / getNumberOfVisibleChilds();
        heightToRemove = heightOfKnownChilds - (height - heightPerChild * getNumberOfVisibleChilds());
      }
    }

    Vec3 size     = getSize();
    setSortingAxe(size, heightPerChild);
    Vec3 position = getPosition();
    setSortingAxe(position, getSortingAxe(position)-heightPerChild/2.0);
    float axePos  = getSortingAxe(position);

    for (int j = 0; j<getNumberOfChilds(); j++) {
      int i = (j+firstvisible)%getNumberOfChilds();
      Widget *child = getChild(i);
      if (isItemVisible(i)) {
      if (!child->getPreferedSize().is_zero()) {
        // center the widget if we know its size
        float coeff = getSortingAxe(child->getPreferedSize()) / heightOfKnownChilds;
        Vec3 csize = size - child->getPreferedSize();
        Vec3 offset(0.0,0.0,0.0);
        setSortingAxe(offset, heightPerChild - getSortingAxe(csize / 2.0));
        Vec3 sizeOffset(0.0,0.0,0.0);
        setSortingAxe(sizeOffset,  heightToRemove*coeff);
        Vec3 cpos  = position + csize / 2.0 + offset;
        child->setSize(child->getPreferedSize()-sizeOffset);
        child->setPosition(cpos);
        axePos += getSortingAxe(child->getSize());
      } else {
        // else give him all the space he want.
        child->setSize(size);
        Vec3 offset(0.0,0.0,0.0);
        setSortingAxe(offset, heightPerChild - getSortingAxe(size / 2.0));
        child->setPosition(position+ offset);
      }
      axePos += heightPerChild;
      setSortingAxe(position, axePos);
      child->show();
      }
      else
      {
      	  child->hide();
      }
    }
  }


  //
  // Screen
  //

  Screen::Screen(float x, float y, float width, float height, GameLoop *loop)
    : DrawableComponent(),
      IdleComponent(),
      rootContainer(loop), bg(NULL)
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
    //rootContainer.checkFocus();
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
    rootContainer.giveFocus(); /* Utile mais mouaif */
    requestDraw();
  }

  void Screen::giveFocus()
  {
    rootContainer.giveFocus();
  }

  void Screen::focus(Widget *w)
  {
    rootContainer.focus(w);
  }

  //
  // Text
  // 

  Text::Text()
    : label(""), mdontMove(true)
  {
      this->font = GameUIDefaults::FONT_TEXT;
      setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
      offsetX = offsetY = 0.;
  }
    
  Text::Text(const String &label, SoFont *font)
    : font(font), label(label), mdontMove(true)
    {
      if (font == NULL) this->font = GameUIDefaults::FONT_TEXT;
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
    if (isVisible()) SoFont_PutString(font, screen, (int)(offsetX + getPosition().x), (int)(offsetY + getPosition().y), (const char*)label, NULL);
  }

  void Text::idle(double currentTime)
  {
    if (mdontMove) return;
    offsetX = 5. * cos(currentTime);
    offsetY = 2. * sin(currentTime);
    //offsetX = 5. * (sin(getPosition().x) + sin(getPosition().y + currentTime));
    //offsetY = 2. * (cos(getPosition().x+1) + sin(getPosition().y - currentTime * 1.1));
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
    mdontMove = false;
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
  // ControlInputWidget
  //

  void ControlInputWidget::init(SoFont *fontActive, SoFont *fontInactive)
  {
    if (fontInactive == NULL) fontInactive = GameUIDefaults::FONT_SMALL_INFO;
    if (fontActive == NULL)   fontActive = GameUIDefaults::FONT_SMALL_ACTIVE;

    this->fontActive   = fontActive;
    this->fontInactive = fontInactive;

    font = fontInactive;
    editionMode = false;

    setFocusable(true);
  }

  ControlInputWidget::ControlInputWidget(int control, bool alternate, Action *action)
    : Text("<Not set>", NULL), control(control), alternate(alternate)
    {
      char temp[255];
      init(NULL,NULL);
      getKeyName(control, alternate, temp);
      setValue(temp);
      if (action != NULL)
        setAction(ON_START, action);
    }

  void ControlInputWidget::eventOccured(GameControlEvent *event)
  {
    if (event->cursorEvent == GameControlEvent::kStart) {
      editionMode = !editionMode;
      if (editionMode == true) {
        previousValue = getValue();
        setValue("<Press>");
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
      else {
        GameControlEvent result;
        if (tryChangeControl(control, alternate, event->sdl_event, &result)) {
            char temp[255];
            getKeyName(control, alternate, temp);
            setValue(temp);
            editionMode = false;
        }
      }
    }
    else {
      if (isDirectionEvent(event))
        lostFocus();
    }
  }

  void ControlInputWidget::lostFocus() {
    Text::lostFocus();
    font = fontInactive;
    requestDraw();
  }

  void ControlInputWidget::giveFocus() {
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
  // ListWidget
  //

  ListWidget::ListWidget(int size, GameLoop *loop) : VBox(loop), size(size), used(0)
  {
    for (int i=0; i<size; ++i) {
      Button *b = new Button("---");
      b->mdontMove=true;
      VBox::add(b);
    }
    setPreferedSize(Vec3(1,(size+2) + SoFont_FontHeight(GameUIDefaults::FONT)*size, 1));
  }

  void ListWidget::set(int pos, Button *widget)
  {
    widget->mdontMove = true;
    changeChild(pos, widget);
  }

  void ListWidget::add(Button *widget)
  {
    if(used>=size) return;
    set(used++, widget);
  }

  void ListWidget::clear()
  {
    used = 0;
    for (int i=0; i<size; ++i) {
      Button *b = new Button("---");
      set(i,b);
    }
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
      stack.top()->removeFromGameLoopActive();
    }
    screen->show();
    screen->giveFocus();
    screen->addToGameLoop(screen->getGameLoop());
    stack.push(screen);
    loop->addDrawable(screen);
    loop->addIdle(screen);
  }

  void ScreenStack::pop() {
    stack.top()->removeFromGameLoopActive();
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

