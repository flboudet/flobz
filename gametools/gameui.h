#ifndef GAMEUI_H
#define GAMEUI_H

#include <vector>
#include "GameControls.h"
#include "ios_fc.h"
#include "vec3.h"
#include "gameloop.h"

namespace gameui {

  class Widget;
  class WidgetContainer;
  class Box;
  class VBox;
  class HBox;
  class ZBox;
  class SliderContainer;
  class Text;
  class GameUIDefaults;
  class Screen;
  class Button;
  class HScrollList;
  class VScrollList;
  class Separator;
  class Action;
  class ScreenStack;


  enum GameUIEnum {
    USE_MAX_SIZE = 0,
    USE_MIN_SIZE,
    USE_MAX_SIZE_NO_MARGIN,
    ON_START,
    ON_MOUSEUP,
    GAMEUIENUM_LAST
  };

bool isDirectionEvent(event_manager::GameControlEvent *event);

  class Action {
    public:
      virtual void action() {}
      virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event) { action(); }
      virtual ~Action() {}
  };


  class GameUIDefaults {
    public:
      static GameUIEnum   CONTAINER_POLICY;
      static float        SPACING;
      static IosFont      *FONT;
      static IosFont      *FONT_INACTIVE;
      static IosFont     *FONT_TEXT;
      static IosFont      *FONT_SMALL_ACTIVE;
      static IosFont      *FONT_SMALL_INFO;
      static IosFont      *FONT_FUNNY;
      static GameLoop    *GAME_LOOP;
      static ScreenStack *SCREEN_STACK;
      static audio_manager::Sound *SLIDE_SOUND;
  };


  class Widget {

      friend class WidgetContainer;
      friend class SliderContainer;
      friend class Box;
      friend class HScrollList;

    public:
      Widget(WidgetContainer *parent = NULL);
      virtual ~Widget();
      void dead() { m_isDead = true; }
      bool isDead() const { return m_isDead; }

      virtual bool drawRequested() const { return _drawRequested; }
      virtual void requestDraw(bool fromParent = false);
      virtual void doDraw(DrawTarget *dt);

      virtual IdleComponent *getIdleComponent() { return NULL; }
      virtual void addToGameLoop(GameLoop *loop);
      virtual void removeFromGameLoopActive();

      virtual void hide();
      virtual void show();
      bool isVisible() const { return !hidden; }

      virtual Vec3 getPreferedSize() const               { return preferedSize; }
      virtual void setPreferedSize(const Vec3 &v3)       { preferedSize = v3;   }

      Vec3 getPosition() const           { return position; }
      Vec3 getSize() const               { return size; }

      // default behaviour is to lose the focus
      virtual void eventOccured(event_manager::GameControlEvent *event);

      virtual bool isFocusable() const { return focusable; }

      virtual void giveFocus()       { focus = true;  }
      virtual void lostFocus()       { focus = false; }
      bool haveFocus() const { return focus;  }

      void setAction(GameUIEnum type, Action *action) { actions[type] = action; }
      Action *getAction(GameUIEnum type)              { return actions[type];   }

      bool receiveUpEvents() const { return receiveUp; }
      void setReceiveUpEvents(bool receiveUp) { this->receiveUp = receiveUp; }
      virtual void checkFocus() {}

      virtual Screen *getParentScreen() const;

      // Notifications
      virtual void onWidgetVisibleChanged(bool visible) { hidden = !visible; }
      virtual void onWidgetAdded(WidgetContainer *parent) {}
      virtual void onWidgetRemoved(WidgetContainer *parent) {}
    protected:
      // To be implemented on each widgets
    virtual void draw(DrawTarget *dt) {     /*SDL_Rect r;
      r.x = getPosition().x;
      r.y = getPosition().y;
      r.h = getSize().y;
      r.w = getSize().x;
      SDL_FillRect(screen,&r,0x2468AC22);*/
    };


      virtual void setPosition(const Vec3 &v3)   { position = v3; }
      virtual void setSize(const Vec3 &v3)       { size     = v3; }
      virtual void setParent(WidgetContainer *p) { parent   =  p; }
      virtual void setFocusable(bool foc);

      virtual void suspendLayout() { }
      virtual void resumeLayout() { }

      WidgetContainer *parent;

    private:
      Vec3    preferedSize;
      Vec3    size;
      Vec3    position;
    protected:
      bool    m_isDead;
      bool    hidden;
    private:
      bool    focus;
      bool    focusable;
      bool _drawRequested;
      Action *actions[GAMEUIENUM_LAST];
      bool receiveUp;
  };


  class WidgetContainer : public Widget {
    public:
      WidgetContainer(GameLoop *loop = NULL);
      virtual ~WidgetContainer();
      virtual void add (Widget *child);
      virtual void remove (Widget *child);
      GameLoop *getGameLoop();
      virtual void arrangeWidgets() {}

      void setSize(const Vec3 &v3);
      void setPosition(const Vec3 &v3);

      void draw(DrawTarget *dt);
      void requestDraw(bool fromParent = false);
      virtual void widgetMustRedraw(Widget *wid) { requestDraw(); }

      virtual void addToGameLoop(GameLoop *loop);
      virtual void removeFromGameLoopActive();

      bool hasWidget(Widget *wid);
      /** Returns the number of focusable child in the child tree
       */
      int getNumberOfFocusableChilds();

      void suspendLayout();
      void resumeLayout();

      virtual void onWidgetVisibleChanged(bool visible);

      Widget *getChild(int i)     const  { return childs[i]; }
      void    changeChild(int i, Widget *w);
      int     getNumberOfChilds() const  { return childs.size(); }
    protected:
      // Accessors to private friend methods of the widget class
      void setWidgetSize(Widget *w, const Vec3 v3) const { w->setSize(v3); }
      void setWidgetPosition(Widget *w, const Vec3 v3) const { w->setPosition(v3); }
      void drawWidget(Widget *w, DrawTarget *dt) { w->draw(dt); }
      void    sortWidgets();
      bool    layoutSuspended;

    private:
      Vector<Widget> childs;
      bool bubbleSortZ_iteration(int itNumber);
      GameLoop *loop;
      bool addedToGameLoop;
  };


  /**
   * The Box class defines the behaviour of a generic box container
   * It is inherited by the different boxes containers: HBox, VBox, ZBox
   */
  class Box : public WidgetContainer {
    public:
      Box(GameLoop *loop = NULL);
      virtual ~Box() {}
      void setPolicy(GameUIEnum policy);
      void setInnerMargin(int margin) { innerMargin = margin; }
      virtual void arrangeWidgets();
      /**
       * Handles and eventually propagates an event on the widget tree of the box container
       * @param event the game event
       */
      virtual void eventOccured(event_manager::GameControlEvent *event);
      virtual void giveFocus();
      virtual void lostFocus();
      virtual void setFocusable(bool foc);
      virtual void add (Widget *child);
      void checkFocus();
      void focus(Widget *widget);
      int getActiveWidget() const { return activeWidget; }
    protected:
      virtual float getSortingAxe(const Vec3 &v3) const = 0;
      virtual float getOtherAxis(const Vec3 &v3) const = 0;
      virtual void setSortingAxe(Vec3 &v3, float value) = 0;
      virtual void setOtherAxis(Vec3 &v3, float value) = 0;
      virtual bool isPrevEvent(event_manager::GameControlEvent *event) const = 0;
      virtual bool isNextEvent(event_manager::GameControlEvent *event) const = 0;
      virtual bool isOtherDirection(event_manager::GameControlEvent *event) const = 0;

      GameUIEnum policy;
      int     innerMargin;
      int        activeWidget;

      virtual void setActiveWidget(int i);
      virtual bool giveFocusToActiveWidget();

    private:
      void handleMouseFocus(event_manager::GameControlEvent *event);
      void handleKeyboardFocus(event_manager::GameControlEvent *event);
      //Events
      Action *onRollDownAction, *onRollUpAction;
  };


  class VBox : public Box {
    public:
      VBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~VBox() {}
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.y;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.y = value; }
      virtual float getOtherAxis(const Vec3 &v3) const { return v3.x; }
      virtual void setOtherAxis(Vec3 &v3, float value) { v3.x = value; }
      bool isPrevEvent(event_manager::GameControlEvent *event) const;
      bool isNextEvent(event_manager::GameControlEvent *event) const;
      bool isOtherDirection(event_manager::GameControlEvent *event) const;
  };


  class HBox : public Box {
    public:
      HBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~HBox() {}
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.x;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.x = value; }
      virtual float getOtherAxis(const Vec3 &v3) const { return v3.y; }
      virtual void setOtherAxis(Vec3 &v3, float value) { v3.y = value; }
      bool isPrevEvent(event_manager::GameControlEvent *event) const;
      bool isNextEvent(event_manager::GameControlEvent *event) const;
      bool isOtherDirection(event_manager::GameControlEvent *event) const;
  };


  class ZBox : public Box {
    public:
      ZBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~ZBox() {}
      void widgetMustRedraw(Widget *wid);

      virtual void giveFocus() { Box::giveFocus(); }
      virtual void lostFocus() { Box::lostFocus(); }
      virtual void eventOccured(event_manager::GameControlEvent *event);
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.z;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.z = value; }
      virtual float getOtherAxis(const Vec3 &v3) const { return v3.x; }
      virtual void setOtherAxis(Vec3 &v3, float value) { v3.x = value; }
      bool isPrevEvent(event_manager::GameControlEvent *event) const;
      bool isNextEvent(event_manager::GameControlEvent *event) const;
      bool isOtherDirection(event_manager::GameControlEvent *event) const;
  };

  class SliderContainer;

  /**
   * Represents a slider notification listener
   */
  class SliderContainerListener {
  public:
      /**
       * Notify that the slider is outside of the screen, before sliding back inside
       */
      virtual void onSlideOutside(SliderContainer &slider) {}
      /**
       * Notify that the slider is inside the screen, at the end of its sliding movement
       */
      virtual void onSlideInside(SliderContainer &slider) {}

      virtual ~SliderContainerListener() {}
  };

  /**
   * Represents a slider container, ie a container which slides from one side of the screen
   * to present a content, and which can slide back to the side of the screen to change its
   * content
   */
  class SliderContainer : public ZBox, IdleComponent {
    public:
      /**
       * Constants defining from which side of the screen
       * the slider will slide in and out
       */
      enum SlideFromSide {
          SLIDE_FROM_TOP,
          SLIDE_FROM_LEFT,
          SLIDE_FROM_RIGHT,
          SLIDE_FROM_BOTTOM
      };
      SliderContainer(GameLoop *loop = NULL);
      virtual ~SliderContainer() {}
      /**
       * Slides the current widget out of the screen, then slides back with a new widget.
       * @param content  The widget to be placed inside the slider when the slider slides back
       */
      void transitionToContent(Widget *content);
      /**
       * Returns the widget contained inside the slider
       * @return  the widget contained inside the slider
       */
      Widget * getContentWidget() const { return contentWidget; }
      /**
       * Changes the background image of the slider
       * @param bg    The new background image
       */
      void setBackground(IosSurface *bg) { this->bg = bg; }
      void setBackgroundVisible(bool visible) { backgroundVisible = visible; }
      void setWhipSound(audio_manager::Sound *whip) { m_whipSound = whip; }
      void setWhopSound(audio_manager::Sound *whop) { m_whopSound = whop; }
      /**
       * Adds a new listener to the events of the SliderContainer widget
       * @param listener   the reference of the new listener object
       */
      void addListener(SliderContainerListener &listener);

      /**
       * Sets current position when not sliding or dest position
       * during slideOut noifications, or inoperant while sliding
       */
      void setPosition(const Vec3 &v3);

      /**
       * Sets the side from where the slider will slide in and out
       */
      void setSlideSide(SlideFromSide slideSide);

      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }

    protected:
      // Implements Widget
      virtual void draw(DrawTarget *dt);
      void eventOccured(event_manager::GameControlEvent *event);
      void addContentWidget();
      void endSlideInside(bool inside);

      // Notifications
      /**
       * Notify that the slider is outside of the screen, before sliding back inside
       */
      virtual void onSlideOutside();
      /**
       * Notify that the slider is inside the screen, at the end of its sliding movement
       */
      virtual void onSlideInside();

    private:
      SlideFromSide m_slideSide;
      double slidingTime;
      Widget *contentWidget;
      Widget *previousWidget;
      double slideStartTime;
      double currentTime;
      IosSurface *bg;
      bool sliding;
      bool slideout;
      Vec3 backupedPosition;
      bool backgroundVisible;
      std::vector<SliderContainerListener *> listeners;
      int m_outsidePosition;
      audio_manager::Sound *m_whipSound, *m_whopSound;
  };

  /**
   * Represents the root container of a screen
   */
  class ScreenRootContainer : public ZBox {
    public:
        ScreenRootContainer(Screen *parentScreen, GameLoop *loop = NULL) : ZBox(loop), m_parentScreen(parentScreen) {}
        virtual Screen *getParentScreen() const { return m_parentScreen; }
    private:
        Screen *m_parentScreen;
  };

  /**
   * Represents a full screen for containing widgets
   */
  class Screen : public GarbageCollectableItem, public DrawableComponent, public IdleComponent {
    public:
      Screen(float x, float y, float width, float height, GameLoop *loop = NULL);
      virtual ~Screen() {}
      void draw(DrawTarget *dt);
      virtual void drawAnyway(DrawTarget *dt);
      bool drawRequested() const { if (isVisible()) return rootContainer.drawRequested(); return false;}
      /**
       * Propagates an event on the widget tree of the screen
       * @param event the game event
       */
      virtual void onEvent(event_manager::GameControlEvent *event);
      void remove(Widget *child) { rootContainer.remove(child); }
      void add(Widget *child) { rootContainer.add(child); }
      virtual void hide() { hidden = true; onScreenVisibleChanged(isVisible()); }
      virtual void show() { hidden = false; onScreenVisibleChanged(isVisible());}
      virtual void onDrawableVisibleChanged(bool visible);
      bool isVisible() const { return !hidden; }

      virtual void addToGameLoop(GameLoop *loop) {
          rootContainer.addToGameLoop(loop);
          loop->addDrawable(this);
          loop->addIdle(this);
      }
      virtual void removeFromGameLoopActive() {
          rootContainer.removeFromGameLoopActive();
          getGameLoop()->removeDrawable(this);
          getGameLoop()->removeIdle(this);
      }

      GameLoop *getGameLoop() { return rootContainer.getGameLoop(); }
      void giveFocus();
      void focus(Widget *widget);
      ZBox *getRootContainer() { return &rootContainer; }

      void setAutoRelease(bool autoRelease) { autoReleaseFlag = autoRelease; }
      void autoRelease();

      void grabEventsOnWidget(Widget *widget);
      void ungrabEventsOnWidget(Widget *widget);

      // screen callbacks
      virtual void onScreenVisibleChanged(bool visible);

    private:
      // The root container of the screen
      ScreenRootContainer rootContainer;
      bool hidden;
      bool autoReleaseFlag;
      std::vector<Widget *> m_grabbedWidgets;
  };

  enum TextAlign {
    TEXT_CENTERED,
    TEXT_LEFT_ALIGN,
    TEXT_RIGHT_ALIGN
  };

  enum ImageAlign {
    IMAGE_CENTERED,
    IMAGE_LEFT_ALIGN,
    IMAGE_RIGHT_ALIGN
  };

  class Text : public Widget, public IdleComponent {
    public:
      Text();
      Text(const String &label, IosFont *font = NULL, bool autosize = true);
      void setTextAlign(TextAlign align) { m_textAlign = align; }
      TextAlign getTextAlign() const { return m_textAlign; }
      void setAutoSize(bool autoSize) { m_autoSize = autoSize; }
      bool getAutoSize() const { return m_autoSize; }
      void setValue(String value);
      String getValue() const { return label; }
      void setFont(IosFont *newFont) { font = newFont; }
      void boing(void);

      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }

    protected:
      void draw(DrawTarget *dt);
      IosFont *font;
      bool startMoving;

    private:
      String label;
      Vec3 offset;
      double startTime;
      bool moving;
      TextAlign m_textAlign;
      bool m_autoSize;
      audio_manager::Sound *m_slideSound;
    public:
      bool mdontMove;
  };

  class Image : public Widget {
  public:
    Image();
    Image(IosSurface *image, ImageAlign align = IMAGE_LEFT_ALIGN);
    ~Image();
    // Properties
    void setImage(IosSurface *image);
    void setAlign(ImageAlign align) { m_align = align; }
    virtual void setFocusable(bool focusable) { Widget::setFocusable(focusable); }
    void setInvertedFocus(bool mode);
    // Notifications
    virtual void onWidgetVisibleChanged(bool visible) { Widget::onWidgetVisibleChanged(visible); }
    virtual void onWidgetAdded(WidgetContainer *parent) { Widget::onWidgetAdded(parent); }
    virtual void onWidgetRemoved(WidgetContainer *parent) { Widget::onWidgetRemoved(parent); }
    virtual void eventOccured(event_manager::GameControlEvent *event);
    // Events
    void setOnStartAction(Action *onStartAction) { setAction(ON_START, onStartAction); setFocusable(true); }
    void setOnMouseUpAction(Action *onMouseUpAction) { setAction(ON_MOUSEUP, onMouseUpAction); setFocusable(true); setReceiveUpEvents(true); }
  protected:
    virtual void draw(DrawTarget *dt);
  private:
    IosSurface *m_image, *m_focusedImage;
    bool m_invertFocusMode;
    ImageAlign m_align;
  };

  class Button : public Text {
    public:
      Button(const String &label, IosFont *fontActive = NULL, IosFont *fontInactive = NULL);
      Button(const String &label, Action *action);

      void eventOccured(event_manager::GameControlEvent *event);

      void lostFocus();
      void giveFocus();
      void setFocusable(bool foc) { Text::setFocusable(foc); }
    private:
      IosFont *fontActive;
      IosFont *fontInactive;
      void init(IosFont *fontActive, IosFont *fontInactive);
  };


  class EditField : public Text {
    public:
      EditField(const String &defaultText, Action *action = NULL);
      EditField(const String &defaultText, const String &persistentID);

      void eventOccured(event_manager::GameControlEvent *event);
      bool handleJoystickEdit(event_manager::GameControlEvent *event);
      void setValue(String value, bool persistent = true);

      void lostFocus();
      void giveFocus();

	    void setEditOnFocus(bool editOnFocus) { this->editOnFocus = editOnFocus; }
      void idle(double currentTime);

    private:
      IosFont *fontActive;
      IosFont *fontInactive;
      bool editionMode;
      String persistence;
      String previousValue;
      void init(IosFont *fontActive, IosFont *fontInactive);
	  bool editOnFocus;

      // Event repeat related attributes
      bool repeat;
      double repeat_date;
      double repeat_speed;
      event_manager::GameControlEvent repeatEvent;
  };

  class ControlInputWidget : public Text {
    public:
      ControlInputWidget(int control, bool alternate, Action *action = NULL);

      void eventOccured(event_manager::GameControlEvent *event);

      void lostFocus();
      void giveFocus();

    private:
      int control;
      bool alternate;
      IosFont *fontActive;
      IosFont *fontInactive;
      bool editionMode;
      String previousValue;
      void init(IosFont *fontActive, IosFont *fontInactive);

      void press(event_manager::GameControlEvent *event);
      void cancel(event_manager::GameControlEvent *event);
      void changeTo(event_manager::GameControlEvent *event);
  };

  class ToggleButton : public Button {
  public:
    ToggleButton(const String &label, const String &offState, const String &onState, bool initialState, Action *action);
    void setToggle(bool toggleValue);
  private:
    String unmodifiedLabel, onState, offState;
  };


  class Separator : public Widget {
    public:
      Separator(float width=0., float height=0.);
  };

