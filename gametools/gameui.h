#ifndef GAMEUI_H
#define GAMEUI_H

#include "GameControls.h"
#include "ios_fc.h"
#include "vec3.h"
#include "sofont.h"
#include "gameloop.h"

namespace gameui {
  

  class Widget;
  class WidgetContainer;
  class BoxWidget;
  class VBoxWidget;
  class TextWidget;


  enum GameUIEnum {
    USE_MAX_SIZE,
    USE_MIN_SIZE
  };




  class Widget : public DrawableComponent {
    
      friend class WidgetContainer;
      friend class BoxWidget;
      
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
  };


  class WidgetContainer : public Widget {
    public:
      WidgetContainer(GameLoop *loop);
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


  class BoxWidget : public WidgetContainer {
    public:
      BoxWidget(GameLoop *loop);
      void setPolicy(GameUIEnum policy);
      void arrangeWidgets();

    protected:
      virtual float getSortingAxe(const Vec3 &v3) const = 0;
      virtual void setSortingAxe(Vec3 &v3, float value) = 0;

      GameUIEnum policy;
  };


  class VBoxWidget : public BoxWidget {
    public:
      VBoxWidget(GameLoop *loop) : BoxWidget(loop) {}

    protected:
      virtual float getSortingAxe(const Vec3 &v3) const        { return v3.y;  }
      virtual void  setSortingAxe(Vec3 &v3, float value)       { v3.y = value; }
  };


  class ScreenVBox : public VBoxWidget {
    public:
      ScreenVBox(GameLoop *loop, float x, float y, float width, float height);
      void setBackground(IIM_Surface *bg);
      void draw(SDL_Surface *surface) const;

    private:
      IIM_Surface *bg;
  };


  class TextWidget : public Widget {
    public:
      TextWidget(SoFont *font, const String &label);

    protected:
      void draw(SDL_Surface *screen) const;

    private:
      SoFont *font;
      String label;
  };

};

#endif

