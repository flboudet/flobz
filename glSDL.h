/*(LGPL)
------------------------------------------------------------
	glSDL 0.6 - SDL 2D API on top of OpenGL
------------------------------------------------------------
 * (c) David Olofson, 2001-2003
 * This code is released under the terms of the GNU LGPL.
 */

#ifndef	_GLSDL_H_
#define	_GLSDL_H_

/*
 * If you don't use GNU autotools or similar, uncomment this to
 * compile with OpenGL enabled:
#define HAVE_OPENGL
 */

/* We're still using SDL datatypes here - we just add some stuff. */
#include <SDL/SDL.h>
#ifdef MACOSX
#include <SDL_image/SDL_image.h>
#else
#include <SDL/SDL_image.h>
#endif

/* Fakes to make glSDL code compile with SDL. */
#define SDL_GLSDL		0
#define	LOGIC_W(s)		( (s)->w )
#define	LOGIC_H(s)		( (s)->h )
#define	GLSDL_FIX_SURFACE(s)

#endif
