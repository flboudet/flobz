#include "SDL_Painter.h"

SDL_Painter::SDL_Painter(SDL_Surface *gameScreen, SDL_Surface *bg)
  : gameScreen(gameScreen), backGround(bg), nbElts(0), nbPrev(0) {}

void SDL_Painter::requestDraw(SDL_Surface *surf, SDL_Rect *where)
{
  DrawElt elt;
  elt.surf =  surf;
  elt.rect = *where;
  onScreenElts[nbElts++] = elt;
}

void SDL_Painter::draw()
{
  redrawAll();
  // Draw what is necessary...
#if 0 
  // Remember what is on screen...
  nbPrev = nbElts;
  while(nbElts--)
    onScreenPrev[nbElts] = onScreenElts[nbElts];
#endif
}

void SDL_Painter::redrawAll()
{
  // Draw everything.
  SDL_BlitSurface(backGround, NULL, gameScreen, NULL);
  for (int i=0; i<nbElts; ++i) {
    SDL_BlitSurface(onScreenElts[i].surf, NULL,
        gameScreen, &onScreenElts[i].rect);
  }

  // Remember what is on screen...
  nbPrev = nbElts;
  while(nbElts > 0) {
    onScreenPrev[nbElts] = onScreenElts[nbElts];
    nbElts --;
  }
  nbElts = 0;
}    
