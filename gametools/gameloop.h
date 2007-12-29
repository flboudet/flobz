#ifndef _GAMELOOP_H
#define _GAMELOOP_H

#include "ios_fc.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif
#include "GameControls.h"

using namespace ios_fc;

class GameLoop;

class DrawableComponent
{
  public:
    DrawableComponent();
    virtual ~DrawableComponent();
    virtual bool drawRequested() const;
    void doDraw(SDL_Surface *screen) ;
    // Reordering of drawable elements
    bool moveToFront();
    bool moveToBack(DrawableComponent *gc);
    
    // Notifications
    virtual void onDrawableVisibleChanged(bool visible) {}
  protected:
    GameLoop *parentLoop;
    void requestDraw();
    virtual void draw(SDL_Surface *screen) {}
    friend class GameLoop;
  private:
    bool _drawRequested;
};

class IdleComponent
{
  public:
    IdleComponent();
    virtual ~IdleComponent();
   
    void callIdle(double currentTime) { if (!paused) idle(currentTime); }
    
    virtual void idle(double currentTime)         {}

    /// return true if you want the GameLoop to skip some frames.
    virtual bool isLate(double currentTime) const { return false; }
    
    /// perform some computation if you're interested in events.
    virtual void onEvent(GameControlEvent *event) {}

    virtual void setPause(bool paused);
    bool getPause() const;

  protected:
    GameLoop *parentLoop;
    bool paused;
    friend class GameLoop;
};


class CycledComponent : public IdleComponent
{
  public:
    CycledComponent(double cycleTime);

    /// called 1 time every cycleTime seconds.
    virtual void cycle()             {}

    void   setCycleTime(double time);
    double getCycleTime() const;

    int    getCycleNumber() const;

    void idle(double currentTime);
    bool isLate(double currentTime) const;

    virtual void setPause(bool paused);
    void reset();
    
  private:
    double cycleTime;

    double cycleNumber;
    double firstCycleTime;
};

class GarbageCollectableItem {
public:
    virtual ~GarbageCollectableItem() {}
};

/**
 * The GameLoop class manages the main loop of the game
 * and schedules the drawin, timing and events of the game
 */
class GameLoop
{
  public:
    GameLoop();

    void addDrawable(DrawableComponent *gc);
    void addIdle(IdleComponent *gc);
    void removeDrawable(DrawableComponent *gc);
    void removeIdle(IdleComponent *gc);
    void garbageCollect(GarbageCollectableItem *item);
    void run();

    // Reordering of drawable elements
    bool moveToFront(DrawableComponent *gc);
    bool moveToBack(DrawableComponent *gc);
    
    void idle(double currentTime);
    void draw();
    
    bool drawRequested() const;
    bool isLate(double currentTime) const;

    SDL_Surface *getSurface() const { return surface; }
    SDL_Surface *getDisplay() const { return display; }
    void setSurface(SDL_Surface *surface) {
      this->surface = surface;
    }
    void setDisplay(SDL_Surface *surface) {
      this->display = surface;
    }

    inline double getCurrentTime() const {
      return 0.001 * (double)SDL_GetTicks();
    }
    
  private:
    double timeDrift;
    double lastDrawTime, deltaDrawTimeMax;
    SDL_Surface *surface;
    SDL_Surface *display;
    Vector<DrawableComponent> drawables;
    Vector<IdleComponent>     idles;
    Vector<GarbageCollectableItem> garbageCollector;
    bool finished;
};

#endif // _GAMELOOP_H

