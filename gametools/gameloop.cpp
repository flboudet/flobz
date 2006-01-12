#include "gameloop.h"

#include "gameui.h"

DrawableComponent::DrawableComponent(GameLoop *parentLoop)
  : _drawRequested(false)
{
  if (parentLoop != NULL)
    parentLoop->addDrawable(this);
}

DrawableComponent::~DrawableComponent()
{
  if (parentLoop != NULL)
    parentLoop->removeDrawable(this);
  parentLoop = NULL;
}

bool DrawableComponent::drawRequested() const
{
  return _drawRequested;
}

void DrawableComponent::requestDraw()
{
  _drawRequested = true;
}

void DrawableComponent::doDraw(SDL_Surface *screen)
{
  draw(screen);
  _drawRequested = false;
}

// IDLE COMPONENT

IdleComponent::IdleComponent(GameLoop *parentLoop)
  :paused(false)
{
  if (parentLoop != NULL)
    parentLoop->addIdle(this);
}

IdleComponent::~IdleComponent()
{
  if (parentLoop != NULL)
    parentLoop->removeIdle(this);
  parentLoop = NULL;
}

void IdleComponent::setPause(bool paused)
{
  this->paused = paused;
}

bool IdleComponent::getPause() const
{
  return paused;
}

// CYCLE COMPONENT

CycledComponent::CycledComponent(double cycleTime, GameLoop *parentLoop)
  : IdleComponent(parentLoop), cycleTime(cycleTime)
{
  reset();
}

void   CycledComponent::setCycleTime(double time)
{
  cycleTime = time;
  reset();
}

int CycledComponent::getCycleNumber() const
{
  return (int)cycleNumber;
}

double CycledComponent::getCycleTime() const
{
  return cycleTime;
}

#define time_tolerence 0.25

void   CycledComponent::idle(double currentTime)
{
  if (cycleNumber < -0.5) {
    firstCycleTime = currentTime;
    cycleNumber = 0.0;
  }
  
  bool requireCycle = (currentTime + time_tolerence > firstCycleTime + cycleTime * cycleNumber);

  if (requireCycle && (!paused)) {
    cycleNumber += 1.0;
    cycle();
  }
}

void CycledComponent::setPause(bool paused)
{
  this->paused = paused;
  if (!paused) reset();
}
  
void CycledComponent::reset()
{
  firstCycleTime = 0.0;
  cycleNumber    = -1.0;
  paused         = false;
}

bool CycledComponent::isLate(double currentTime) const
{
  if (cycleNumber < 0.0)
    return false;
  double idealTime = firstCycleTime + cycleNumber * cycleTime;
  return (currentTime > idealTime + time_tolerence);
}

// GAME LOOP

GameLoop::GameLoop() {
  finished = false;
}

void GameLoop::addDrawable(DrawableComponent *gc)
{
  for (int i = 0; i < drawables.size(); ++i)
    if (drawables[i] == gc) return;

  //printf("Component %x added to gameloop!\n", gc);
  drawables.add(gc);
  gc->parentLoop = this;
}

void GameLoop::addIdle(IdleComponent *gc)
{
  for (int i = 0; i < idles.size(); ++i)
    if (idles[i] == gc) return;

  //printf("Component %x added to gameloop!\n", gc);
  idles.add(gc);
  gc->parentLoop = this;
}

void GameLoop::removeDrawable(DrawableComponent *gc)
{
  for (int i = 0; i < drawables.size(); ++i) {
    if (gc == drawables[i]) {
        //printf("Drawable %d will be removed!\n", i);
      drawables[i]  = NULL;
      gc->parentLoop = NULL;
    }
  }
}

void GameLoop::removeIdle(IdleComponent *gc)
{
  for (int i = 0; i < idles.size(); ++i) {
    if (gc == idles[i]) {
        //printf("Idle %d will be removed!\n", i);
      idles[i]      = NULL;
      gc->parentLoop = NULL;
      }
  }
}

#include <unistd.h>

void GameLoop::run()
{
  static int tot_dropped = 0;
  static int tot_cycle   = 0;
  
  draw();
  while (!finished)
  {
    double currentTime = getCurrentTime();
    idle(getCurrentTime());
    if (drawRequested()) {
      if (!isLate(getCurrentTime()))
        draw();
      else
        tot_dropped ++;
    }

    tot_cycle ++;
    if  (tot_cycle == 100) {
      //printf("%d%% Frames Dropped\n", tot_dropped);
      tot_dropped = tot_cycle = 0;
    }

//    if (!isLate(getCurrentTime())) {
      SDL_Delay(10);
//      printf("DELAY()\n");
//    }
  }
}

void GameLoop::idle(double currentTime)
{
  int i, idles_size_at_start;

  idles_size_at_start = idles.size();

  //Vector<IdleComponent> idlesCpy = idles.dup();
  
  // 1- process events
  SDL_Event e;
  while (SDL_PollEvent (&e)) {
    
    GameControlEvent controlEvent;
    getControlEvent(e, &controlEvent);

    for (i=0; i < idles_size_at_start; ++i) {
      
      if (idles[i])
        idles[i]->onEvent(&controlEvent);
    }

    if (controlEvent.cursorEvent == GameControlEvent::kQuit) { // TODO: Laisser libre l'utilisateur ?
      SDL_Quit();
      exit(0);
    }
  }
  
  // 2- call idles
  for (i = 0; i < idles_size_at_start; ++i) {
    IdleComponent *gc = idles[i];
    if (gc != NULL)
      gc->callIdle(currentTime);
  }
  /*for (i = idles.size()-1; i >= 0 ; --i) {
    if (idles[i] && !idles[i]->removeMe())
      idles[i]->callIdle(currentTime);
  }*/

  // 3- check components to remove/kill
  // 3a- active remove
  for (i = 0; i<idles.size();) {
    IdleComponent *gc = idles[i];
    if (gc == NULL) {
       // printf("IDLE %d REMOVED############\n", i);
      idles.removeAt(i);
    }
    else i++;
  }
  for (i = 0; i<drawables.size();) {
    DrawableComponent *gc = drawables[i];
    if (gc == NULL) {
    //printf("DRAWABLE %d REMOVED############\n", i);
      drawables.removeAt(i);
    }
    else i++;
  }
}

bool GameLoop::drawRequested() const
{
  for (int i = 0; i < drawables.size(); ++i) {
    if (drawables[i]->drawRequested())
      return true;
  }
  return false;
}

bool GameLoop::isLate(double currentTime) const
{
  for (int i = 0; i < idles.size(); ++i) {
    if (idles[i]->isLate(currentTime))
      return true;
  }
  return false;
}

void GameLoop::draw()
{
  for (int i = 0; i < drawables.size(); ++i) {
    drawables[i]->doDraw(getSurface());
  }
  SDL_Flip(getSurface());
}

