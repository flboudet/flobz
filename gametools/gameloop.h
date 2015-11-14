#ifndef _GAMELOOP_H
#define _GAMELOOP_H

#include "ios_fc.h"
#include "drawcontext.h"
#include "audiomanager.h"
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
    virtual void onEvent(event_manager::GameControlEvent *event) {}

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
    virtual ~CycledComponent();

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
    bool *_deleteToken;
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
    void setEventManager(event_manager::EventManager *em) { m_em = em; }
    event_manager::EventManager *getEventManager() const { return m_em; }
    void setAudioManager(audio_manager::AudioManager *am) { m_am = am; }
    audio_manager::AudioManager *getAudioManager() const { return m_am; }

    void addDrawable(DrawableComponent *gc);
    void addIdle(IdleComponent *gc);
    void removeDrawable(DrawableComponent *gc);
    void removeIdle(IdleComponent *gc);
    void garbageCollect(GarbageCollectableItem *item);
    void garbageCollectNow(); // run garbage collector.
    void run();

    // Reordering of drawable elements
    bool moveToFront(DrawableComponent *gc);
    bool moveToBack(DrawableComponent *gc);

    void idle(double currentTime);
    void draw(bool flip = true);

    bool drawRequested() const;
    bool isLate(double currentTime) const;

    static inline double getCurrentTime() {
		return ios_fc::getUnixTime();
    }

  private:
    DrawContext *m_dc;
    event_manager::EventManager *m_em;
    audio_manager::AudioManager *m_am;

    double timeDrift;
    double lastDrawTime, deltaDrawTimeMax;

    Vector<DrawableComponent> drawables;
    Vector<IdleComponent>     idles;
    Vector<GarbageCollectableItem> garbageCollector;
    bool finished;
};

#endif // _GAMELOOP_H
