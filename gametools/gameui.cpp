#include "gameui.h"
#include "audiomanager.h"
#include "preferences.h"

#define MIN_REPEAT_TIME 100.0
#define REPEAT_TIME 300.0

using namespace event_manager;

namespace gameui {

    GameUIEnum   GameUIDefaults::CONTAINER_POLICY = USE_MAX_SIZE;
    float        GameUIDefaults::SPACING          = 16.0f;
    IosFont      *GameUIDefaults::FONT             = NULL;
    IosFont      *GameUIDefaults::FONT_TEXT        = NULL;
    IosFont      *GameUIDefaults::FONT_INACTIVE    = NULL;
    IosFont      *GameUIDefaults::FONT_SMALL_ACTIVE= NULL;
    IosFont      *GameUIDefaults::FONT_SMALL_INFO  = NULL;
    IosFont      *GameUIDefaults::FONT_FUNNY       = NULL;
    GameLoop    *GameUIDefaults::GAME_LOOP        = new GameLoop();
    ScreenStack *GameUIDefaults::SCREEN_STACK     = new ScreenStack();
    audio_manager::Sound *GameUIDefaults::SLIDE_SOUND = NULL;

    bool isDirectionEvent(GameControlEvent *event)
    {
        if (event->cursorEvent == kUp)
            return true;
        if (event->cursorEvent == kRight)
            return true;
        if (event->cursorEvent == kLeft)
            return true;
        if (event->cursorEvent == kDown)
            return true;
        return false;
    }

    //
    // Widget
    //
    Widget::Widget(WidgetContainer *parent)
        : parent(parent), preferedSize(0,0,0), size(0,0,0),
        position(0,0,0), m_isDead(false), hidden(false), focus(false), focusable(false), _drawRequested(true), receiveUp(false)
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

