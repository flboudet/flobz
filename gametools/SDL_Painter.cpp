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

static inline bool isEqual(const SDL_Rect &r1, const SDL_Rect &r2)
{
  return (r1.x == r2.x)
    && (r1.y == r2.y)
    && (r1.w == r2.w)
    && (r1.h == r2.h);
}

static inline bool isInside(const SDL_Rect &r1, const SDL_Rect &r2)
{
  return (r1.x > r2.x) && (r1.x + r1.w < r2.x + r2.w)
      && (r1.y > r2.y) && (r1.y + r1.h < r2.y + r2.h);
}

static inline int addRectToList(SDL_Rect rectList[MAX_PAINT_ELTS], int nbRect, const SDL_Rect &rect)
{
	for (int r=0; r<nbRect; ++r)
  {
    // rectangle deja contenu dans un autre...
    if (isInside(rect, rectList[r]) || isEqual(rect, rectList[r]))
      return nbRect;
    if (isInside(rectList[r], rect)) {
      rectList[r] = rect;
      return nbRect;
    }
    // rectangle colle a un autre: on etend l'autre.
    if ((rect.y == rectList[r].y) && (rect.h == rectList[r].h)) { // voisin horizontal
      if (rect.x == rectList[r].x + rectList[r].w) {
        rectList[r].w += rect.w;
        return nbRect;
      }
      if (rectList[r].x == rect.x + rect.w) {
        rectList[r].x = rect.x;
        rectList[r].w += rect.w;
        return nbRect;
      }
    }
    if ((rect.x == rectList[r].x) && (rect.w == rectList[r].w)) { // voisin vertical
      if (rect.y == rectList[r].y + rectList[r].h) {
        rectList[r].h += rect.h;
        return nbRect;
      }
      if (rectList[r].y == rect.y + rect.h) {
        rectList[r].y = rect.y;
        rectList[r].h += rect.h;
        return nbRect;
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
  for (int r=0; r<nbRects; ++r) {
    SDL_SetClipRect(surf, &rectToUpdate[r]);
    SDL_BlitSurface(backGround, &rectToUpdate[r], surf, &rectToUpdate[r]);
    for (int i=0; i<nbElts; ++i) {
      // Afficher ces elements.
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
