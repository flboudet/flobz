#include "gameloop.h"
#include "gameui.h"

#ifdef HAVE_OPENGL
#include "SDL_opengl.h"
#endif

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
  display = NULL;
  opengl_mode = false;
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
  gc->onDrawableVisibleChanged(true);
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
      gc->onDrawableVisibleChanged(false);
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
  SDL_Surface *screen = getSurface();

  for (int i = 0; i < drawables.size(); ++i) {
    if (drawables[i] != NULL)
        drawables[i]->doDraw(screen);
    else
        printf("INVALID DRAWABLE\n");
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

  if (opengl_mode) {
#ifdef HAVE_OPENGL
      static GLuint texture = 0;
      GLenum texture_format;
      GLint  nOfColors;
      static int texture_mode = 0;

      if (texture_mode == 0) {
          char *ext = (char *)glGetString(GL_EXTENSIONS);
          if (strstr(ext, "EXT_texture_rectangle")) {
              printf("Using EXT_texture_rectangle\n");
              texture_mode = 2;
          }
          else if (strstr(ext, "NV_texture_rectangle")) {
              texture_mode = 2; 
              printf("Using NV_texture_rectangle\n");
          }
          else if (strstr(ext, "ARB_texture_non_power_of_two")) {
              texture_mode = 1;
              printf("Using ARB_texture_non_power_of_two\n");
          }
      }

      if (texture_mode == 0) {
          printf("OpenGL mode unsupported\n");
          SDL_Quit();
      }

      // get the number of channels in the SDL surface
      nOfColors = surface->format->BytesPerPixel;
      if (nOfColors == 4)     // contains an alpha channel
      {
          if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGBA;
          else texture_format = GL_BGRA;
      } else if (nOfColors == 3)     // no alpha channel
      {
          if (surface->format->Rmask == 0x000000ff) texture_format = GL_RGB;
          else texture_format = GL_BGR;
      } else {
          printf("warning: the image is not truecolor..  this will probably break\n");
          // this error should not go unhandled
      }

      // Have OpenGL generate a texture object handle for us
      if (texture == 0)
          glGenTextures(1, &texture);

      if (texture_mode == 2) {
          // Bind the texture object
          glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
          // Set the texture's stretching properties
          //glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          //glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          // Edit the texture object's image data using the information SDL_Surface gives us
          glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, nOfColors, surface->w, surface->h, 0,
                  texture_format, GL_UNSIGNED_BYTE, surface->pixels);
          glEnable(GL_TEXTURE_RECTANGLE_ARB);
      }
      if (texture_mode == 1) {
          // Bind the texture object
          glBindTexture(GL_TEXTURE_2D, texture);
          // Set the texture's stretching properties
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          // Edit the texture object's image data using the information SDL_Surface gives us
          glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                  texture_format, GL_UNSIGNED_BYTE, surface->pixels);
          glEnable(GL_TEXTURE_2D);
      }

      glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
      glViewport(0, 0, display->w, display->h);
      glMatrixMode( GL_PROJECTION );
      glLoadIdentity();
      glOrtho(0.0f, display->w, display->h, 0.0f, -1.0f, 1.0f);
      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();
      glClear(GL_COLOR_BUFFER_BIT);

      glDisable(GL_DEPTH_TEST);

      // Max texture coordinate
      int tx = 1;
      int ty = 1;
      if (texture_mode == 2) {
          // TEXTURE_RECTANGLE does not have normalized coordinates
          tx = 640;
          ty = 480;
      }

      // Scale the image without changing the 1.33 aspect ratio
      float ratio = (float)display->w / (float)display->h;
      int dx = 0;
      int dy = 0;
      int sx = 640;
      int sy = 480;

      if (ratio > 4.0 / 3.0) {
          // Window is too large, scale verticaly
          sx = display->h * 4 / 3;
          sy = display->h;
          dx = (display->w - sx) / 2;
          dy = 0;
      }
      else {
          // Window is too tall, scale horizontaly
          sx = display->w;
          sy = display->w * 3 / 4;
          dx = 0;
          dy = (display->h - sy) / 2;
      }

      // Draw the quad on screen
      glBegin(GL_QUADS);
      glTexCoord2i(0, 0);
      glVertex3i(dx, dy, 0);
      glTexCoord2i(tx, 0);
      glVertex3i(dx + sx, dy, 0);
      glTexCoord2i(tx, ty);
      glVertex3i(dx + sx, dy + sy, 0);
      glTexCoord2i(0, ty);
      glVertex3i(dx, dy + sy, 0);
      glEnd();

      SDL_GL_SwapBuffers();
#endif
  }
  else if (display != NULL) {
      SDL_Rect r;
      r.x = (display->w - 640) / 2;
      r.y = (display->h - 480) / 2;
      SDL_BlitSurface(screen, NULL, display, &r);
      SDL_Flip(display);
  }
  else {
      SDL_Flip(screen);
  }
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


