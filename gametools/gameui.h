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
  class Text;


  enum GameUIEnum {
    USE_MAX_SIZE,
    USE_MIN_SIZE
  };


  class GameUIDefaults {
    public:
      static GameUIEnum CONTAINER_POLICY;
      static float      SPACING;
      static SoFont     *FONT;
      static SoFont     *FONT_INACTIVE;
      static GameLoop   *GAME_LOOP;
  };


  class Widget : public DrawableComponent {
    
      friend class WidgetContainer;
      friend class Box;
      
    public:
      Widget(WidgetContainer *parent = NULL);
      virtual ~Widget() {}

      virtual bool drawRequested() const              { return false; }
      void draw(SDL_Surface *screen, bool force) const;

      virtual IdleComponent *getIdleComponent() const { return NULL; }

      virtual void hide()   { hidden = true;  }
      virtual void show()   { hidden = false; }

      virtual bool hasFocus() const { return false; }
      virtual void event(const GameControlEvent &event) const    { }

      virtual Vec3 getPreferedSize() const               { return preferedSize; }
      virtual void setPreferedSize(const Vec3 &v3)       { preferedSize = v3;   }

      Vec3 getPosition() const           { return position; }
      Vec3 getSize() const               { return size; }

      // default behaviour is to lost the focus
      virtual void eventOccured(GameControlEvent *event);

      virtual void giveFocus()       { focus = true;  }
      virtual void lostFocus()       { focus = false; }
      bool haveFocus() const { return focus;  }
      
    protected:
      // To be implemented on each widgets
      virtual void draw(SDL_Surface *screen) const { };

      
      virtual void setPosition(const Vec3 &v3)   { position = v3; }
      virtual void setSize(const Vec3 &v3)       { size     = v3; }
      virtual void setParent(WidgetContainer *p) { parent   =  p; }

    private:
      WidgetContainer *parent;
      Vec3    preferedSize;
      Vec3    size;
      Vec3    position;
      bool    hidden;
      bool    focus;
  };


  class WidgetContainer : public Widget {
    public:
      WidgetContainer(GameLoop *loop = NULL);
      void add (Widget *child);
      void remove (Widget *child);
      GameLoop *getGameLoop();
      virtual void arrangeWidgets() {}

      void setSize(const Vec3 &v3);
      void setPosition(const Vec3 &v3);

      void draw(SDL_Surface *surface) const;
      bool drawRequested() const              { return true; }

    protected:
      Widget *getChild(int i)     const { return childs[i]; }
      int     getNumberOfChilds() const { return childs.size(); }
      void    sortWidgets();

    private:
      Vector<Widget> childs;
      bool bubbleSortZ_iteration(int itNumber);
      GameLoop *loop;
  };


  class Box : public WidgetContainer {
    public:
      Box(GameLoop *loop = NULL);
      void setPolicy(GameUIEnum policy);
      void arrangeWidgets();

      void eventOccured(GameControlEvent *event);
      void giveFocus();
      void lostFocus();

    protected:
      virtual float getSortingAxe(const Vec3 &v3) const = 0;
      virtual void setSortingAxe(Vec3 &v3, float value) = 0;
      virtual bool isPrevEvent(GameControlEvent *event) const = 0;
      virtual bool isNextEvent(GameControlEvent *event) const = 0;
      virtual bool isOtherDirection(GameControlEvent *event) const = 0;

      GameUIEnum policy;
      int        activeWidget;

    private:
      void setActiveWidget(int i);
      bool giveFocusToActiveWidget();
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


  class ScreenVBox : public VBox, IdleComponent {
    public:
      ScreenVBox(float x, float y, float width, float height, GameLoop *loop = NULL);
      void setBackground(IIM_Surface *bg);
      void draw(SDL_Surface *surface) const;
      void onEvent(GameControlEvent *event);

    private:
      IIM_Surface *bg;
  };


  class Text : public Widget {
    public:
      Text(const String &label, SoFont *font = NULL);

    protected:
      void draw(SDL_Surface *screen) const;
      SoFont *font;
      
    private:
      String label;
  };


  class Button : public Text {
    public:
      Button(const String &label, SoFont *fontActive = NULL, SoFont *fontInactive = NULL);

      void eventOccured(GameControlEvent *event);

      void lostFocus();
      void giveFocus();
      
    private:
      SoFont *fontActive;
      SoFont *fontInactive;
  };


  class Separator : public Widget {
    public:
      Separator(float width, float height);
  };

};

#endif

