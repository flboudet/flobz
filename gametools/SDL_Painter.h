#ifndef _SDL_PAINTER_H
#define _SDL_PAINTER_H

#define MAX_PAINT_ELTS 0x800

#include "IosImgProcess.h"

struct DrawElt {
  IIM_Surface *surf;
  SDL_Rect     rect;
};
      
class SDL_Painter
{
  public:
      IIM_Surface *backGround;
      IIM_Surface *gameScreen;
/*      SDL_Surface *display; */

      SDL_Painter(IIM_Surface *gameScreen = NULL, IIM_Surface *bg = NULL);
      void requestDraw(IIM_Surface *surf, SDL_Rect *where);
      void draw(SDL_Surface *surf);
      void redrawAll(SDL_Surface *surf);
      void draw()      { draw(gameScreen->surf); }
      void redrawAll() { redrawAll(gameScreen->surf); }

  private:

      int nbElts;
      int nbPrev;
      DrawElt onScreenElts[MAX_PAINT_ELTS];
      DrawElt onScreenPrev[MAX_PAINT_ELTS];
      void storeScreenContent(SDL_Surface *surf);
}; 

#endif
