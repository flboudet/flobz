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

#include "SDL/SDL.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define COL_WIDTH 8
#define NUM_COLS SCREEN_WIDTH / COL_WIDTH + 1

typedef struct column
{
    short x, y;
} column_t;

static column_t columns[NUM_COLS];


static int rand_fun ()
{
    return rand() % 256;
}

static void init_columns ()
{
    int i, start_x = 1;
    
    columns[0].y = -(rand_fun() % 16);
    columns[0].x = 0;
    
    for (i = 1; i < NUM_COLS; i++)
    {
        int r = (rand_fun() % 3) - 1;
        columns[i].y = columns[i-1].y + r;
        if (columns[i].y > 0)
            columns[i].y = 0;
        else if (columns[i].y == -16)
            columns[i].y = -15;
        
        columns[i].x = start_x;
        
        start_x += COL_WIDTH;
    }
}



static inline void column_draw (column_t *column, SDL_Surface *meltImage, SDL_Surface *screen)
{
    static SDL_Rect image_rect = {0, 0, COL_WIDTH, };
    static SDL_Rect dest_rect = {0, 0, COL_WIDTH, SCREEN_HEIGHT};
    
    int tmp = column->y;
    if (tmp < 0) 
        tmp = 0;
    
    dest_rect.x = column->x;
    dest_rect.y = tmp;
    
    image_rect.x = column->x;
    image_rect.h = meltImage->h - tmp;
    
    SDL_BlitSurface(meltImage, &image_rect, screen, &dest_rect);
}

static void column_think (column_t *column, char *isFinished)
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

static void main_loop (SDL_Surface *meltImage, SDL_Surface *backgroundImage, SDL_Surface *screen)
{
    int i;
    char isFinished = 0;
    
    init_columns();
    while (!isFinished) {
        isFinished = 1;
        SDL_BlitSurface(backgroundImage, 0, screen, 0);
        
        for (i = 0; i < NUM_COLS; i++)
        {
            column_draw (columns + i, meltImage, screen);
            column_think (&columns[i], &isFinished); 
        }
        
        SDL_Flip(screen);
        SDL_Delay(1);
    }
}

void melt(SDL_Surface *nextImage, SDL_Surface *screen)
{
    SDL_Surface *meltImage;
    
    // clone of the screen
    meltImage = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                     screen->w, screen->h,
                                     screen->format->BitsPerPixel, 
                                     screen->format->Rmask, 
                                     screen->format->Gmask, 
                                     screen->format->Bmask, 
                                     screen->format->Amask);
    SDL_BlitSurface(screen, 0, meltImage, 0);
    
    init_columns ();
    main_loop (meltImage, nextImage, screen);
    SDL_FreeSurface(meltImage);
}

