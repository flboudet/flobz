#include "gameloop.h"

GameComponent::GameComponent() : _kill(false), _remove(false)
{}

GameComponent::~GameComponent()
{}

bool GameComponent::killMe() const
{
  return _kill;
}

bool GameComponent::removeMe() const
{
  return _remove;
}

void GameComponent::kill()
{
  _kill = _remove = true;
}

void GameComponent::remove()
{
  _remove = true;
}


DrawableComponent::DrawableComponent() : _drawRequested(false)
{}

bool DrawableComponent::drawRequested() const
{
  return _drawRequested;
}

void DrawableComponent::requestDraw()
{
  _drawRequested = true;
}

void DrawableComponent::doDraw()
{
  draw();
  _drawRequested = false;
}

CycledComponent::CycledComponent(double cycleTime)
{
  this->cycleTime = cycleTime;
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

void   CycledComponent::idle(double currentTime)
{
  if (cycleNumber < -0.5) {
    firstCycleTime = currentTime;
    cycleNumber = 0.0;
  }
  
  bool requireCycle = (currentTime > firstCycleTime + cycleTime * cycleNumber);

  if (requireCycle && !paused) {
    cycleNumber += 1.0;
    cycle();
  }
}

void CycledComponent::setPause(bool paused)
{
  this->paused = paused;
  if (!paused) reset();
}
  
bool CycledComponent::getPause() const
{
  return paused;
}

void CycledComponent::reset()
{
  firstCycleTime = 0.0;
  cycleNumber = -1.0;
  paused = false;
}

#define time_tolerence 0.5

bool CycledComponent::isLate(double currentTime) const
{
  if (cycleNumber < 0.0)
    return false;
  double idealTime = firstCycleTime + cycleNumber * cycleTime;
  return (currentTime > idealTime + time_tolerence);
}


GameLoop::GameLoop() {
  finished = false;
}

void GameLoop::add(GameComponent *gc)
{
  DrawableComponent *dc = dynamic_cast<DrawableComponent*>(gc);
  IdleComponent     *ic = dynamic_cast<IdleComponent*>(gc);

  components.add(gc);
  if (dc != NULL)
    drawables.add(dc);
  if (ic != NULL)
    idles.add(ic);
}

void GameLoop::run()
{
  while (!finished)
  {
    double currentTime = 0.001 * (double)SDL_GetTicks();
    idle(currentTime);
    if (!isLate(currentTime) && drawRequested())
      draw();
  }
}

void GameLoop::idle(double currentTime)
{
  for (int i = 0; i < idles.size(); ++i) {
    idles[i]->idle(currentTime);
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
    drawables[i]->doDraw();
  }
}

