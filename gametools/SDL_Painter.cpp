#include "SDL_Painter.h"

SDL_Painter::SDL_Painter(SDL_Surface *gameScreen, SDL_Surface *bg)
  : gameScreen(gameScreen), backGround(bg), nbElts(0), nbPrev(0) {}

void SDL_Painter::requestDraw(SDL_Surface *surf, SDL_Rect *where)
{
#if DEBUG
  if (nbElts >= MAX_PAINT_ELTS) {
    fprintf(stderr, "To much elements given to SDL_Painter...\n");
    exit(1);
  }
#endif
  DrawElt elt;
  elt.surf =  surf;
  elt.rect = *where;
  onScreenElts[nbElts++] = elt;
}

static inline bool isEqual(DrawElt &d1, DrawElt &d2)
{
  return (d1.surf == d2.surf)   
    && (d1.rect.x == d2.rect.x)
    && (d1.rect.y == d2.rect.y)
    && (d1.rect.w == d2.rect.w)
    && (d1.rect.h == d2.rect.h);
}

static inline bool isEqual(SDL_Rect &r1, SDL_Rect &r2)
{
  return (r1.x == r2.x)
    && (r1.y == r2.y)
    && (r1.w == r2.w)
    && (r1.h == r2.h);
}

static inline bool isInside(SDL_Rect &r1, SDL_Rect &r2)
{
  return (r1.x > r2.x) && (r1.x + r1.w < r2.x + r2.w)
      && (r1.y > r2.y) && (r1.y + r1.h < r2.y + r2.h);
}

void SDL_Painter::draw(SDL_Surface *surf)
{
  SDL_Rect rectToUpdate1[MAX_PAINT_ELTS]; // liste brute des rectangles.
  SDL_Rect rectToUpdate2[MAX_PAINT_ELTS]; // liste videe des rect en trop.
  int nbRects1 = 0;
  int nbRects2 = 0;

  bool findMatchElts[MAX_PAINT_ELTS];
  bool findMatchPrev[MAX_PAINT_ELTS];

  for (int i=0; i<nbElts; ++i)
    findMatchElts[i] = false;
  for (int j=0; j<nbPrev; ++j)
    findMatchPrev[j] = false;

  // Chercher les differences entre la liste actuelle et l'ancienne,
  // les stocker dans une liste.
  for (int i=0; i<nbElts; ++i) {
    for (int j=0; j<nbPrev; ++j) {
      if (isEqual(onScreenElts[i], onScreenPrev[j])) {
        findMatchElts[i] = true;
        findMatchPrev[j] = true;
      }
    }
  }

  // Creer la liste des rectangles a reafficher.
  for (int i=0; i<nbElts; ++i) {
    if (!findMatchElts[i])
      rectToUpdate1[nbRects1++] = onScreenElts[i].rect;
  }
  for (int j=0; j<nbPrev; ++j) {
    if (!findMatchPrev[j])
      rectToUpdate1[nbRects1++] = onScreenPrev[j].rect;
  }

  // Optimisation de la liste de rectangles.
  for (int r1=0; r1<nbRects1; ++r1) {
    bool inside = false;
    for (int r2=0; r2<nbRects1; ++r2) {
      if (r1 == r2) continue;
      if (isInside(rectToUpdate1[r1],rectToUpdate1[r2])
          || ((r1>r2) && isEqual(rectToUpdate1[r1],rectToUpdate1[r2])))
      {
        inside = true;
        break;
      }
    }
    if (!inside)
      rectToUpdate2[nbRects2++] = rectToUpdate1[r1];
  }

  // Pour chaque rectangle
  // Chercher les elements de la liste actuelle qui intersectent
  //  (question, peut-t'on assumer que SDL fait ca aussi bien que nous ?)
  // Afficher ces elements.
  for (int r=0; r<nbRects2; ++r) {
    SDL_SetClipRect(surf, &rectToUpdate2[r]);
    SDL_BlitSurface(backGround, &rectToUpdate2[r], surf, &rectToUpdate2[r]);
    for (int i=0; i<nbElts; ++i) {
      SDL_Rect rect = onScreenElts[i].rect;
      SDL_BlitSurface(onScreenElts[i].surf, NULL,
                      surf, &rect);
    }
  }

  // Draw what is necessary...
  storeScreenContent(surf);
}

void SDL_Painter::storeScreenContent(SDL_Surface *surf)
{
  if (surf != display) {
    SDL_SetClipRect(display,NULL);
    SDL_BlitSurface(surf,NULL,display,NULL);
  }
  nbPrev = nbElts;
  while(nbElts > 0) {
    nbElts --;
    onScreenPrev[nbElts] = onScreenElts[nbElts];
  }
  nbElts = 0;
}

void SDL_Painter::redrawAll(SDL_Surface *surf)
{
  SDL_SetClipRect(surf, NULL);

  // Draw everything.
  SDL_BlitSurface(backGround, NULL, surf, NULL);
  for (int i=0; i<nbElts; ++i) {
    SDL_BlitSurface(onScreenElts[i].surf, NULL,
        surf, &onScreenElts[i].rect);
  }

  // Remember what is on screen...
  storeScreenContent(surf);
} 
