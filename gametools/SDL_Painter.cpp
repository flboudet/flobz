#include "SDL_Painter.h"
#include <stdlib.h>

// #define DEBUG

extern SDL_Surface *display;

SDL_Painter::SDL_Painter(IIM_Surface *gameScreen, IIM_Surface *bg)
  : gameScreen(gameScreen), backGround(bg), nbElts(0), nbPrev(0) {}

void SDL_Painter::requestDraw(IIM_Surface *surf, SDL_Rect *where)
{
#ifdef DEBUG
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

static inline bool isEqual(const SDL_Rect &r1, const SDL_Rect &r2)
{
  return (r1.x == r2.x)
    && (r1.y == r2.y)
    && (r1.w == r2.w)
    && (r1.h == r2.h);
}

static inline bool isInside(const SDL_Rect &r1, const SDL_Rect &r2)
{
  return (r1.x >= r2.x) && (r1.x + r1.w <= r2.x + r2.w)
      && (r1.y >= r2.y) && (r1.y + r1.h <= r2.y + r2.h);
}

static inline int addRectToList(SDL_Rect rectList[MAX_PAINT_ELTS], int nbRect, const SDL_Rect &rect)
{
  if ((rect.w <= 0) || (rect.h <= 0)) return nbRect;
  for (int r=0; r<nbRect; ++r)
  {
    // rectangle deja contenu dans un autre...
    if (isInside(rect, rectList[r]) || isEqual(rect, rectList[r]))
      return nbRect;
    // rectangle en contenant d'autre
    if (isInside(rectList[r], rect)) {
      rectList[r] = rectList[nbRect-1];
      return addRectToList(rectList, nbRect-1, rect);
    }
    // rectangle colle a un autre: on etend l'autre.
    // voisin horizontal
    if ((rect.y == rectList[r].y) && (rect.h == rectList[r].h)) {
      if ((rect.x >= rectList[r].x) && (rect.x <= rectList[r].x + rectList[r].w)) {
        SDL_Rect newRect = rectList[r];
        newRect.w = rect.w + rect.x - rectList[r].x;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
      if ((rectList[r].x >= rect.x) && (rectList[r].x <= rect.x + rect.w)) {
        SDL_Rect newRect = rect;
        newRect.w = rectList[r].w + rectList[r].x - rect.x;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
    }
    // voisin vertical
    if ((rect.x == rectList[r].x) && (rect.w == rectList[r].w)) {
      if ((rect.y >= rectList[r].y) && (rect.y <= rectList[r].y + rectList[r].h)) {
        SDL_Rect newRect = rectList[r];
        newRect.h = rect.h + rect.y - rectList[r].y;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
      if ((rectList[r].y >= rect.y) && (rectList[r].y <= rect.y + rect.h)) {
        SDL_Rect newRect = rect;
        newRect.h = rectList[r].h + rectList[r].y - rect.y;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
    }
  }
  rectList[nbRect] = rect;
  return nbRect + 1;
}

void SDL_Painter::draw(SDL_Surface *surf)
{
  SDL_Rect rectToUpdate[MAX_PAINT_ELTS]; // liste des zones a reafficher.
  int nbRects = 0;

  bool findMatchPrev[MAX_PAINT_ELTS];

  for (int j=0; j<nbPrev; ++j)
    findMatchPrev[j] = false;

  // Chercher les differences entre la liste actuelle et l'ancienne,
  // les stocker dans une liste.
  for (int i=0; i<nbElts; ++i) {
    bool findMatchElts = false;
    for (int j=0; j<nbPrev; ++j) {
      if (isEqual(onScreenElts[i], onScreenPrev[j])) {
        findMatchElts = true;
        findMatchPrev[j] = true;
      }
    }
    // Nouvel elements: ajouter une zone a reafficher.
    if (!findMatchElts)
      nbRects = addRectToList(rectToUpdate, nbRects, onScreenElts[i].rect);
  }

  // Reafficher aussi les zones des elements ayant disparus.
  for (int j=0; j<nbPrev; ++j) {
    if (!findMatchPrev[j])
      nbRects = addRectToList(rectToUpdate, nbRects, onScreenPrev[j].rect);
  }

  // Pour chaque rectangle
  // Chercher les elements de la liste actuelle qui intersectent
  //  (note: j'assume que SDL fait ca aussi bien que nous)
#ifdef DEBUG

  SDL_SetClipRect(surf, NULL);
  SDL_BlitSurface(backGround, NULL, surf, NULL);

  // Draw everything.
  for (int i=0; i<nbElts; ++i) {
    SDL_Rect copy = onScreenElts[i].rect;
    SDL_BlitSurface(onScreenElts[i].surf, NULL,
        surf, &copy);
  }

  for (int r=0; r<nbRects; ++r) {
    SDL_Rect over1 = rectToUpdate[r];
    SDL_Rect over2 = over1;
    SDL_Rect over3 = over2;
    SDL_Rect over4 = over3;
    over1.h = 1;
    over2.w = 1;
    over3.y += over3.h;
    over3.h = 1;
    over4.x += over4.w;
    over4.w = 1;
    SDL_FillRect(surf,&over1,0xffffffff);
    SDL_FillRect(surf,&over2,0xffffffff);
    SDL_FillRect(surf,&over3,0xffffffff);
    SDL_FillRect(surf,&over4,0xffffffff);
  }    

#else

  for (int r=0; r<nbRects; ++r) {
    SDL_SetClipRect(surf, &rectToUpdate[r]);
    SDL_BlitSurface(backGround->surf, &rectToUpdate[r], surf, &rectToUpdate[r]);
    for (int i=0; i<nbElts; ++i) {
      // Afficher ces elements.
      SDL_Rect rect = onScreenElts[i].rect;
      SDL_BlitSurface(onScreenElts[i].surf->surf, NULL,
                      surf, &rect);
    }
  }
#endif

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
  SDL_BlitSurface(backGround->surf, NULL, surf, NULL);
  for (int i=0; i<nbElts; ++i) {
    SDL_BlitSurface(onScreenElts[i].surf->surf, NULL,
        surf, &onScreenElts[i].rect);
  }

  // Remember what is on screen...
  storeScreenContent(surf);
} 