    void Widget::doDraw(DrawTarget *dt)
    {
        if (_drawRequested) {
            draw(dt);
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

    Screen *Widget::getParentScreen() const
    {
        if (parent == NULL)
            return NULL;
        return parent->getParentScreen();
    }

    //
    // WidgetContainer
    //

    WidgetContainer::WidgetContainer(GameLoop *loop) : layoutSuspended(false), loop(loop), addedToGameLoop(false) {
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
        if (! layoutSuspended)
            arrangeWidgets();
        child->onWidgetAdded(this);
        child->onWidgetVisibleChanged(isVisible());
    }

    void WidgetContainer::remove (Widget *child)
    {
        childs.remove(child);

        // Ensures the removed children doesn't grabs the screen events
        Screen *childScreen = getParentScreen();
        if (childScreen != NULL)
            childScreen->ungrabEventsOnWidget(child);

        child->setParent(NULL);
        child->removeFromGameLoopActive();
        arrangeWidgets();
        child->onWidgetVisibleChanged(false);
        child->onWidgetRemoved(this);
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

    void WidgetContainer::draw(DrawTarget *dt)
    {
		int n = getNumberOfChilds();
        for (int i = 0; i < n; ++i) {
            getChild(i)->doDraw(dt);
        }
    }

    void WidgetContainer::requestDraw(bool fromParent)
    {
        if (!_drawRequested) {
			int n = getNumberOfChilds();
            for (int i = 0; i < n; ++i) {
				Widget * w = getChild(i);
                if (!w->drawRequested()) w->requestDraw(true);
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
        if (!layoutSuspended)
        {
            Widget::setSize(v3);
            arrangeWidgets();
        }
        else {
            Vec3 v3offset = v3 - position;
            int s = childs.size();
            for (int i = 0; i < s ; i++) {
                Widget * c = childs[i];
                c->setSize(c->getSize()+v3offset);
            }
            Widget::setSize(v3);
        }
    }

    void WidgetContainer::setPosition(const Vec3 &v3)
    {
        if (!layoutSuspended)
        {
            Widget::setPosition(v3);
            arrangeWidgets();
        }
        else {
            Vec3 v3offset = v3 - position;
            int s = childs.size();
            for (int i = 0; i < s ; i++) {
                Widget * c = childs[i];
                c->setPosition(c->getPosition()+v3offset);
            }
            Widget::setPosition(v3);
        }
    }

    void WidgetContainer::suspendLayout()
    {
        layoutSuspended = true;

        int s = childs.size();
        for (int i = 0; i < s ; i++) {
            childs[i]->suspendLayout();
        }
    }

    void WidgetContainer::resumeLayout()
    {
        layoutSuspended = false;

        int s = childs.size();
        for (int i = 0; i < s ; i++) {
            childs[i]->resumeLayout();
        }
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
    Box::Box(GameLoop *loop) : WidgetContainer(loop), innerMargin(0)
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
        int numZeroSizedChildren = 0;
        int numAnyChildren = getNumberOfChilds();

        if (numAnyChildren == 0) return;
        requestDraw();
        checkFocus();

        // TODO Ugly, set size of children to own size if we are a ZBox
        if (getSortingAxe(Vec3(1.0f, 2.0f, 3.0f)) == 3.0f)
        {
            for (int i = 0; i < getNumberOfChilds(); ++i) {
                Widget *child = getChild(i);
                if (child->getPreferedSize().is_zero()) child->setSize(getSize());
                child->setPosition(getPosition());
            }
            return;
        }

        // Get the total sorted prefered size of the childs of this box
        float sortedSizeOfKnownChilds = 0.0f;
        for (int i = 0; i < numAnyChildren; ++i) {
            float preferedWidgetSortedSize = getSortingAxe(getChild(i)->getPreferedSize());
            if (preferedWidgetSortedSize != 0.0f)
                sortedSizeOfKnownChilds += preferedWidgetSortedSize;
            else
                numZeroSizedChildren++;
        }
        Vec3 boxSize = getSize() - Vec3(innerMargin*2, innerMargin*2);
        if ((getSortingAxe(boxSize) < 0.0f) || (getOtherAxis(boxSize) < 0.0f))
        {
            for (int i = 0; i < numAnyChildren; ++i) {
                Widget *child = getChild(i);
                child->setSize(Vec3(0.0f,0.0f));
            }
            return;
        }

        Vec3 boxPosition = getPosition() + Vec3(innerMargin, innerMargin);

        float spaceLeft = getSortingAxe(boxSize) - sortedSizeOfKnownChilds;
        if (spaceLeft < 0.0f) spaceLeft = 0.0f;

        float spaceAllocated = 0.0f;
        if (numZeroSizedChildren > 0) {
            spaceAllocated = spaceLeft / (float)numZeroSizedChildren;
        }
        //fprintf(stderr,"%p Size %f %f Pos %f %f\n",this, boxSize.x, boxSize.y, boxPosition.x, boxPosition.y);

        // Set child sizes
        Vec3 newSize(0.0f,0.0f);
        for (int i = 0; i < numAnyChildren; ++i) {
            Widget *child = getChild(i);
            Vec3 PreferedSize = child->getPreferedSize();

            // Sorting axe
            float sortingAxePref = getSortingAxe(PreferedSize);
            if (sortingAxePref == 0.0f) { // if zero, allocate space left / nb zero sized widgets
                setSortingAxe(newSize, spaceAllocated);
            } else { // else allocate min between boxSize and requested
                float s = getSortingAxe(boxSize);
                if (s < sortingAxePref)
                    setSortingAxe(newSize, s);
                else setSortingAxe(newSize, sortingAxePref);
            }

            // Other axe
            float otherAxisPref = getOtherAxis(PreferedSize);
            if (otherAxisPref == 0.0f) { // if zero, allocate box size
                setOtherAxis(newSize, getOtherAxis(boxSize));
            } else { // else allocate min between boxSize and requested
                float s = getOtherAxis(boxSize);
                if (s < otherAxisPref)
                    setOtherAxis(newSize, s);
                else setOtherAxis(newSize, otherAxisPref);
            }

            // final size setting
            //fprintf(stderr,"%p -- %p Size %f %f Pref %f %f\n",this, child, newSize.x, newSize.y, PreferedSize.x, PreferedSize.y);
            child->setSize(newSize);
        }

        // Set the position of the child widgets
        Vec3 childPosition = boxPosition;
        float offsetPosition = 0.0f;

        if (numZeroSizedChildren == 0) { // If we have no zero sized widget
            switch (policy) {
                default:
                    fprintf(stderr,"Layout policy %d not implemented, using USE_MAX_SIZE", policy);
                    policy = USE_MAX_SIZE;
                case USE_MAX_SIZE: // dispatch everyone to fill space
                    if (numAnyChildren > 0) offsetPosition = spaceLeft / ( 2.0f * (float)(numAnyChildren) );
                    break;
                case USE_MAX_SIZE_NO_MARGIN: // dispatch everyone to fill space with no margin before or after
                    if (numAnyChildren > 1) offsetPosition = spaceLeft / ( (float)(numAnyChildren-1) );
                    break;
                case USE_MIN_SIZE: // Collate everyone and add padding before and after
                    if ((getSortingAxe(boxSize) - sortedSizeOfKnownChilds) > 0.0f)
                        setSortingAxe(childPosition, getSortingAxe(childPosition) + (getSortingAxe(boxSize) - sortedSizeOfKnownChilds)/2.0f);
                    break;
            }
        }


        // Set positions
        for (int i = 0; i < numAnyChildren; ++i) {
            Widget *child = getChild(i);
            Vec3 childSize = child->getSize();
            setOtherAxis(childPosition, getOtherAxis(boxPosition) + (getOtherAxis(boxSize) - getOtherAxis(childSize))/2.0f);
            if (policy == USE_MAX_SIZE) setSortingAxe(childPosition, getSortingAxe(childPosition) + offsetPosition);
            child->setPosition(childPosition);
            setSortingAxe(childPosition, getSortingAxe(childPosition) + getSortingAxe(childSize) + offsetPosition);
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
        }
    }

    void Box::eventOccured(GameControlEvent *event)
    {
        // If the box has no focusable child, give up the focus
        if (getNumberOfFocusableChilds() <= 0) {
            lostFocus();
            return;
        }

        // Mouse focus management
        handleMouseFocus(event);

        // Event transmission to the widget tree
        Widget *child = getChild(activeWidget);
        // If the event is a key up event and the child is not interrested, discard the event
        if ((event->isUp) && (! child->receiveUpEvents()))
            return;
        // Send the event to the active child. If the child doesn't gives up the focus, we're done.
        child->eventOccured(event);
        if (child->haveFocus()) return;

        // Keyboard focus management
        // Discard up events for the keyboard focus management
        if (event->isUp)
            return;
        handleKeyboardFocus(event);
    }

    void Box::handleMouseFocus(GameControlEvent *event)
    {
        Vec3 ref(1.0f,2.0f,3.0f);
        float axis = getSortingAxe(ref);

        // If the box is a zbox, don't perform mouse management
        if (axis == 3.0f)
            return;

        // If the event is a mouse moved event, search and focus the widget beneath the cursor
        if (event->cursorEvent == kGameMouseMoved) {
            Widget *child = getChild(activeWidget);
            for (int i = 0 ; i < this->getNumberOfChilds() ; i++) {
                Widget *wid = this->getChild(i);
                Vec3 widPosition = wid->getPosition();
                Vec3 widSize = wid->getSize();
                if ((wid->isFocusable()) && (widPosition.x < event->x) && (widPosition.y < event->y)
                    && (widPosition.x + widSize.x > event->x) && (widPosition.y + widSize.y > event->y) && (activeWidget != i)) {
                    if (child != NULL)
                        child->lostFocus();
                    activeWidget = i;
                    wid->giveFocus();
                }
            }
        }
    }

    void Box::handleKeyboardFocus(GameControlEvent *event)
    {
        bool dontrollover = false; // rollover is enabled by default (i.e., when we reach the bottom of the box, we continue at the top)
        int direction = 0; // direction of the active widget change related to the event (1: next, -1: prev, 0: unrelated)

        Vec3 ref(1.0f,2.0f,3.0f);
        float axis = getSortingAxe(ref);

        // This stuff has the following behaviour:
        //   - if this container has an ancestor that is sorted in the same way as this box (i.e., a HBox that has another HBox as parent),
        //     disables the rollover in the current box.
        //   - if this container has no ancestor sorted in the same way, let the rollover enabled.
        WidgetContainer * curParent = parent;
        while ((curParent!=NULL) && (dontrollover==false)) {
            // DANGER: parent can be a container, not a box
            if ((static_cast<Box *>(curParent)->getSortingAxe(ref)==axis) && (static_cast<Box *>(curParent)->getNumberOfFocusableChilds()>1))
                dontrollover = true;
            curParent = curParent->parent;
        }

        if (isPrevEvent(event)) direction = -1;
        else if (isNextEvent(event)) direction = 1;
        else if (isOtherDirection(event)) {
            lostFocus();
            return;
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
                    if (dontrollover) {
                        possibleNewWidget = activeWidget;
                        lostFocus();
                    }
                    else possibleNewWidget = getNumberOfChilds()-1;
                }
                if (possibleNewWidget >= getNumberOfChilds()) // roll from last to first
                {
                    if (dontrollover) {
                        possibleNewWidget = activeWidget;
                        lostFocus();
                    }
                    else possibleNewWidget = 0;
                }
            }
            while (!getChild(possibleNewWidget)->isFocusable());

            if (possibleNewWidget != activeWidget) {
	        // If the child to be focused is a box sorted in the same way as
	        // the box which has just left the focus, try to give the focus
	        // to the child wearing the same number as in the box that has
	        // just left the focus
                Widget *possibleActiveWidget = getChild(possibleNewWidget);
  	        Box *prevActiveBox = NULL;
		if ((activeWidget >= 0)
		    && (activeWidget < getNumberOfChilds())) {
		  prevActiveBox = dynamic_cast<Box *>(getChild(activeWidget));
		}
		Box *curActiveBox = dynamic_cast<Box *>(possibleActiveWidget);
		if ((prevActiveBox != NULL) && (curActiveBox != NULL)) {
		  if (prevActiveBox->getSortingAxe(ref) == curActiveBox->getSortingAxe(ref))
		    curActiveBox->setActiveWidget(prevActiveBox->activeWidget);
		}
                possibleActiveWidget->eventOccured(event);
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
        ev.cursorEvent   = kCursorNone;
        ev.keyboardEvent = kKeyboardNone;
        ev.gameEvent     = kGameNone;
        ev.isUp          = false;

        Widget *child = getChild(activeWidget);
        if (child->isDead()) return false;
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
        return event->cursorEvent == kLeft;
    }

    bool HBox::isNextEvent(GameControlEvent *event) const
    {
        return event->cursorEvent == kRight;
    }

    bool HBox::isOtherDirection(GameControlEvent *event) const
    {
        return (event->cursorEvent == kUp)
            ||   (event->cursorEvent == kDown);
    }


    //
    // VBox
    //
    bool VBox::isPrevEvent(GameControlEvent *event) const
    {
        return event->cursorEvent == kUp;
    }
    bool VBox::isNextEvent(GameControlEvent *event) const
    {
        return event->cursorEvent == kDown;
    }
    bool VBox::isOtherDirection(GameControlEvent *event) const
    {
        return (event->cursorEvent == kRight)
            ||   (event->cursorEvent == kLeft);
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
        return (event->cursorEvent == kRight)
            ||   (event->cursorEvent == kLeft)
            ||   (event->cursorEvent == kUp)
            ||   (event->cursorEvent == kDown);
    }
    void ZBox::widgetMustRedraw(Widget *wid)
    {
		/*int n = getNumberOfChilds();
        for (int i = 0; i < n; ++i) {
			Widget * w = getChild(i);
            if (!w->drawRequested()) w->requestDraw(true);
        }*/
        requestDraw(false);
    }

    void ZBox::eventOccured(GameControlEvent *event)
    {
        Box::eventOccured(event);
    }

    // SliderContainer

    SliderContainer::SliderContainer(GameLoop *loop)
        : ZBox(loop), IdleComponent()
        , m_slideSide(SLIDE_FROM_RIGHT), slidingTime(.45)
        , contentWidget(NULL), previousWidget(NULL)
        , slideStartTime(0.0), currentTime(0.0)
        , bg(NULL), sliding(false), slideout(false)
        , backgroundVisible(true)
        , m_whipSound(NULL), m_whopSound(NULL)
    {
    }

    void SliderContainer::eventOccured(GameControlEvent *event)
    {
        if ( !(sliding && slideout)) ZBox::eventOccured(event);
    }

    void SliderContainer::endSlideInside(bool inside)
    {
        sliding = false;
        suspendLayout();
        setPosition(backupedPosition);
        resumeLayout();
        requestDraw();
        onSlideInside(); // Send notification that we have slided inside
    }

    void SliderContainer::setPosition(const Vec3 &v3)
    {
        if (sliding == false) backupedPosition = v3;
        ZBox::setPosition(v3);
    }

    void SliderContainer::setSlideSide(SlideFromSide slideSide)
    {
        m_slideSide = slideSide;
    }

    void SliderContainer::draw(DrawTarget *dt)
    {
        if (bg != NULL && backgroundVisible)
        {
            IosRect rect;
            rect.x = (Sint16)getPosition().x - (bg->w - getSize().x)/2;
            rect.y = (Sint16)getPosition().y - (bg->h - getSize().y)/2;
            dt->renderCopy(bg, NULL, &rect);
        }
        ZBox::draw(dt);
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
        slideStartTime = 0.;
        previousWidget = contentWidget;
        contentWidget = content;
        if (previousWidget != NULL)
        {
            sliding = true;
            slideout= true;
            previousWidget->lostFocus();
            if (m_whopSound != NULL)
                getGameLoop()->getAudioManager()->playSound(m_whopSound);
        }
        else
        {
            if (contentWidget != NULL)
            {
                sliding = true;
                slideout= false;
                addContentWidget();
                if (m_whipSound != NULL)
                    getGameLoop()->getAudioManager()->playSound(m_whipSound);
            }
            else
            {
                onSlideOutside();
                endSlideInside(false);
            }
        }
        if (contentWidget != NULL) {
            switch (m_slideSide) {
                case SLIDE_FROM_RIGHT:
                    m_outsidePosition = 640;
                    break;
                case SLIDE_FROM_LEFT:
                    m_outsidePosition = 0 - contentWidget->getSize().x;
                    break;
                case SLIDE_FROM_TOP:
                    m_outsidePosition = 0 - contentWidget->getSize().y;
                    break;
                case SLIDE_FROM_BOTTOM:
                    m_outsidePosition = 480;
                    break;
            }
        }
        else m_outsidePosition = 640;
    }

    void SliderContainer::idle(double currentTime)
    {
        if (sliding && slideStartTime == 0)
            slideStartTime = currentTime;
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
                    if (m_whipSound != NULL)
                        getGameLoop()->getAudioManager()->playSound(m_whipSound);
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

        Vec3 pos = backupedPosition;

        double distance;
	if ((m_slideSide == SLIDE_FROM_LEFT)
            || (m_slideSide ==SLIDE_FROM_RIGHT))
	  distance = m_outsidePosition - pos.x;
	else
	  distance = m_outsidePosition - pos.y;
        if (slideout)
        {
            double stime = t*t;
            double shtime = slidingTime*slidingTime;
	    if ((m_slideSide == SLIDE_FROM_LEFT)
		|| (m_slideSide ==SLIDE_FROM_RIGHT))
	      pos.x += distance*stime/shtime;
	    else
	      pos.y += distance*stime/shtime;

        }
        else
        {
            double stime = (slidingTime-t)*(slidingTime-t);
            double shtime = slidingTime*slidingTime;
	    if ((m_slideSide == SLIDE_FROM_LEFT)
		|| (m_slideSide ==SLIDE_FROM_RIGHT))
	      pos.x += distance*stime/shtime;
	    else
	      pos.y += distance*stime/shtime;
        }
        suspendLayout();
        ZBox::setPosition(pos);
        requestDraw();
        resumeLayout();
    }

    void SliderContainer::addContentWidget()
    {
      bool s = sliding;
      sliding = false;
      onSlideOutside(); // Sends notification that we are now outside the screen, before adding the content widget
      sliding = s;
      add(contentWidget);
      Vec3 pos = Vec3(640, backupedPosition.y);
      ZBox::setPosition(pos);
      contentWidget->giveFocus();
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
        setPreferedSize(Vec3(0.0f, Text("").getPreferedSize().y));
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
        ev.cursorEvent = kStart;
        ev.gameEvent   = kGameNone;
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
        float sortedSizeOfKnownChilds = 0.0f;
        for (int i = 0; i < getNumberOfVisibleChilds(); ++i) {
            Widget *child = getVisibleChild(i);
            if (!child->getPreferedSize().is_zero()) {
                sortedSizeOfKnownChilds += getSortingAxe(child->getPreferedSize());
            }
        }
        float heightPerChild = (height - sortedSizeOfKnownChilds) / (getNumberOfVisibleChilds());
        float heightToRemove = 0.0;
        if (heightPerChild < GameUIDefaults::SPACING)
        {
            if (height >= GameUIDefaults::SPACING * getNumberOfVisibleChilds())
            {
                heightToRemove = sortedSizeOfKnownChilds - (height - GameUIDefaults::SPACING * getNumberOfVisibleChilds());
                heightPerChild = GameUIDefaults::SPACING;
            }
            else
            {
                heightPerChild = (GameUIDefaults::SPACING * getNumberOfVisibleChilds() - height) / getNumberOfVisibleChilds();
                heightToRemove = sortedSizeOfKnownChilds - (height - heightPerChild * getNumberOfVisibleChilds());
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
                    float coeff = getSortingAxe(child->getPreferedSize()) / sortedSizeOfKnownChilds;
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

    Screen::Screen(GameLoop *loop)
        : DrawableComponent(),
        IdleComponent(),
        rootContainer(this, loop), autoReleaseFlag(false)
    {
        GameLoop *curLoop = loop;
        if (loop == NULL)
            curLoop = GameUIDefaults::GAME_LOOP;
        int height = curLoop->getDrawContext()->getHeight();
        int width = curLoop->getDrawContext()->getWidth();
        initWithDimensions(0, 0, width, height);
    }

    Screen::Screen(float x, float y, float width, float height, GameLoop *loop)
        : DrawableComponent(),
        IdleComponent(),
        rootContainer(this, loop), autoReleaseFlag(false)
    {
        initWithDimensions(x, y, width, height);
    }

    void Screen::initWithDimensions(float x, float y, float width, float height)
    {
        rootContainer.setPosition(Vec3(x, y, 1.0f));
        rootContainer.setPreferedSize(Vec3(width, height, 1.0f));
        rootContainer.setSize(Vec3(width, height, 1.0f));
        rootContainer.giveFocus();
        grabEventsOnWidget(&rootContainer);
    }

    void Screen::draw(DrawTarget *dt)
    {
        if (!isVisible()) return;
        rootContainer.doDraw(dt);
    }

    void Screen::drawAnyway(DrawTarget *dt)
    {
        rootContainer.requestDraw(true);
        rootContainer.draw(dt);
    }

    void Screen::onEvent(GameControlEvent *event)
    {
        // If the screen is not visible, don't propagate events
        if (!isVisible()) return;
        // If no other widget has been grabbed, the grabbed widget is rootContainer
        Widget *grabbedWidget = m_grabbedWidgets.back();
        // If the grabbed widget doesn't have the focus, try to give it
        // (important so that whenever the grabbed widget gives up focus, it will be
        // given back to it)
        if ((! grabbedWidget->haveFocus()) && (grabbedWidget->isFocusable()))
            grabbedWidget->giveFocus();
        // Pass the event to the last grabbed widget
        grabbedWidget->eventOccured(event);
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

    void Screen::grabEventsOnWidget(Widget *widget)
    {
        m_grabbedWidgets.push_back(widget);
    }

    void Screen::ungrabEventsOnWidget(Widget *widget)
    {
        for (std::vector<Widget *>::iterator iter = m_grabbedWidgets.begin() ; iter != m_grabbedWidgets.end() ; iter++) {
            if (*iter == widget) {
                m_grabbedWidgets.erase(iter);
                break;
            }
        }
    }

    //
    // Text
    //

    Text::Text()
        : label(""), offset(0.0,0.0,0.0), m_textAlign(TEXT_LEFT_ALIGN),
          m_autoSize(true), mdontMove(true), m_slideSound(GameUIDefaults::SLIDE_SOUND)
    {
        this->font = GameUIDefaults::FONT_TEXT;
        setPreferedSize(Vec3(m_autoSize?this->font->getTextWidth(label):0.0f, this->font->getHeight(), 1.0));
        moving = false;
        startMoving = false;
    }

    Text::Text(const String &label, IosFont *font, bool autosize)
        : font(font), label(label), offset(0.0,0.0,0.0),
          m_textAlign(TEXT_LEFT_ALIGN), m_autoSize(autosize),
          mdontMove(true), m_slideSound(GameUIDefaults::SLIDE_SOUND)
    {
        if (font == NULL) this->font = GameUIDefaults::FONT_TEXT;
        setPreferedSize(Vec3(m_autoSize?this->font->getTextWidth(label):0.0f, this->font->getHeight(), 1.0));
        moving = false;
        startMoving = false;
    }

    void Text::setValue(String value)
    {
        label = value;
        requestDraw();
        if (m_autoSize)
            setPreferedSize(Vec3(font->getTextWidth(label), font->getHeight(), 1.0f));
        if (parent)
            parent->arrangeWidgets();
    }

    void Text::draw(DrawTarget *dt)
    {
#ifdef DEBUG_POSITION
      IosRect r;
      r.x = getPosition().x;
      r.y = getPosition().y;
      r.h = getSize().y;
      r.w = getSize().x;
      RGBA rectColor = {0xAA, 0xAA, 0xAA, 0xAA};
      if (r.h>0.0f && r.w>0.0f)
          dt->fillRect(&r,rectColor);
#endif
      if (isVisible())
      {
          switch (m_textAlign) {
              case TEXT_CENTERED:
                  dt->putString(font,
                                (int)(offset.x + getPosition().x + (getSize().x-(font->getTextWidth(label)))/2.0f),
                                (int)(offset.y + getPosition().y + (getSize().y-(font->getHeight()))/2.0f),
                                (const char*)label);
                  break;
              case TEXT_RIGHT_ALIGN:
                  dt->putString(font,
                                (int)(offset.x + getPosition().x + getSize().x-(font->getTextWidth(label))),
                                (int)(offset.y + getPosition().y + (getSize().y-(font->getHeight()))/2.0f),
                                (const char*)label);
                  break;
              case TEXT_LEFT_ALIGN:
                  dt->putString(font,
                                (int)(offset.x + getPosition().x),
                                (int)(offset.y + getPosition().y + (getSize().y-(font->getHeight()))/2.0f),
                                (const char*)label);
                  break;
          }
      }
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
        if (m_slideSound != NULL)
            GameUIDefaults::GAME_LOOP->getAudioManager()->playSound(m_slideSound);
    }


    //
    // Image
    //
    Image::Image() : m_image(NULL), m_focusedImage(NULL), m_invertFocusMode(false)
    {
    }

    Image::Image(IosSurface *image, ImageAlign align)
      : m_focusedImage(NULL), m_invertFocusMode(false), m_align(align)
    {
        setImage(image);
    }

    Image::~Image()
    {
        if (m_focusedImage != NULL) delete m_focusedImage;
    }

    void Image::setImage(IosSurface *image)
    {
        m_image = image;
        if (m_focusedImage != NULL) {
            delete m_focusedImage;
            m_focusedImage = NULL;
        }
        this->setPreferedSize(Vec3(m_image->w, m_image->h));
        this->setSize(Vec3(m_image->w, m_image->h));
    }

    void Image::draw(DrawTarget *dt)
    {
        IosRect dstRect;
        IosSurface *imageToDraw = m_image;
        Vec3 pos = this->getPosition();
        Vec3 size = this->getSize();
        switch (m_align) {
            case IMAGE_CENTERED:
                dstRect.x = pos.x - (imageToDraw->w - size.x)/2;
                dstRect.y = pos.y - (imageToDraw->h - size.y)/2;
                dstRect.h = imageToDraw->h; dstRect.w = imageToDraw->w;
                break;
            default:
                dstRect.x = pos.x;
                dstRect.y = pos.y;
                dstRect.h = size.y; dstRect.w = size.x;
        }
        if (haveFocus() ^ m_invertFocusMode) {
            if (m_focusedImage == NULL) {
                IIMLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getIIMLibrary();
                m_focusedImage = iimLib.shiftHSV(m_image, 0., 0., m_invertFocusMode ? -0.3 : 0.3);
            }
            imageToDraw = m_focusedImage;
        }
        dt->renderCopy(imageToDraw, NULL, &dstRect);
    }

    void Image::setInvertedFocus(bool mode)
    {
        m_invertFocusMode = mode;
        if (m_focusedImage != NULL) {
            delete m_focusedImage;
            m_focusedImage = NULL;
        }
        requestDraw();
    }

    void Image::eventOccured(GameControlEvent *event)
    {
        bool clicked = false;
        if (isDirectionEvent(event) && !event->isUp)
            lostFocus();
        if (event->cursorEvent == kStart)
            clicked = true;
        if (event->cursorEvent == kGameMouseDown) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y))
                clicked = true;
        }
        if (clicked) {
            Action *action = (event->isUp ? getAction(ON_MOUSEUP) : getAction(ON_START));
            if (action)
                action->action(this, event->isUp ? ON_MOUSEUP : ON_START, event);
        }
    }

    //
    // Button
    //

    void Button::init(IosFont *fontActive, IosFont *fontInactive)
    {
        if (fontInactive == NULL) fontInactive = GameUIDefaults::FONT_INACTIVE;
        if (fontActive == NULL)   fontActive = GameUIDefaults::FONT;

        this->fontActive   = fontActive;
        this->fontInactive = fontInactive;

        font = fontInactive;
        setFocusable(true);
        mdontMove = false;
    }

    Button::Button(const String &label, IosFont *fontActive, IosFont *fontInactive)
        : Text(label, fontInactive)
    {
        init(fontActive, fontInactive);
        setValue(label);
    }

    Button::Button(const String &label, Action *action)
        : Text(label, NULL)
    {
        init(NULL,NULL);
        setAction(ON_START, action);
        setValue(label);
    }

    void Button::eventOccured(GameControlEvent *event)
    {
        bool clicked = false;

        if (event->isUp)
            return;

        if (isDirectionEvent(event))
            lostFocus();
        if (event->cursorEvent == kStart)
            clicked = true;
        if (event->cursorEvent == kGameMouseDown) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= event->x) && (widPosition.y + widSize.y >= event->y))
                clicked = true;
        }
        if (clicked) {
//            fprintf(stderr,"clicked...\n");
            Action *action = getAction(ON_START);
            if (action)
                action->action(this, ON_START, event);
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

    void EditField::init(IosFont *fontActive, IosFont *fontInactive)
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
        : Text(defaultText, NULL, false), persistence(persistentID), editOnFocus(false)
    {
        char mytext[256];
        GetStrPreference(persistentID, mytext, defaultText);
        setValue(mytext);
        init(NULL,NULL);
    }

    EditField::EditField(const String &defaultText,  Action *action)
        : Text(defaultText, NULL, false), persistence(""), editOnFocus(false)
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
        int i, nbytes;

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
        bool shouldHandleKbdInput = true;
        if (event->isUp) {
            repeat = false;
            return;
        }
        repeat_speed = REPEAT_TIME;

        // If editOnFocus is enabled, losing our focus is a priority!
        if (editOnFocus) {
            if (isDirectionEvent(event)) {
                lostFocus();
                return;
            }
        }

        if (event->cursorEvent == kStart) {
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
            shouldHandleKbdInput = false;
        }
        else if ((event->cursorEvent == kGameMouseDown) && (editionMode == false)) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y)) {
                previousValue = getValue();
                setValue(previousValue+"_",false);
                editionMode = true;
            }
            shouldHandleKbdInput = false;
        }
        else if (editionMode == true) {
            // kBack => Cancel current entry
            if (event->cursorEvent == kBack) {
                if (!editOnFocus) {
                    setValue(previousValue, false);
                    editionMode = false;
                    event->setCaught();
                }
                repeat = false;
                shouldHandleKbdInput = false;
            }
            // Joystick entry, discarded when editOnFocus is enabled
            else if (!editOnFocus)
                shouldHandleKbdInput = !handleJoystickEdit(event);
            // Keyboard input is also supported
            if ((shouldHandleKbdInput) && (event->keyboardEvent == kKeyboardDown)) {
                char ch = 0;
                // Ascii character
                if ((event->unicodeKeySym & 0xFF80) == 0) {
                    ch = event->unicodeKeySym & 0x7F;
                }
                // International character
                else {
                    String newValue = getValue();
                    Uint16 unicode = event->unicodeKeySym;
                    char utf8[5];
                    int nchars = utf16_to_utf8(unicode, utf8);
                    printf("%d\n", nchars);
                    for (int i=0; i<nchars; ++i) {
                        newValue[newValue.length() - 1] = utf8[i];
                        newValue += "_";
                    }
                    setValue(newValue,false);
                }
                // Backspace handling
                if (event->unicodeKeySym == 0x0008) {
                    if (getValue().length() > 1) {
                        int last=getValue().length() - 2;
                        while ((getValue()[last] & 0xc0) == 0x80)
                            last--;
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
    }

    bool EditField::handleJoystickEdit(GameControlEvent *event)
    {
        static const char CHAR_ORDER[] = "/:-. _ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?@";
        bool handled = false;
        // kUp => Change last char of the entry (forward)
        if (event->cursorEvent == kUp) {
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
            handled = true;
        }
        // kDown => Change last char of the entry (downward)
        else if (event->cursorEvent == kDown) {
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
            handled = true;
        }
        // kLeft => Like Backspace
        else if (event->cursorEvent == kLeft) {
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
                handled = true;
            }
        }
        // kRight => Duplicate last char
        else if (event->cursorEvent == kRight) {
            String newValue = getValue();
            newValue[newValue.length() - 1] = newValue[newValue.length() - 2];
            newValue += "_";
            setValue(newValue,false);
            repeat = true;
            repeat_date = ios_fc::getTimeMs();
            repeatEvent = *event;
            handled = true;
        }
        return handled;
    }

    void EditField::lostFocus() {
        Text::lostFocus();
        font = fontInactive;
        if (editionMode == true && !editOnFocus)  {
            setValue(getValue().substring(0, getValue().length() - 1));
            editionMode = false;
            Action *action = getAction(ON_START);
            if (action)
              action->action();
            editionMode = false;
        }
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

    void ControlInputWidget::init(IosFont *fontActive, IosFont *fontInactive)
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

    void ControlInputWidget::cancel(GameControlEvent *event)
    {
        if (!editionMode) return;
        setValue(previousValue);
        editionMode = false;
        if (event != NULL) event->caught = true;
    }
    void ControlInputWidget::press(GameControlEvent *event)
    {
        if (editionMode) return;
        editionMode = true;
        previousValue = getValue();
        setValue("<Press>");
    }

    void ControlInputWidget::changeTo(GameControlEvent *event)
    {
#ifdef TODO
        GameControlEvent result;
        if (tryChangeControl(control, alternate, event->sdl_event, &result)) {
            char temp[255];
            getKeyName(control, alternate, temp);
            setValue(temp);
            editionMode = false;
        }
#endif
    }

    void ControlInputWidget::eventOccured(GameControlEvent *event)
    {
        // if evenement joystick et editionMode alors traiter le cas.
        if (event->isJoystick && editionMode && (event->cursorEvent != kBack)) {
            printf("XXXXX\n");
            changeTo(event);
        }
        else if (event->cursorEvent == kStart) {
            if (editionMode == false)
                press(event);
            else
                cancel(event);
        }
        else if (event->cursorEvent == kGameMouseDown) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                    && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y))
                press(event);
        }
        else if (editionMode == true) {
            if (event->cursorEvent == kBack)
                cancel(event);
            else
                changeTo(event);
        }
        else if (isDirectionEvent(event))
            lostFocus();
    }

    void ControlInputWidget::lostFocus() {
        Text::lostFocus();
        font = fontInactive;
        if (editionMode) cancel(NULL);
        requestDraw();
    }

    void ControlInputWidget::giveFocus() {
        Text::giveFocus();
        font = fontActive;
        requestDraw();
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
        setFocusable(false);
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
