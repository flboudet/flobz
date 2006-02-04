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

      virtual bool hasFocus() const { return false; }
/*      virtual void event(const GameControlEvent &event) const    { } */

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
      ~WidgetContainer();
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


  class SingleContainer : public WidgetContainer {
  };


  class Box : public WidgetContainer {
    public:
      Box(GameLoop *loop = NULL);
      void setPolicy(GameUIEnum policy);
      void arrangeWidgets();

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
      void transitionToContent(Widget *content);
      Widget * getContentWidget() const { return contentWidget; }

      void setBackground(IIM_Surface *bg) { this->bg = bg; }
      
      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }
      
    protected:
      // Implements Widget
      virtual void draw(SDL_Surface *screen);

    private:
      Widget *contentWidget;
      Widget *previousWidget;
      double slideStartTime;
      double currentTime;
      double slidingOffset;
      IIM_Surface *bg;
      bool sliding;
  };

  class Screen : public DrawableComponent, public IdleComponent {
    public:
      Screen(float x, float y, float width, float height, GameLoop *loop = NULL);
      virtual ~Screen() {}
      void setBackground(IIM_Surface *bg);
      void draw(SDL_Surface *surface);
      virtual void drawAnyway(SDL_Surface *surface);
      bool drawRequested() const { if (isVisible()) return rootContainer.drawRequested(); return false;}
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
      
    private:
      ZBox rootContainer;
      IIM_Surface *bg;
      bool hidden;
  };


  class Text : public Widget, public IdleComponent {
    public:
      Text();
      Text(const String &label, SoFont *font = NULL);
      void setValue(String value);
      String getValue() const { return label; }
      void setFont(SoFont *newFont) { font = newFont; }

      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }
      
    protected:
      void draw(SDL_Surface *screen);
      SoFont *font;
      
    private:
      String label;
      double offsetX;
      double offsetY;

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

      void eventOccured(GameControlEvent *event);

      void lostFocus();
      void giveFocus();
      
    private:
      SoFont *fontActive;
      SoFont *fontInactive;
      bool editionMode;
      String previousValue;
      void init(SoFont *fontActive, SoFont *fontInactive);
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
    EditField * getEditField() const { return editField; }
  private:
    EditField *editField;
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
  };


  // Manage a stack of screens.
  class ScreenStack
  {
    public:
      ScreenStack(GameLoop *loop = NULL);
      ~ScreenStack() {}

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
  		bool HScrollList::isItemVisible(int id);
  		Text threedotsbefore;
  		Text threedotsafter;
  	};

};

#endif

