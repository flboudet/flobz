#ifndef _SDL_PAINTER_H
#define _SDL_PAINTER_H

#include <SDL/SDL.h>

class SDL_Painter
{
      struct DrawElt {
          SDL_Surface *surf;
          SDL_Rect     rect;
      };
      
  public:
      SDL_Painter(SDL_Surface *gameScreen = NULL, SDL_Surface *bg = NULL);
      void requestDraw(SDL_Surface *surf, SDL_Rect *where);
      void draw();
      void redrawAll();

      SDL_Surface *backGround;
      SDL_Surface *gameScreen;

  private:
      int nbElts;
      int nbPrev;
      DrawElt onScreenElts[0x800];
      DrawElt onScreenPrev[0x800];
}; 

#endif
