#ifndef _IOS_IMG_PROCESS
#define _IOS_IMG_PROCESS

#include "glSDL.h"

typedef struct _RGBA {
  Uint8 red;
  Uint8 green;
  Uint8 blue;
  Uint8 alpha;
} RGBA;

typedef struct _HSVA {
  float hue;
  float saturation;
  float value;
  Uint8 alpha;
} HSVA;

/* Extracting color components from a 32-bit color value
 * pre: SDL_Locked(surface) */
RGBA iim_surface_get_rgba(SDL_Surface *surface, Uint32 x, Uint32 y);

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgb(SDL_Surface *surface,
                         Uint32 x, Uint32 y, RGBA c);

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgba(SDL_Surface *surface,
                          Uint32 x, Uint32 y, RGBA c);

//-- RGB<->HSV conversion

//-- RGB, each 0 to 255
//-- H = 0.0 to 360.0 (corresponding to 0..360.0 degrees around hexcone)
//-- S = 0.0 (shade of gray) to 1.0 (pure color)
//-- V = 0.0 (black) to 1.0 (white)

//-- Based on C Code in "Computer Graphics -- Principles and Practice,"
//-- Foley et al, 1996, pp. 592,593.
HSVA iim_rgba2hsva(RGBA c);
RGBA iim_hsva2rgba(HSVA c);

typedef struct {
  SDL_Surface *surf;
  int w;
  int h;
  bool isAlpha;
} IIM_Surface;

IIM_Surface *iim_surface_shift_hue(IIM_Surface *src, float hue_offset);
IIM_Surface *iim_surface_set_value(IIM_Surface *src, float value);
void iim_surface_convert_to_gray(IIM_Surface *src);

IIM_Surface * IIM_Load_DisplayFormat (const char *path);
IIM_Surface * IIM_Load_DisplayFormatAlpha (const char *path);
void          IIM_Free(IIM_Surface *img);
IIM_Surface * IIM_RegisterImg(SDL_Surface *img, bool isAlpha);
void          IIM_ReConvertAll(void);

#endif
