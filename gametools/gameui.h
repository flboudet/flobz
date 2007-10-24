#ifndef GAMEUI_H
#define GAMEUI_H

#include "SDL_Painter.h"
#include "GameControls.h"
#include "ios_fc.h"
#include "vec3.h"
#include "sofont.h"
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
    ON_START,
    GAMEUIENUM_LAST
  };


  class Action {
    public:
      virtual void action() = 0;
      virtual ~Action() {}
  };
  

  class GameUIDefaults {
    public:
      static GameUIEnum   CONTAINER_POLICY;
      static float        SPACING;
      static SoFont      *FONT;
      static SoFont      *FONT_INACTIVE;
      static SoFont      *FONT_TEXT;
      static SoFont      *FONT_SMALL_ACTIVE;
      static SoFont      *FONT_SMALL_INFO;
      static GameLoop    *GAME_LOOP;
      static ScreenStack *SCREEN_STACK;
  };


  class Widget /*: public DrawableComponent*/ {
    
      friend class WidgetContainer;
      friend class SliderContainer;
      friend class Box;
      friend class HScrollList;
      
    public:
      Widget(WidgetContainer *parent = NULL);
      virtual ~Widget();

      virtual bool drawRequested() const { return _drawRequested; }
      virtual void requestDraw(bool fromParent = false);
      virtual void doDraw(SDL_Surface *screen);

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
      virtual void eventOccured(GameControlEvent *event);

      virtual bool isFocusable() const { return focusable; }
      
      virtual void giveFocus()       { focus = true;  }
      virtual void lostFocus()       { focus = false; }
      bool haveFocus() const { return focus;  }

      void setAction(GameUIEnum type, Action *action) { actions[type] = action; }
      Action *getAction(GameUIEnum type)              { return actions[type];   }
      
      bool receiveUpEvents() const { return receiveUp; }
      void setReceiveUpEvents(bool receiveUp) { this->receiveUp = receiveUp; }
      virtual void checkFocus() {}

    protected:
      // To be implemented on each widgets
      virtual void draw(SDL_Surface *screen) { };

      
      virtual void setPosition(const Vec3 &v3)   { position = v3; }
      virtual void setSize(const Vec3 &v3)       { size     = v3; }
      virtual void setParent(WidgetContainer *p) { parent   =  p; }
      virtual void setFocusable(bool foc);
      
      WidgetContainer *parent;
      
    private:
      Vec3    preferedSize;
      Vec3    size;
      Vec3    position;
    protected:
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
      void add (Widget *child);
      virtual void remove (Widget *child);
      GameLoop *getGameLoop();
      virtual void arrangeWidgets() {}

      void setSize(const Vec3 &v3);
      void setPosition(const Vec3 &v3);

      void draw(SDL_Surface *surface);
      void requestDraw(bool fromParent = false);
      virtual void widgetMustRedraw(Widget *wid) { requestDraw(); }
      
      virtual void addToGameLoop(GameLoop *loop);
      virtual void removeFromGameLoopActive();
      
      bool hasWidget(Widget *wid);
      /** Returns the number of focusable child in the child tree
       */
      int getNumberOfFocusableChilds();
    protected:
      Widget *getChild(int i)     const  { return childs[i]; }
      void    changeChild(int i, Widget *w);
      int     getNumberOfChilds() const  { return childs.size(); }
      void    sortWidgets();

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
      void arrangeWidgets();
      /**
       * Handles and eventually propagates an event on the widget tree of the box container
       * @param event the game event
       */
      void eventOccured(GameControlEvent *event);
      void giveFocus();
      void lostFocus();
      void setFocusable(bool foc);
      void add (Widget *child);
      void checkFocus();
      void focus(Widget *widget);

    protected:
      virtual float getSortingAxe(const Vec3 &v3) const = 0;
      virtual void setSortingAxe(Vec3 &v3, float value) = 0;
      virtual bool isPrevEvent(GameControlEvent *event) const = 0;
      virtual bool isNextEvent(GameControlEvent *event) const = 0;
      virtual bool isOtherDirection(GameControlEvent *event) const = 0;

      GameUIEnum policy;
      int        activeWidget;

      virtual void setActiveWidget(int i);
      virtual bool giveFocusToActiveWidget();

    private:
  };


  class VBox : public Box {
    public:
      VBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~VBox() {}
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.y;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.y = value; }
      bool isPrevEvent(GameControlEvent *event) const;
      bool isNextEvent(GameControlEvent *event) const;
      bool isOtherDirection(GameControlEvent *event) const;
  };


  class HBox : public Box {
    public:
      HBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~HBox() {}
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.x;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.x = value; }
      bool isPrevEvent(GameControlEvent *event) const;
      bool isNextEvent(GameControlEvent *event) const;
      bool isOtherDirection(GameControlEvent *event) const;
  };


  class ZBox : public Box {
    public:
      ZBox(GameLoop *loop = NULL) : Box(loop) {}
      virtual ~ZBox() {}
      void widgetMustRedraw(Widget *wid);
    protected:
      float getSortingAxe(const Vec3 &v3) const        { return v3.z;  }
      void  setSortingAxe(Vec3 &v3, float value)       { v3.z = value; }
      bool isPrevEvent(GameControlEvent *event) const;
      bool isNextEvent(GameControlEvent *event) const;
      bool isOtherDirection(GameControlEvent *event) const;
  };

  class SliderContainer : public ZBox, IdleComponent {
    public:
      SliderContainer(GameLoop *loop = NULL);
      virtual ~SliderContainer() {}
      void transitionToContent(Widget *content);
      Widget * getContentWidget() const { return contentWidget; }

      void setBackground(IIM_Surface *bg) { this->bg = bg; }
      
      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }
      
    protected:
      // Implements Widget
      virtual void draw(SDL_Surface *screen);
      void eventOccured(GameControlEvent *event);
      void endSlideInside(bool inside);
      
    private:
      Widget *contentWidget;
      Widget *previousWidget;
      double slideStartTime;
      double currentTime;
      double slidingOffset;
      IIM_Surface *bg;
      bool sliding;
      bool slideout;
  };

  /**
   * Represents a full screen for containing widgets
   */
  class Screen : public GarbageCollectableItem, public DrawableComponent, public IdleComponent {
    public:
      Screen(float x, float y, float width, float height, GameLoop *loop = NULL);
      virtual ~Screen() {}
      void setBackground(IIM_Surface *bg);
      void draw(SDL_Surface *surface);
      virtual void drawAnyway(SDL_Surface *surface);
      bool drawRequested() const { if (isVisible()) return rootContainer.drawRequested(); return false;}
      /**
       * Propagates an event on the widget tree of the screen
       * @param event the game event
       */
      void onEvent(GameControlEvent *event);
      void remove(Widget *child) { rootContainer.remove(child); }
      void add(Widget *child) { rootContainer.add(child); }
      virtual void hide() { hidden = true; }
      virtual void show() { hidden = false; }
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
      
      void setAutoRelease(bool autoRelease) { autoReleaseFlag = autoRelease; }
      void autoRelease();
      
    private:
      ZBox rootContainer;
      IIM_Surface *bg;
      bool hidden;
      bool autoReleaseFlag;
  };


  class Text : public Widget, public IdleComponent {
    public:
      Text();
      Text(const String &label, SoFont *font = NULL);
      void setValue(String value);
      String getValue() const { return label; }
      void setFont(SoFont *newFont) { font = newFont; }
      void boing(void);

      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }
      
    protected:
      void draw(SDL_Surface *screen);
      SoFont *font;
      bool startMoving;
      
    private:
      String label;
      Vec3 offset;
      double startTime;
      bool moving;

    public:
      bool mdontMove;
  };


  class Button : public Text {
    public:
      Button(const String &label, SoFont *fontActive = NULL, SoFont *fontInactive = NULL);
      Button(const String &label, Action *action);

      void eventOccured(GameControlEvent *event);

      void lostFocus();
      void giveFocus();
      
    private:
      SoFont *fontActive;
      SoFont *fontInactive;
      void init(SoFont *fontActive, SoFont *fontInactive);
  };
  
  
  class EditField : public Text {
    public:
      EditField(const String &defaultText, Action *action = NULL);
      EditField(const String &defaultText, const String &persistentID);

      void eventOccured(GameControlEvent *event);
      void setValue(String value, bool persistent = true);
      
      void lostFocus();
      void giveFocus();
	  
	  bool setEditOnFocus(bool editOnFocus) { this->editOnFocus = editOnFocus; }
      void idle(double currentTime);
      
    private:
      SoFont *fontActive;
      SoFont *fontInactive;
      bool editionMode;
      String persistence;
      String previousValue;
      void init(SoFont *fontActive, SoFont *fontInactive);
	  bool editOnFocus;

      // Event repeat related attributes
      bool repeat;
      double repeat_date;
      double repeat_speed;
      GameControlEvent repeatEvent;
  };
  
  class ControlInputWidget : public Text {
    public:
      ControlInputWidget(int control, bool alternate, Action *action = NULL);

      void eventOccured(GameControlEvent *event);

      void lostFocus();
      void giveFocus();
      
    private:
      int control;
      bool alternate;
      SoFont *fontActive;
      SoFont *fontInactive;
      bool editionMode;
      String previousValue;
      void init(SoFont *fontActive, SoFont *fontInactive);
  };


  class EditFieldWithLabel : public HBox {
  public:
    EditFieldWithLabel(String label, String defaultValue, Action *action = NULL);
    EditFieldWithLabel(String label, String defaultValue, String persistentID);
    virtual ~EditFieldWithLabel();
    EditField * getEditField() const { return editField; }
  private:
    EditField * editField;
    Text text;
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
      Separator(float width, float height);
  };

  class ListWidget : public VBox
  {
    public:
      ListWidget(int size, GameLoop *loop = NULL);
      void set(int pos, Button *widget);
      void add(Button *widget);
      void clear();

    private:
      int size;
      int used;
      Button button;
  };


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
  		void eventOccured(GameControlEvent *event);
  		int lastvisible;
  		int firstvisible;
  		bool isItemVisible(int id);
  		Text threedotsbefore;
  		Text threedotsafter;
  	};

};

#endif

