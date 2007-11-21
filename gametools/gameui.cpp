#include "gameui.h"
#include "../audio.h"
#include "preferences.h"

#define MIN_REPEAT_TIME 100.0
#define REPEAT_TIME 300.0

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

    void Widget::hide()   { onWidgetVisibleChanged(false);  }
    void Widget::show()
    {
        onWidgetVisibleChanged(true);
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

    WidgetContainer::WidgetContainer(GameLoop *loop) : loop(loop), addedToGameLoop(false), layoutSuspended(false) {
        if (loop == NULL) this->loop = GameUIDefaults::GAME_LOOP;
        innerMargin = 10;
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
        child->onWidgetVisibleChanged(isVisible());
    }

    void WidgetContainer::remove (Widget *child)
    {
        childs.remove(child);
        child->setParent(NULL);
        child->removeFromGameLoopActive();
        arrangeWidgets();
        child->onWidgetVisibleChanged(false);
    }

    void WidgetContainer::changeChild(int i, Widget *w)
    {
        Widget *tmp = getChild(i);
        if (tmp) {
            tmp->removeFromGameLoopActive();
            tmp->onWidgetVisibleChanged(false);
        }
        childs[i] = w;
        w->setParent(this);
        if (addedToGameLoop)
            w->addToGameLoop(loop);
        arrangeWidgets();
        w->onWidgetVisibleChanged(isVisible());
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

    int WidgetContainer::getNumberOfFocusableChilds()
    {
        int n=0;
        for (int i = 0; i < getNumberOfChilds() ; i++)
            if (getChild(i)->isFocusable()) n++;
        return n;
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
        if (!layoutSuspended)
            arrangeWidgets();
    }

    void WidgetContainer::onWidgetVisibleChanged(bool visible)
    {
        for (int i = 0; i < childs.size() ; i++) {
            childs[i]->onWidgetVisibleChanged(visible);
        }
        Widget::onWidgetVisibleChanged(visible);
    }

    // 
    // Box
    // 
    Box::Box(GameLoop *loop) : WidgetContainer(loop)
    {
        setPolicy(GameUIDefaults::CONTAINER_POLICY);
        setReceiveUpEvents(true);
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
                    float height = getSortingAxe(getSize()) - innerMargin*2;

                    float heightOfKnownChilds = 0.0f;
                    for (int i = 0; i < getNumberOfChilds(); ++i) {
                        Widget *child = getChild(i);
                        if (!child->getPreferedSize().is_zero())
                            heightOfKnownChilds += getSortingAxe(child->getPreferedSize());
                    }

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
                    setSortingAxe(position, getSortingAxe(position)-heightPerChild/2.0+innerMargin);
                    float axePos  = getSortingAxe(position);

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
        if (child->isFocusable()) {
            if (activeWidget < 0) activeWidget = getNumberOfChilds() - 1;
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
            //checkFocus();
        }
    }

    void Box::eventOccured(GameControlEvent *event)
    {
        bool dontrollover = false; // rollover is enabled by default (i.e., when we reach the bottom of the box, we continue at the top)
        int direction = 0; // direction of the active widget change related to the event (1: next, -1: prev, 0: unrelated)

        // If the box has no focusable child, give up the focus
        if (getNumberOfFocusableChilds() <= 0) {
            lostFocus();
            return;
        }
        
        Vec3 ref(1.0f,2.0f,3.0f);
        float axe = getSortingAxe(ref);
        // If the event is a mouse moved event, search and focus the widget beneath the cursor
        if ((axe < 3.0f) && (event->cursorEvent == GameControlEvent::kGameMouseMoved)) {
            Widget *child = getChild(activeWidget);
            for (int i = 0 ; i < this->getNumberOfChilds() ; i++) {
                Widget *wid = this->getChild(i);
                Vec3 widPosition = wid->getPosition();
                Vec3 widSize = wid->getSize();
                if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y) && (activeWidget != i)) {
                    if (child != NULL)
                        child->lostFocus();
                    activeWidget = i;
                    wid->giveFocus();
                }
            }
            return;
        }
        
        // This stuff has the following behaviour:
        //   - if this container has an ancestor that is sorted in the same way as this box (i.e., a HBox that has another HBox as parent),
        //     disables the rollover in the current box.
        //   - if this container has no ancestor sorted in the same way, let the rollover enabled.
        WidgetContainer * curParent = parent;
        while ((curParent!=NULL) && (dontrollover==false)) {
            // DANGER: parent can be a container, not a box
            if ((static_cast<Box *>(curParent)->getSortingAxe(ref)==axe) && (static_cast<Box *>(curParent)->getNumberOfFocusableChilds()>1))
                dontrollover = true;
            curParent = curParent->parent;
        }
        // Ensures that the active widget index is not out of bounds
        if (activeWidget >= getNumberOfChilds())
            activeWidget = getNumberOfChilds() - 1;
        if (activeWidget < 0) activeWidget = 0;
        // child is the active child widget
        Widget *child = getChild(activeWidget);
        // If the event is a key up event and the child is not interrested, discard the event
        if ((event->isUp) && (! child->receiveUpEvents()))
            return;
        // Handle the case where the widget has self-destroyed
        if (!hasWidget(child)) {
            throw Exception("Truc qui ne devait pas rester !");
            return; // there are probably wiser things to do
        }
        // Send the focus to the active child. If the child doesn't gives up the focus, we're done.
        child->eventOccured(event);
        if (child->haveFocus()) return;
        // The rest of the code handles the change of active widget and the rollover
        if (isPrevEvent(event)) direction = -1;
        else
        {
            if (isNextEvent(event)) direction = 1;
            else
            {
                if (isOtherDirection(event))
                {
                    lostFocus();
                    return;
                }
            }
        }
        if (!haveFocus())
            if (isPrevEvent(event)) { activeWidget = getNumberOfChilds(); direction = -1; }
            else if (isNextEvent(event)) { activeWidget = -1; direction = 1; }
        if (direction != 0)
        {
            int possibleNewWidget = activeWidget;
            do {
                possibleNewWidget += direction;
                if (possibleNewWidget < 0) // roll from first to last
                {
                    if (dontrollover)
                    {
                        possibleNewWidget = activeWidget;
                        lostFocus();
                    }  
                    else possibleNewWidget = getNumberOfChilds()-1;
                }  
                if (possibleNewWidget >= getNumberOfChilds()) // roll from last to first
                {
                    if (dontrollover)
                    {
                        possibleNewWidget = activeWidget;
                        lostFocus();
                    }  
                    else possibleNewWidget = 0;
                }  
            }
            while (!getChild(possibleNewWidget)->isFocusable());

            if (possibleNewWidget != activeWidget)
            {
                child->lostFocus();
                getChild(possibleNewWidget)->eventOccured(event);
                setActiveWidget(possibleNewWidget);
            }  
        }
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
    }
    bool ZBox::isNextEvent(GameControlEvent *event) const
    {
        return false;
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
        for (int i = 0; i < getNumberOfChilds(); ++i) {
            getChild(i)->requestDraw(true);
        }
        requestDraw(false);
    }

    // SliderContainer

    SliderContainer::SliderContainer(GameLoop *loop)
        : IdleComponent()
        , ZBox(loop), slidingTime(.4)
        , contentWidget(NULL), previousWidget(NULL)
        , currentTime(0.0), slideStartTime(0.0)
        , sliding(false), bg(NULL)
    {
    }

    void SliderContainer::eventOccured(GameControlEvent *event)
    {
        if ( !(sliding && slideout)) ZBox::eventOccured(event);
    }

    void SliderContainer::endSlideInside(bool inside)
    {
        sliding = false;
        setPosition(Vec2(640 - getSize().x, getPosition().y));
        requestDraw();
        onSlideInside(); // Send notification that we have slided inside
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
        if (sliding && slideout)
        {
            if (previousWidget != NULL) {
                previousWidget->lostFocus();
                remove(previousWidget);
                previousWidget = NULL;
            }
            if (contentWidget != NULL)
            {
                addContentWidget();
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
        }
        else
        {
            if (contentWidget != NULL)
            {
                sliding = true;
                slideout= false;
                addContentWidget();
                AudioManager::playSound("whip.wav", .1);
            }
            else
            {
                endSlideInside(false);
            }
        }
    }

    void SliderContainer::idle(double currentTime)
    {
        this->currentTime = currentTime;
        if (!sliding) return; // only executed when the slider is animated
        double t = (currentTime - slideStartTime);

        if (t > slidingTime) // At end of sliding period
        {
            if (slideout) // If the previous widget has gone
            {
                if (previousWidget != NULL) {
                    // Remove the previous widget for good
                    remove(previousWidget);
                    previousWidget = NULL;
                }
                if (contentWidget != NULL) // there is a new widget to show
                {
                    // Then slide in the new widget
                    t = 0;
                    slideStartTime = currentTime;
                    slideout = false;
					addContentWidget();
                    AudioManager::playSound("whip.wav", .1);
                }
                else // Stop here
                {
                    endSlideInside(false);
                    return;
                }
            }
            else {
                endSlideInside(true);
                return;
            }
        }
        
        Vec2 pos = getPosition();
        pos.x = 235;

        double distance = getSize().x;
        if (slideout)
        {
            double stime = t*t;
            double shtime = slidingTime*slidingTime;
            pos.x += distance*stime/shtime;
            
        }
        else
        {
            double stime = (slidingTime-t)*(slidingTime-t);
            double shtime = slidingTime*slidingTime;
            pos.x += distance*stime/shtime;
        }
        //suspendLayout();
        setPosition(pos);
        //resumeLayout();
        requestDraw();
    }
	
	void SliderContainer::addContentWidget()
	{
        onSlideOutside(); // Sends notification that we are now outside the screen, before adding the content widget
		add(contentWidget);
	}
    
    void SliderContainer::addListener(SliderContainerListener &listener)
    {
        this->listeners.push_back(&listener);
    }
    
    void SliderContainer::onSlideOutside()
    {
        for (std::vector<SliderContainerListener *>::iterator iter = this->listeners.begin() ; iter != this->listeners.end() ; iter++) {
            (*iter)->onSlideOutside(*this);
        }
    }
    
    void SliderContainer::onSlideInside()
    {
        for (std::vector<SliderContainerListener *>::iterator iter = this->listeners.begin() ; iter != this->listeners.end() ; iter++) {
            (*iter)->onSlideInside(*this);
        }
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
        rootContainer(loop), bg(NULL), autoReleaseFlag(false)
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
    
    void Screen::onDrawableVisibleChanged(bool visible)
    {
        hidden = !visible;
        onScreenVisibleChanged(visible);
    }
    
    void Screen::onScreenVisibleChanged(bool visible)
    {
        rootContainer.onWidgetVisibleChanged(visible);
    }

    void Screen::giveFocus()
    {
        rootContainer.giveFocus();
    }

    void Screen::focus(Widget *w)
    {
        rootContainer.focus(w);
    }

    void Screen::autoRelease()
    {
        if (autoReleaseFlag)
            GameUIDefaults::GAME_LOOP->garbageCollect(this);
    }

    //
    // Text
    // 

    Text::Text()
        : label(""), mdontMove(true), offset(0.0,0.0,0.0)
    {
        this->font = GameUIDefaults::FONT_TEXT;
        setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
        moving = false;
        startMoving = false;
    }

    Text::Text(const String &label, SoFont *font)
        : font(font), label(label), mdontMove(true), offset(0.0,0.0,0.0)
    {
        if (font == NULL) this->font = GameUIDefaults::FONT_TEXT;
        setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
        moving = false;
        startMoving = false;
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
        if (isVisible()) SoFont_PutString(font, screen, (int)(offset.x + getPosition().x), (int)(offset.y + getPosition().y), (const char*)label, NULL);
        //if (moving) printf("draw (%p)\n",this);
    }

    void Text::idle(double currentTime)
    {
        static const double duration = 0.5;
        static const double bounces = 3.0;
        static const double omega = bounces * 3.1415 / duration;

        if (startMoving)
        {
            moving = true;
            startMoving = false;
            startTime = currentTime;
        }

        if (mdontMove || !moving) return;

        //printf("idle (%p)\n",this);
        double t = currentTime - startTime;

        if (t>duration)
        {
            moving = false;
            offset.x = 0.0;
        }
        else offset.x = 150.0 * sin(omega * t) * ((1.0/(t+1.0)) - (1.0/(duration+1.0)));
        requestDraw();
    }

    void Text::boing()
    {
        startMoving = true;
        AudioManager::playSound("slide.wav", .5);
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
        bool clicked = false;
        
        if (event->isUp)
            return;

        if (isDirectionEvent(event))
            lostFocus();
        if (event->cursorEvent == GameControlEvent::kStart)
            clicked = true;
        if (event->cursorEvent == GameControlEvent::kGameMouseClicked) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y))
                clicked = true;
        }
        if (clicked) {
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
        if (!haveFocus()) boing();
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
        repeat = false;

        setFocusable(true);
        setReceiveUpEvents(true);
    }

    EditField::EditField(const String &defaultText, const String &persistentID)
        : Text(defaultText, NULL), persistence(persistentID), editOnFocus(false)
    {
        char mytext[256];
        GetStrPreference(persistentID, mytext, defaultText);
        setValue(mytext);
        init(NULL,NULL);
    }

    EditField::EditField(const String &defaultText,  Action *action)
        : Text(defaultText, NULL), persistence(""), editOnFocus(false)
    {
        init(NULL,NULL);
        if (action != NULL)
            setAction(ON_START, action);
    }

    void EditField::setValue(String value, bool persistent)
    {
        Text::setValue(value);
        if (persistent && (persistence != "")) SetStrPreference(persistence, getValue());
    }

    void EditField::idle(double currentTime)
    {
        Text::idle(currentTime);

        if (repeat) {
            double t = ios_fc::getTimeMs() - repeat_date;
            if (t > repeat_speed) {
                GameControlEvent e = repeatEvent;
                double save_repeat_speed = repeat_speed;

                eventOccured(&e);

                repeat_speed = save_repeat_speed * 0.66;
                if (repeat_speed < MIN_REPEAT_TIME)
                    repeat_speed = MIN_REPEAT_TIME;
                repeat_date = ios_fc::getTimeMs();
                repeat = true;
                repeatEvent = e;
            }
        }
    }

    int utf16_to_utf8(int src, char *dest)
    {
        char byte[4];
        int i, ch, nbytes;

        if (src < 0x80) {
            nbytes = 1;
            byte[0] = src;
        } else if (src < 0x800) {
            nbytes = 2;
            byte[1] = (src & 0x3f) | 0x80;
            byte[0] = ((src << 2) & 0xcf00 | 0xc000) >> 8;
        } else {
            nbytes = 3;
            byte[2] = (src & 0x3f) | 0x80;
            byte[1] = ((src << 2) & 0x3f00 | 0x8000) >> 8;
            byte[0] = ((src << 4) & 0x3f0000 | 0xe00000) >> 16;
        }

        for (i = nbytes; i < 4; i++)
            byte[i] = 0;

        strcpy(dest, byte);
        return nbytes;
    }

    void EditField::eventOccured(GameControlEvent *event)
    {

        if (event->isUp) {
            repeat = false;
            return;
        }
        repeat_speed = REPEAT_TIME;

        if (event->cursorEvent == GameControlEvent::kStart) {
            editionMode = !editionMode;
            if (editionMode == true) {
                previousValue = getValue();
                setValue(previousValue+"_",false);
            }
            else {
                editionMode = true;
                setValue(getValue().substring(0, getValue().length() - 1));
                editionMode = false;
                Action *action = getAction(ON_START);
                if (action)
                    action->action();
            }
        }
        else if ((event->cursorEvent == GameControlEvent::kGameMouseClicked) && (editionMode == false)) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y)) {
                previousValue = getValue();
                setValue(previousValue+"_",false);
                editionMode = true;
            }
        }
        else if (editionMode == true) {
            const char CHAR_ORDER[] = "/:-. _ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            // kBack => Cancel current entry
            if (event->cursorEvent == GameControlEvent::kBack) {
                if (!editOnFocus) {
                    setValue(previousValue, false);
                    editionMode = false;
                    event->setCaught();
                }
                repeat = false;
            }
            // kUp => Change last char of the entry (forward)
            else if (event->cursorEvent == GameControlEvent::kUp) {
                String newValue = getValue();
                while (newValue.length() <= 1)
                    newValue += '_';
                char ch = newValue[newValue.length() - 2];
                int index = 0;
                while (CHAR_ORDER[index] != ch) {
                    ++index;
                    if (CHAR_ORDER[index] == '\0') {
                        index = 0;
                        break;
                    }
                }
                if (CHAR_ORDER[index+1] == '\0')
                    index = 0;
                else
                    index += 1;
                newValue[newValue.length() - 2] = CHAR_ORDER[index];
                setValue(newValue,false);
                repeat = true;
                repeat_date = ios_fc::getTimeMs();
                repeatEvent = *event;
            }
            // kDown => Change last char of the entry (downward)
            else if (event->cursorEvent == GameControlEvent::kDown) {
                String newValue = getValue();
                while (newValue.length() <= 1)
                    newValue += '_';
                char ch = newValue[newValue.length() - 2];
                int index = 0;
                while (CHAR_ORDER[index] != ch) {
                    ++index;
                    if (CHAR_ORDER[index] == '\0') {
                        index = 0;
                        break;
                    }
                }
                if (index == 0)
                    index = strlen(CHAR_ORDER) - 1;
                else
                    index -= 1;
                newValue[newValue.length() - 2] = CHAR_ORDER[index];
                setValue(newValue,false);
                repeat = true;
                repeat_date = ios_fc::getTimeMs();
                repeatEvent = *event;
            }
            // kLeft => Like Backspace
            else if (event->cursorEvent == GameControlEvent::kLeft) {
                if (getValue().length() > 1) {
                    int last=getValue().length() - 2;
#ifdef ENABLE_TTF
                    while ((getValue()[last] & 0xc0) == 0x80)
                        last--;
#endif
                    String newValue = getValue().substring(0, last);
                    newValue += "_";
                    setValue(newValue,false);
                    repeat = true;
                    repeat_date = ios_fc::getTimeMs();
                    repeatEvent = *event;
                }
            }
            // kRight => Duplicate last char
            else if (event->cursorEvent == GameControlEvent::kRight) {
                String newValue = getValue();
                newValue[newValue.length() - 1] = newValue[newValue.length() - 2];
                newValue += "_";
                setValue(newValue,false);
                repeat = true;
                repeat_date = ios_fc::getTimeMs();
                repeatEvent = *event;
            }
            // Keyboard input is also supported
            else if (event->sdl_event.type == SDL_KEYDOWN) {
                SDL_Event e = event->sdl_event;
                char ch = 0;

                if ((e.key.keysym.unicode & 0xFF80) == 0) {
                    ch = e.key.keysym.unicode & 0x7F;
                }
                else {
#ifdef ENABLE_TTF
                    String newValue = getValue();
                    Uint16 unicode = e.key.keysym.unicode;
                    char utf8[5];
                    int nchars = utf16_to_utf8(unicode, utf8);
                    printf("%d\n", nchars);
                    for (int i=0; i<nchars; ++i) {
                        newValue[newValue.length() - 1] = utf8[i];
                        newValue += "_";
                    }
                    setValue(newValue,false);
#else
                    printf("Not supported.\n");
#endif
                }

                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (getValue().length() > 1) {
                        int last=getValue().length() - 2;
#ifdef ENABLE_TTF
                        while ((getValue()[last] & 0xc0) == 0x80)
                            last--;
#endif
                        String newValue = getValue().substring(0, last);
                        newValue += "_";
                        setValue(newValue,false);
                    }
                }
                else if (ch) {
                    String newValue = getValue();
                    newValue[newValue.length() - 1] = ch;
                    newValue += "_";
                    setValue(newValue,false);
                }
                repeat = true;
                repeat_date = ios_fc::getTimeMs();
                repeatEvent = *event;
            }
        }
        else {
            if (isDirectionEvent(event))
                lostFocus();
        }
        if (editOnFocus) {
            if (isDirectionEvent(event)) {
                lostFocus();
            }
        }
    }

    void EditField::lostFocus() {
        Text::lostFocus();
        font = fontInactive;
        requestDraw();
    }

    void EditField::giveFocus() {
        if (!haveFocus()) boing();
        Text::giveFocus();
        font = fontActive;
        if (editOnFocus) {
            previousValue = getValue();
            if (!editionMode) {
                editionMode = true;
                setValue("_",false);
            }
        }
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

    EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, Action *action) : editField(new EditField(defaultValue, action)), text(label)
    {
        add(&text);
        add(editField);
    }

    EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, String persistentID) : editField(new EditField(defaultValue, persistentID)), text(label)
    {
        add(&text);
        add(editField);
    }

    EditFieldWithLabel::~EditFieldWithLabel()
    {
        delete editField;
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

    ListWidget::ListWidget(int size, GameLoop *loop) : VBox(loop), size(size), used(0), button("---")
    {
        for (int i=0; i<size; ++i) {
            button.mdontMove=true;
            VBox::add(&button);
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
        Screen *topScreen = stack.top();
        topScreen->removeFromGameLoopActive();
        topScreen->hide();
        stack.pop();
        stack.top()->show();
        stack.top()->addToGameLoop(stack.top()->getGameLoop());
        topScreen->autoRelease();
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
/* vi: set ts=4 sw=4 expandtab: */
