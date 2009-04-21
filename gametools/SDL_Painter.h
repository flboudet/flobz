#ifndef _SDL_PAINTER_H
#define _SDL_PAINTER_H

#define MAX_PAINT_ELTS 0x800

#include <stdlib.h>
#include "drawcontext.h"

struct DrawElt {
  IosSurface *surf;
  IosRect     rect;
  IosRect     what;
};

class SDL_Painter
{
  public:
      IosSurface *backGround;
      DrawTarget *gameScreen;

      SDL_Painter(DrawTarget *gameScreen = NULL, IosSurface *bg = NULL);
      void requestDraw(IosSurface *surf, IosRect *where);
      void requestDraw(IosSurface *surf, IosRect *what, IosRect *where);
      void draw(DrawTarget *dt);
      void redrawAll(DrawTarget *dt);
      void draw()      { draw(gameScreen); }
      void redrawAll() { redrawAll(gameScreen); }

  private:

      int nbElts;
      int nbPrev;
      DrawElt onScreenElts[MAX_PAINT_ELTS];
      DrawElt onScreenPrev[MAX_PAINT_ELTS];
      void storeScreenContent(DrawTarget *dt);
};

#endif
