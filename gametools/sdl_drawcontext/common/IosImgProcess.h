#ifndef _IOS_IMG_PROCESS
#define _IOS_IMG_PROCESS

#include "SDL.h"
#include "rgba.h"

// Pure SDL part

/* Low-level functions */

/* Extracting color components from a 32-bit color value
 * pre: SDL_Locked(surface) */
RGBA iim_surface_get_rgba(SDL_Surface *surface, Uint32 x, Uint32 y);

/* Extracting color components from a 32-bit color value
 * pre: SDL_Locked(surface) */
Uint8 iim_surface_get_alpha(SDL_Surface *surface, Uint32 x, Uint32 y);

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgb(SDL_Surface *surface,
                         Uint32 x, Uint32 y, RGBA c);

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgba(SDL_Surface *surface,
                          Uint32 x, Uint32 y, RGBA c);

/* pre: SDL_Locked(surface) */
void iim_surface_set_alpha(SDL_Surface *surface,
                          Uint32 x, Uint32 y, Uint8 alpha);

/* pre: SDL_Locked(surface) */
void iim_surface_blend_rgba(SDL_Surface *surface,
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

/* Higher level functions */

/**
 * Shift the saturation of a surface
 */
SDL_Surface *iim_sdlsurface_shift_hsv(SDL_Surface *src, float h, float s, float v);

/**
 * Shift the hue of a surface
 */
SDL_Surface *iim_sdlsurface_shift_hue(SDL_Surface *isrc, float hue_offset);

/**
 * Shift the hue of a surface with a 1-bit mask
 */
SDL_Surface *iim_sdlsurface_shift_hue_masked(SDL_Surface *src, SDL_Surface *mask, float hue_offset);

/**
 * Change the value (luminosity) of each pixel in a surface
 */
SDL_Surface *iim_sdlsurface_set_value(SDL_Surface *src, float value);

/**
 * Change the alpha (transparency) of each pixel in a surface
 */
SDL_Surface *iim_sdlsurface_set_alpha(SDL_Surface *src, float alpha);

/**
* Resize a surface
 */
SDL_Surface *iim_sdlsurface_resize(SDL_Surface *src, int width, int height);

/**
 * Resize a surface
 */
SDL_Surface *iim_sdlsurface_resize_alpha(SDL_Surface *src, int width, int height);

/**
 * Mirror a surface
 */
SDL_Surface *iim_sdlsurface_mirror_h(SDL_Surface *src);

/**
 * rotate a surface into a surface of the same size (may lost datas)
 */
SDL_Surface *iim_sdlsurface_rotate(SDL_Surface *src, int degrees);

/**
 * Convert a surface to B&W
 */
void iim_sdlsurface_convert_to_gray(SDL_Surface *src);

SDL_Surface *iim_sdlsurface_create_rgba(int width, int height);

SDL_Surface *iim_sdlsurface_create_rgb(int width, int height);

// IIM abstraction
extern bool useGL;

typedef struct _IIM_Surface {
  SDL_Surface *surf;
  int w;
  int h;
  bool isAlpha;
  struct _IIM_Surface *rotated[36];
  struct _IIM_Surface *fliph;
} IIM_Surface;

#define IIM_Rect SDL_Rect

IIM_Surface *iim_surface_shift_hue(IIM_Surface *src, float hue_offset);
IIM_Surface *iim_surface_shift_hue_masked(IIM_Surface *isrc, IIM_Surface *imask, float hue_offset);
IIM_Surface *iim_surface_set_value(IIM_Surface *src, float value);
IIM_Surface *iim_surface_resize(IIM_Surface *isrc, int width, int height);
IIM_Surface *iim_surface_resize_alpha(IIM_Surface *isrc, int width, int height);
IIM_Surface *iim_surface_duplicate(IIM_Surface *isrc);
IIM_Surface *iim_surface_shift_hsv(IIM_Surface *src, float h, float s, float v);
IIM_Surface *iim_surface_mirror_h(IIM_Surface *isrc);
void iim_surface_convert_to_gray(IIM_Surface *src);
IIM_Surface *iim_surface_create_rgba(int width, int height);
IIM_Surface *iim_surface_create_rgb(int width, int height);

IIM_Surface * IIM_Load_Absolute_DisplayFormat (const char *path);
IIM_Surface * IIM_Load_Absolute_DisplayFormatAlpha (const char *path);
void          IIM_Free(IIM_Surface *img);
IIM_Surface * IIM_RegisterImg(SDL_Surface *img, bool isAlpha);
void          IIM_ReConvertAll(void);

int IIM_BlitSurface(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect);
int IIM_BlitFlippedSurface(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect);
void IIM_BlitSurfaceAlpha(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect, Uint8 alpha);
void IIM_BlitRotatedSurfaceCentered(IIM_Surface *src, int degrees, SDL_Surface *dst, int x, int y);


#endif
