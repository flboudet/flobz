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

/* There is still room for improvements... */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "PuyoDoomMelt.h"

/* A few definitions */

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define COL_WIDTH 8
#define NUM_COLS SCREEN_WIDTH / COL_WIDTH + 1

static int rand_fun () {
    return rand() % 256;
}

typedef struct column {
    short x;
    short y;
} column_t;

/* Private methods */

static void _init_columns (DoomMelt *_this);
static void _column_draw  (column_t *column, SDL_Surface *meltImage, SDL_Surface *screen);
static void _column_think (column_t *column, int *isFinished);


/* Private members */

struct _DoomMelt {
  column_t     columns[NUM_COLS];
  IIM_Surface *surf;
  int          isFinished;
};

/* Public methods */

DoomMelt *doom_melt_new (void)
{
  DoomMelt *_this = (DoomMelt*)malloc(sizeof(DoomMelt));
  _this->isFinished = 1;
  return _this;
}

void doom_melt_delete (DoomMelt *_this)
{
  free(_this);
}

void doom_melt_start (DoomMelt *_this, IIM_Surface *surf)
{
  _this->surf       = surf;
  _this->isFinished = 0;
  _init_columns(_this);
}

void doom_melt_update (DoomMelt *_this)
{
  if (!_this->isFinished)
  {
    int i = 0;
    _this->isFinished = 1;
    for (; i < NUM_COLS; i++)
      _column_think (&_this->columns[i], &_this->isFinished); 
  }
}

void doom_melt_display(DoomMelt *_this, SDL_Surface *display)
{
  if (!_this->isFinished)
  {
    int i = 0;
    for (; i < NUM_COLS; i++)
    _column_draw(&_this->columns[i], _this->surf->surf, display);
  }
}

/*
 * Private methods
 */

void _init_columns (DoomMelt *_this)
{
    int i, start_x = 1;
    
    _this->columns[0].y = -(rand_fun() % 16);
    _this->columns[0].x = 0;
    
    for (i = 1; i < NUM_COLS; i++)
    {
        int r = (rand_fun() % 3) - 1;
        _this->columns[i].y = _this->columns[i-1].y + r;
        if (_this->columns[i].y > 0)
            _this->columns[i].y = 0;
        else if (_this->columns[i].y == -16)
            _this->columns[i].y = -15;
        
        _this->columns[i].x = start_x;
        
        start_x += COL_WIDTH;
    }
}

void _column_draw (column_t *column, SDL_Surface *meltImage, SDL_Surface *display)
{
    static SDL_Rect image_rect = {0, 0, COL_WIDTH, };
    static SDL_Rect dest_rect =  {0, 0, COL_WIDTH, SCREEN_HEIGHT};
    
    int tmp = column->y;
    if (tmp < 0) 
        tmp = 0;
    
    dest_rect.x = column->x;
    dest_rect.y = tmp;
    
    image_rect.x = column->x;
    image_rect.h = meltImage->h - tmp;
    
    SDL_BlitSurface(meltImage, &image_rect, display, &dest_rect);
}

void _column_think (column_t *column, int *isFinished)
{
    static int grow = 0;
    
    if (column->y < 0)
    {
        *isFinished = 0;
        grow = 1;
    }
    else if (column->y < SCREEN_HEIGHT)
    {
        *isFinished = 0;
        if (column->y < 16)
            grow = column->y+3;
        else
        {
            grow = 15; 
        }
    }
    
    column->y += grow;
}

