/* file created on Wed Aug 28 15:38:28 CEST 2002 by doom */

#ifndef _scrollingtext_H
#define _scrollingtext_H

/// SCROLLING TEXT

#include "sofont.h"
#include <SDL/SDL.h>

typedef struct _SCROLLING_TEXT ScrollingText;

/// mettre NULL pour créer un objet vide.. et affecter ce qu'on veut + tard
ScrollingText *scrolling_text_new (const char *text, SoFont * font);
void    scrolling_text_set_font (ScrollingText * st, SoFont * font);
void    scrolling_text_set_text (ScrollingText * st, const char *text);

void    scrolling_text_update (ScrollingText * st, SDL_Surface * surf);
void    scrolling_text_draw (ScrollingText * st, SDL_Surface * surf, int y);

#endif /* _scrollingtext_H */
