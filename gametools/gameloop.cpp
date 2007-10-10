#include "gameloop.h"

#include "gameui.h"

DrawableComponent::DrawableComponent()
  : parentLoop(NULL), _drawRequested(false)
{
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

bool DrawableComponent::moveToFront()
{
    return parentLoop->moveToFront(this);
}

bool DrawableComponent::moveToBack(DrawableComponent *gc)
{
    return parentLoop->moveToBack(this);
}

// IDLE COMPONENT

IdleComponent::IdleComponent()
  : parentLoop(NULL), paused(false)
{
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

CycledComponent::CycledComponent(double cycleTime)
  : IdleComponent(), cycleTime(cycleTime)
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

GameLoop::GameLoop() : timeDrift(0), lastDrawTime(getCurrentTime()), deltaDrawTimeMax(2.) {
  finished = false;
}

void GameLoop::addDrawable(DrawableComponent *gc)
{
  for (int i = 0; i < drawables.size(); ++i)
    if (drawables[i] == gc) return;

#ifdef DEBUG_GAMELOOP
  printf("Drawable %x added to gameloop!\n", gc);
#endif
  drawables.add(gc);
  gc->parentLoop = this;
}

void GameLoop::addIdle(IdleComponent *gc)
{
  for (int i = 0; i < idles.size(); ++i)
    if (idles[i] == gc) return;

#ifdef DEBUG_GAMELOOP
  printf("Idle %x added to gameloop!\n", gc);
#endif
  idles.add(gc);
  gc->parentLoop = this;
}

void GameLoop::removeDrawable(DrawableComponent *gc)
{
  for (int i = 0; i < drawables.size(); ++i) {
    if (gc == drawables[i]) {
#ifdef DEBUG_GAMELOOP
      printf("Drawable %x removed from gameloop!\n", gc);
#endif
      drawables[i] = NULL;
      gc->parentLoop = NULL;
    }
  }
}

void GameLoop::removeIdle(IdleComponent *gc)
{
  for (int i = 0; i < idles.size(); ++i) {
    if (gc == idles[i]) {
#ifdef DEBUG_GAMELOOP
      printf("Idle %x removed from gameloop!\n", gc);
#endif
      idles[i] = NULL;
      gc->parentLoop = NULL;
      }
  }
}

void GameLoop::garbageCollect(GarbageCollectableItem *item)
{
    garbageCollector.add(item);
}

#include <unistd.h>

void GameLoop::run()
{
  SDL_EnableUNICODE(1);
  draw();
  while (!finished)
  {
    double currentTime = getCurrentTime();
    idle(currentTime);
    if (drawRequested()) {
      currentTime = getCurrentTime();
      // Ensure at least a frame every deltaDrawTimeMax gets drawn
      if ((!isLate(currentTime)) || (currentTime - lastDrawTime > deltaDrawTimeMax)) {
        draw();
        lastDrawTime = currentTime;
      }
    }
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
      
      if (idles[i]) {
#ifdef DEBUG_GAMELOOP
	printf("onEvent called on idle %x!\n", idles[i]);
	fflush(stdout);
#endif
        idles[i]->onEvent(&controlEvent);
      }
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
  
  // 3b- garbage collector for passive remove
  while (garbageCollector.size() > 0) {
    printf("GARBAGE COLLECTABLE REMOVED\n");
    delete garbageCollector[0];
    garbageCollector.removeAt(0);
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
    if (idles[i] == NULL) continue;
    if (idles[i]->isLate(currentTime))
      return true;
  }
  return false;
}

#ifdef BENCHMARKS
#include "ios_fc.h"
#endif

void GameLoop::draw()
{

  for (int i = 0; i < drawables.size(); ++i) {
    drawables[i]->doDraw(getSurface());
  }

#ifdef BENCHMARKS
  extern SoFont *DBG_FONT;
  static double nFrames = 0.0;
  static double t0 = 0.0;
  static char fps[] = "FPS: .....   ";
  static double minFPS = 100000.0;
  static double t1 = 0.0;

  double t2 = ios_fc::getTimeMs();
  double curFPS = 1000.0 / (t2 - t1);
  if (curFPS < minFPS) minFPS = curFPS;
  t1 = t2;

  if (nFrames > 60.0) {
      sprintf(fps, "FPS: %3.1f >> %3.1f", (1000.0 * nFrames / (t2-t0)), minFPS);
      t0 = t2;
      nFrames = 0.0;
      minFPS = 1000000.0;
  }

  nFrames += 1.0;
  if (DBG_FONT != NULL)
        SoFont_PutString (DBG_FONT, getSurface(), 16, 16, fps, NULL);
#endif

  SDL_Flip(getSurface());
}

bool GameLoop::moveToFront(DrawableComponent *gc)
{
    if (drawables[drawables.size()-1] == gc)
        return false;
    removeDrawable(gc);
    addDrawable(gc);
    return true;
}

bool GameLoop::moveToBack(DrawableComponent *gc)
{
    return false;
}