#ifdef DISABLED
  class ListWidget : public HBox
  {
    public:
      ListWidget(int size, IIM_Surface *downArrow, GameLoop *loop = NULL);
      void set(int pos, Button *widget);
      void add(Button *widget);
      void clear();
    protected:
        virtual void draw(SDL_Surface *screen);
    private:
      int size;
      int used;
      Button button;
      Image downButton, upButton;
      VBox scrollerBox;
      VBox listBox;
  };
#endif

  // Manage a stack of screens.
  class ScreenStack
  {
    public:
      ScreenStack(GameLoop *loop = NULL);
      virtual ~ScreenStack() {}

      void push(Screen *screen);
      void pop();
      Screen * top() const {return stack.top();}

    private:
      Stack<Screen*> stack;
      GameLoop *loop;

      void checkLoop();
  };


  class PushScreenAction : public Action
  {
    public:
      PushScreenAction(Screen *screen, ScreenStack *stack = NULL);
      virtual ~PushScreenAction() {}
      void action();

    private:
      ScreenStack *stack;
      Screen      *screen;
  };

  class PopScreenAction : public Action
  {
    public:
      PopScreenAction(ScreenStack *stack = NULL);
      virtual ~PopScreenAction() {}
      void action();

    private:
      ScreenStack *stack;
  };

    class HScrollList : public HBox {
      public:
  	    HScrollList(GameLoop *loop = NULL);
  	  protected:
  		int getNumberOfVisibleChilds(void);
  	  	Widget* getVisibleChild(int i);
  		void updateShownWidgets(void);
  		void arrangeWidgets(void);
  		void eventOccured(event_manager::GameControlEvent *event);
  		int lastvisible;
  		int firstvisible;
  		bool isItemVisible(int id);
  		Text threedotsbefore;
  		Text threedotsafter;
  	};

};

#endif

