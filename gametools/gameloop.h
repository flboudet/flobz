#include "ios_fc.h"
#include <SDL.h>
#include "GameControls.h"

using namespace ios_fc;

class GameComponent {
  public:
    GameComponent();
    virtual ~GameComponent();

    virtual bool killMe() const;
    virtual bool removeMe() const;

    void kill();
    void remove();

  protected:
    bool _kill;
    bool _remove;
};

class DrawableComponent : public virtual GameComponent
{
  public:
    DrawableComponent();
    bool drawRequested() const;
    void doDraw(SDL_Surface *screen) ;

  protected:
    void requestDraw();
    virtual void draw(SDL_Surface *screen) const {}

  private:
    bool _drawRequested;
};

class IdleComponent : public virtual GameComponent
{
  public:
    virtual void idle(double currentTime)         {}

    /// return true if you want the GameLoop to skip some frames.
    virtual bool isLate(double currentTime) const { return false; }
    
    /// perform some computation if you're interested in events.
    virtual void onEvent(GameControlEvent *event) {}
};


class CycledComponent : public virtual IdleComponent
{
  public:
    CycledComponent(double cycleTime);
      
    /// called 1 time every cycleTime seconds.
    virtual void cycle()             {}

    void   setCycleTime(double time);
    double getCycleTime() const;

    int    getCycleNumber() const;

    void idle(double currentTime);

    void setPause(bool paused);
    bool getPause() const;

    void reset();

    bool isLate(double currentTime) const;
    
  private:
    double cycleTime;

    double cycleNumber;
    double firstCycleTime;

    bool paused;
};


class GameLoop : public IdleComponent, DrawableComponent
{
  public:
    GameLoop();

    void add(GameComponent *gc);
    void run();

    void idle(double currentTime);
    void draw();
    
    bool drawRequested() const;
    bool isLate(double currentTime) const;

    SDL_Surface *getSurface() const { return surface; }

  protected:
    void setSurface(SDL_Surface *surface) {
      this->surface = surface;
    }

  private:
    SDL_Surface *surface;
    Vector<GameComponent>     components;
    Vector<DrawableComponent> drawables;
    Vector<IdleComponent>     idles;
    bool finished;
};