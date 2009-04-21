#ifndef _GAMELOOP_H
#define _GAMELOOP_H

#include "ios_fc.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif
#include "drawcontext.h"
#include "GameControls.h"

using namespace ios_fc;

class GameLoop;

class DrawableComponent
{
  public:
    DrawableComponent();
    virtual ~DrawableComponent();
    virtual bool drawRequested() const;
    // Immediately draws the DrawableComponent
    void doDraw(DrawTarget *dt) ;
    // Reordering of drawable elements
    bool moveToFront();
    bool moveToBack(DrawableComponent *gc);

    // Notifications
    virtual void onDrawableVisibleChanged(bool visible) {}
  protected:
    GameLoop *parentLoop;
    void requestDraw();
    virtual void draw(DrawTarget *dt) {}
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

    void setDrawContext(DrawContext *dc) { m_dc = dc; }
    DrawContext *getDrawContext() const { return m_dc; }

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

    void setOpenGLMode(bool state) {
        this->opengl_mode = state;
    }

    static inline double getCurrentTime() {
      return 0.001 * (double)SDL_GetTicks();
    }

  private:
    DrawContext *m_dc;

    double timeDrift;
    double lastDrawTime, deltaDrawTimeMax;
    // obsolete
    SDL_Surface *surface;
    SDL_Surface *display;

    Vector<DrawableComponent> drawables;
    Vector<IdleComponent>     idles;
    Vector<GarbageCollectableItem> garbageCollector;
    bool finished;
    bool opengl_mode;
};

#endif // _GAMELOOP_H

