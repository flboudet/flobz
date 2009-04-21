#include "SDL_Painter.h"
#include <stdlib.h>

#undef DEBUG


SDL_Painter::SDL_Painter(DrawTarget *gameScreen, IosSurface *bg)
  : backGround(bg), gameScreen(gameScreen), nbElts(0), nbPrev(0) {}

void SDL_Painter::requestDraw(IosSurface *surf, IosRect *where)
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
  elt.what.x = 0;
  elt.what.y = 0;
  elt.what.w = surf->w;
  elt.what.h = surf->h;
  onScreenElts[nbElts++] = elt;
}

void SDL_Painter::requestDraw(IosSurface *surf, IosRect *what, IosRect *where)
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
  elt.what = *what;
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

static inline bool isEqual(const IosRect &r1, const IosRect &r2)
{
  return (r1.x == r2.x)
    && (r1.y == r2.y)
    && (r1.w == r2.w)
    && (r1.h == r2.h);
}

static inline bool isInside(const IosRect &r1, const IosRect &r2)
{
  return (r1.x >= r2.x) && (r1.x + r1.w <= r2.x + r2.w)
      && (r1.y >= r2.y) && (r1.y + r1.h <= r2.y + r2.h);
}

static inline int addRectToList(IosRect rectList[MAX_PAINT_ELTS], int nbRect, const IosRect &rect)
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
        IosRect newRect = rectList[r];
        newRect.w = rect.w + rect.x - rectList[r].x;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
      if ((rectList[r].x >= rect.x) && (rectList[r].x <= rect.x + rect.w)) {
        IosRect newRect = rect;
        newRect.w = rectList[r].w + rectList[r].x - rect.x;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
    }
    // voisin vertical
    if ((rect.x == rectList[r].x) && (rect.w == rectList[r].w)) {
      if ((rect.y >= rectList[r].y) && (rect.y <= rectList[r].y + rectList[r].h)) {
        IosRect newRect = rectList[r];
        newRect.h = rect.h + rect.y - rectList[r].y;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
      if ((rectList[r].y >= rect.y) && (rectList[r].y <= rect.y + rect.h)) {
        IosRect newRect = rect;
        newRect.h = rectList[r].h + rectList[r].y - rect.y;
        rectList[r] = rectList[nbRect-1];
        return addRectToList(rectList, nbRect-1, newRect);
      }
    }
  }
  rectList[nbRect] = rect;
  return nbRect + 1;
}

void SDL_Painter::draw(DrawTarget *dt)
{
  IosRect rectToUpdate[MAX_PAINT_ELTS]; // liste des zones a reafficher.
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

  dt->setClipRect(NULL);
  dt->renderCopy(backGround, NULL, NULL);

  // Draw everything.
  for (int i=0; i<nbElts; ++i) {
    IosRect copy = onScreenElts[i].rect;
    dt->renderCopy(onScreenElts[i].surf, NULL, &copy);
  }

  for (int r=0; r<nbRects; ++r) {
    IosRect over1 = rectToUpdate[r];
    IosRect over2 = over1;
    IosRect over3 = over2;
    IosRect over4 = over3;
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
    dt->setClipRect(&rectToUpdate[r]);
    dt->renderCopy(backGround, &rectToUpdate[r], &rectToUpdate[r]);
    for (int i=0; i<nbElts; ++i) {
      // Afficher ces elements.
      IosRect rect = onScreenElts[i].rect;
      IosRect what = onScreenElts[i].what;
      dt->renderCopy(onScreenElts[i].surf, &what, &rect);
    }
  }
#endif

  // Draw what is necessary...
  storeScreenContent(dt);
}

void SDL_Painter::storeScreenContent(DrawTarget *dt)
{
  /*if (surf != display) {
    SDL_SetClipRect(display,NULL);
    SDL_BlitSurface(surf,NULL,display,NULL);
  }*/
  nbPrev = nbElts;
  while(nbElts > 0) {
    nbElts --;
    onScreenPrev[nbElts] = onScreenElts[nbElts];
  }
  nbElts = 0;
}

void SDL_Painter::redrawAll(DrawTarget *dt)
{
  dt->setClipRect(NULL);

  // Draw everything.
  dt->renderCopy(backGround, NULL, NULL);
  for (int i=0; i<nbElts; ++i) {
    dt->renderCopy(onScreenElts[i].surf, NULL, &onScreenElts[i].rect);
  }

  // Remember what is on screen...
  storeScreenContent(dt);
}
