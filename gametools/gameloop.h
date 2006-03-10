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

    void idle(double currentTime);
    void draw();
    
    bool drawRequested() const;
    bool isLate(double currentTime) const;

    SDL_Surface *getSurface() const { return surface; }
    void setSurface(SDL_Surface *surface) {
      this->surface = surface;
    }

    double getCurrentTime() const {
      return 0.001 * (double)SDL_GetTicks();
    }
    
  private:
    SDL_Surface *surface;
    Vector<DrawableComponent> drawables;
    Vector<IdleComponent>     idles;
    Vector<GarbageCollectableItem> garbageCollector;
    bool finished;
};

#endif // _GAMELOOP_H

