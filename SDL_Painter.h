#ifndef _SDL_PAINTER_H
#define _SDL_PAINTER_H

#define MAX_PAINT_ELTS 0x800

#include "glSDL.h"

struct DrawElt {
  SDL_Surface *surf;
  SDL_Rect     rect;
};
      
class SDL_Painter
{
  public:
      SDL_Surface *backGround;
      SDL_Surface *gameScreen;
      SDL_Surface *display;

      SDL_Painter(SDL_Surface *gameScreen = NULL, SDL_Surface *bg = NULL);
      void requestDraw(SDL_Surface *surf, SDL_Rect *where);
      void draw(SDL_Surface *surf);
      void redrawAll(SDL_Surface *surf);
      void draw()      { draw(gameScreen); }
      void redrawAll() { redrawAll(gameScreen); }

  private:

      int nbElts;
      int nbPrev;
      DrawElt onScreenElts[MAX_PAINT_ELTS];
      DrawElt onScreenPrev[MAX_PAINT_ELTS];
      void storeScreenContent(SDL_Surface *surf);
}; 

#endif
