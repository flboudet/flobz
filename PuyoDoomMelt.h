/* Copyright (C) 2002 W.P. van Paassen - peter@paassen.tmfweb.nl, 
                      Ed Sinjiashvili  - slimb@swes.saren.ru
		      dekoder          - dekoder81@users.sourceforge.net
   Copyright (C) 2004 flobo for this hacked version
   
   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _PUYODOOMMELT
#define _PUYODOOMMELT

#include "glSDL.h"
#include "IosImgProcess.h"

typedef struct _DoomMelt DoomMelt;

DoomMelt *doom_melt_new    (void);
void      doom_melt_delete (DoomMelt *_this);
void      doom_melt_update (DoomMelt *_this);
void      doom_melt_start  (DoomMelt *_this, IIM_Surface *surf);
void      doom_melt_display(DoomMelt *_this, SDL_Surface *display);

#endif /*_PUYODOOMMELT*/

