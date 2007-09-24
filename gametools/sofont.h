 /* 
  * SoFont - SDL Object Font Library
  * 
  * This library is free software; you can redistribute it and/or modify it
  * under the terms of the GNU Library General Public License as published by 
  * the Free Software Foundation; either version 2 of the License, or (at
  * your option) any later version.
  * 
  * This library is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library 
  * General Public License for more details.
  * 
  * You should have received a copy of the GNU Library General Public License 
  * along with this library; if not, write to the Free Foundation, Inc., 59
  * Temple Place, Suite 330, Boston, MA  02111-1307  USA
  * 
  * Originally from C Library: Karl Bartel
  * 
  * *    SFONT - SDL Font Library by Karl Bartel <karlb@gmx.net>            *
  * *                                                                       *
  * *  All functions are explained below. There are two versions of each    *
  * *  funtction. The first is the normal one, the function with the        *
  * *  2 at the end can be used when you want to handle more than one font  *
  * *  in your program.                                                     *
  * *                                                                       *
  * 
  * Copied into a C++ object to allow multiple fonts by: Luc-Olivier de
  * Charriere
  * 
  * David Olofson <do@reologica.se>:
  * * Strings changed to 'const char *'
  * * Cursor tests first check if '|' is present.
  * * Shadowed variables fixed.
  * * Garbage data in spacing table fixed. (Thanks to Andreas Spångberg for
  *   discovering this one!)
  * 
  * Put back into C by Jean-Christophe Hoelt <jeko@free.fr> */

#ifndef __SOFONT_H
#define __SOFONT_H

//#include "glSDL.h"
#include "IosImgProcess.h"

typedef struct _SOFONT SoFont;

SoFont *SoFont_new ();
void    SoFont_free (SoFont * font);

#ifdef ENABLE_TTF
int     SoFont_load_ttf (SoFont * font, const char *fileName, int size);
#else
int     SoFont_load (SoFont * font, IIM_Surface *picture);
#endif
void    SoFont_Refresh(SoFont * font);

/// Blits a string to a surface
///   Destination: the suface you want to blit to
///   text: a string containing the text you want to blit.
void    SoFont_PutString (SoFont * font, SDL_Surface * Surface, int x, int y,
													const char *text, SDL_Rect * clip /*=NULL*/ );
void    SoFont_PutStringWithCursor (SoFont * font, SDL_Surface * Surface,
																		int x, int y, const char *text,
																		int cursPos, SDL_Rect * clip /*=NULL*/ , int showCurs																															/*=true*/
	);

/// Returns the width of "text" in pixels
int     SoFont_TextWidth (SoFont * font, const char *text);
int     SoFont_TextWidth_MinMax (SoFont * font, const char *text,
																 int min /*=0*/ , int max /*=255*/ );

int     SoFont_FontHeight (SoFont * font);

/// Blits a string to with centered x position
void    SoFont_XCenteredString (SoFont * font, SDL_Surface * Surface, int y,
																const char *text, SDL_Rect * clip /*=NULL*/ );

/// Blits a string to with centered x & y position
void    SoFont_CenteredString (SoFont * font, SDL_Surface * Surface,
															 const char *text, SDL_Rect * clip /*=NULL*/ );

/// Blits a string to with centered around x & y positions
void    SoFont_CenteredString_XY (SoFont * font, SDL_Surface * Surface, int x,
																	int y, const char *text,

																	SDL_Rect * clip /*=NULL*/ );

/// This was specially developped for GUI
void    SoFont_PutStringCleverCursor (SoFont * font, SDL_Surface * Surface,
																			const char *text, int cursPos,
																			SDL_Rect * r,
																			SDL_Rect * clip /*=NULL*/ ,

																			int showCurs /*=true*/ );

/// Gives the cursor position given a x-axix point in the text
int     SoFont_TextCursorAt (SoFont * font, const char *text, int px);
int     SoFont_CleverTextCursorAt (SoFont * font, const char *text, int px,
																	 int cursPos, SDL_Rect * r);

#define START_CHAR 33
int     SoFont_getMinChar (SoFont * font);
int     SoFont_getMaxChar (SoFont * font);

#endif
