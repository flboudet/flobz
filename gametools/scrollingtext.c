/* file created on Wed Aug 28 15:38:28 CEST 2002 by doom */

#include "scrollingtext.h"
#include <string.h>
#include <stdlib.h>

struct _SCROLLING_TEXT
{
	char   *text;
	SoFont *font;

	int     pos;
	int     text_width;
};

static int scrolling_text_width (ScrollingText * st);

ScrollingText *
scrolling_text_new (const char *text, SoFont * font)
{
	ScrollingText *st = (ScrollingText*)malloc (sizeof (ScrollingText));

	st->text = NULL;
	scrolling_text_set_font (st, font);

	scrolling_text_set_text (st, text);

	st->text_width = scrolling_text_width (st);
	st->pos = 640;
	return st;
}

void
scrolling_text_set_font (ScrollingText * st, SoFont * font)
{
	st->font = font;
	st->text_width = scrolling_text_width (st);
}

void
scrolling_text_set_text (ScrollingText * st, const char *text)
{
	if (st->text)
		free (st->text);
	if (text) {
		st->text = (char*)malloc (strlen (text) + 1);
		strcpy (st->text, text);
	}
	else
		st->text = NULL;
	st->text_width = scrolling_text_width (st);
}

void
scrolling_text_update (ScrollingText * st, SDL_Surface * surf)
{
	if ((st->text == NULL) || (st->font == NULL))
		return;

	st->pos -= 3;
	if (st->pos < -st->text_width)
		st->pos = surf->w;
}

void
scrolling_text_draw (ScrollingText * st, SDL_Surface * surf, int y)
{
	if ((st->text == NULL) || (st->font == NULL))
		return;

	SoFont_PutString (st->font, surf, st->pos, y, st->text, NULL);
}

static int
scrolling_text_width (ScrollingText * st)
{
	if ((st->text == NULL) || (st->font == NULL))
		return 0;

	return SoFont_TextWidth (st->font, st->text) + 400;
}
