#include "IosImgProcess.h"
#include <math.h>
#include <stdlib.h>


extern char *dataFolder;


/** Image loading... free... conversion */

bool fullscreen = true;
bool useGL = false;

IIM_Surface  imgList[4096];
int          imgListSize = 0;

IIM_Surface * IIM_Load_DisplayFormat (const char *fname)
{
  char path[1024];
  SDL_Surface *tmpsurf, *retsurf;
  sprintf(path, "%s/gfx/%s", dataFolder, fname);
  tmpsurf = IMG_Load (path);
  if (tmpsurf==0) {
    fprintf(stderr,"Could not load %s\n", path);
    exit(1);
  }
  retsurf = SDL_DisplayFormat (tmpsurf);
  SDL_FreeSurface (tmpsurf);
  return IIM_RegisterImg(retsurf, false);
}

IIM_Surface * IIM_Load_DisplayFormatAlpha (const char *fname)
{
  char path[1024];
  SDL_Surface *tmpsurf, *retsurf;
  sprintf(path, "%s/gfx/%s", dataFolder, fname);
  tmpsurf = IMG_Load (path);
  if (tmpsurf==0) {
    fprintf(stderr,"Could not load %s\n", path);
    exit(1);
  }
  retsurf = SDL_DisplayFormatAlpha (tmpsurf);
  SDL_SetAlpha (retsurf, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
  SDL_FreeSurface (tmpsurf);
  return IIM_RegisterImg(retsurf, true);
}

void IIM_Free(IIM_Surface *img)
{
  for (int i=0; i<imgListSize; ++i)
  {
    if (&imgList[i] == img)
    {
      SDL_FreeSurface(imgList[i].surf);
      imgList[i].surf = 0;
      return;
    }
  }
}

IIM_Surface * IIM_RegisterImg(SDL_Surface *img, bool isAlpha)
{
  imgList[imgListSize].isAlpha = isAlpha;
  imgList[imgListSize].surf    = img;
  imgList[imgListSize].w       = img->w;
  imgList[imgListSize].h       = img->h;
  return &(imgList[imgListSize++]);
}

void IIM_ReConvertAll(void)
{
  for (int i=0; i<imgListSize; ++i)
  {
    if (imgList[i].surf)
    {
      if (imgList[i].isAlpha)
      {
        SDL_Surface *retsurf = SDL_DisplayFormat(imgList[i].surf);
        SDL_SetAlpha (retsurf, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
        SDL_FreeSurface(imgList[i].surf);
        imgList[i].surf = retsurf;
      }
      else
      {
        SDL_Surface *retsurf = SDL_DisplayFormat(imgList[i].surf);
        SDL_FreeSurface(imgList[i].surf);
        imgList[i].surf = retsurf;
      }
    }
  }
}

/** Image processing */

static int max3(int i1, int i2, int i3)
{
  if ((i1>=i2)&&(i1>=i3)) return i1;
  if ((i2>=i3)&&(i2>=i1)) return i2;
  return i3;
}

static int min3(int i1, int i2, int i3)
{
  if ((i1<=i2)&&(i1<=i3)) return i1;
  if ((i2<=i3)&&(i2<=i1)) return i2;
  return i3;
}

/* Extracting color components from a 32-bit color value
 * pre: SDL_Locked(surface) */
RGBA iim_surface_get_rgba(SDL_Surface *surface, Uint32 x, Uint32 y)
{
  Uint32 temp, pixel;
  RGBA result;
  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;
  pixel=*(Uint32*)((char*)surface->pixels+index);

  /* Get Red component */
  temp=pixel&fmt->Rmask; /* Isolate red component */
  temp=temp>>fmt->Rshift;/* Shift it down to 8-bit */
  temp=temp<<fmt->Rloss; /* Expand to a full 8-bit number */
  result.red=(Uint8)temp;

  /* Get Green component */
  temp=pixel&fmt->Gmask; /* Isolate green component */
  temp=temp>>fmt->Gshift;/* Shift it down to 8-bit */
  temp=temp<<fmt->Gloss; /* Expand to a full 8-bit number */
  result.green=(Uint8)temp;

  /* Get Blue component */
  temp=pixel&fmt->Bmask; /* Isolate blue component */
  temp=temp>>fmt->Bshift;/* Shift it down to 8-bit */
  temp=temp<<fmt->Bloss; /* Expand to a full 8-bit number */
  result.blue=(Uint8)temp;

  /* Get Alpha component */
  temp=pixel&fmt->Amask; /* Isolate alpha component */
  temp=temp>>fmt->Ashift;/* Shift it down to 8-bit */
  temp=temp<<fmt->Aloss; /* Expand to a full 8-bit number */
  result.alpha=(Uint8)temp;

  return result;
}

Uint8 iim_rgba2gray(RGBA col)
{
  unsigned int level;
  level  = col.red;
  level += col.green;
  level += col.blue;
  level /= 3; // Volontairement assombrie.
  return level;
}

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgb(SDL_Surface *surface,
                         Uint32 x, Uint32 y, RGBA c)
{
  Uint32 temp, pixel;
  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;
  temp = ~(fmt->Rmask | fmt->Gmask | fmt->Bmask);

  pixel = *(Uint32*)((char*)surface->pixels+index);
  pixel &= temp;

  /* Get Red component */
  temp = c.red >> fmt->Rloss;
  temp = temp  << fmt->Rshift;
  pixel |= temp;

  /* Get Green component */
  temp = c.green >> fmt->Gloss;
  temp = temp    << fmt->Gshift;
  pixel |= temp;

  /* Get Blue component */
  temp = c.blue >> fmt->Bloss;
  temp = temp   << fmt->Bshift;
  pixel |= temp;

  *(Uint32*)((char*)surface->pixels+index) = pixel;
}

/* pre: SDL_Locked(surface) */
void iim_surface_set_rgba(SDL_Surface *surface,
                          Uint32 x, Uint32 y, RGBA c)
{
  Uint32 temp, pixel = 0;
  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;

  /* Get Red component */
  temp = c.red >> fmt->Rloss;
  temp = temp  << fmt->Rshift;
  pixel |= temp;

  /* Get Green component */
  temp = c.green >> fmt->Gloss;
  temp = temp    << fmt->Gshift;
  pixel |= temp;

  /* Get Blue component */
  temp = c.blue >> fmt->Bloss;
  temp = temp   << fmt->Bshift;
  pixel |= temp;

  /* Get Alpha component */
  temp = c.alpha >> fmt->Aloss;
  temp = temp    << fmt->Ashift;
  pixel |= temp;

  *(Uint32*)((char*)surface->pixels+index) = pixel;
}

//-- RGB<->HSV conversion

//-- RGB, each 0 to 255
//-- H = 0.0 to 360.0 (corresponding to 0..360.0 degrees around hexcone)
//-- S = 0.0 (shade of gray) to 1.0 (pure color)
//-- V = 0.0 (black) to 1.0 (white)

//-- Based on C Code in "Computer Graphics -- Principles and Practice,"
//-- Foley et al, 1996, pp. 592,593.
HSVA iim_rgba2hsva(RGBA c)
{
  HSVA res;
  float minVal = (float)min3(c.red, c.blue, c.green);
  res.value    = (float)max3(c.red, c.green, c.blue);
  float delta  = res.value - minVal;

  // -- Calculate saturation: saturation is 0 if r, g and b are all 0
  if (res.value == 0.0f)
    res.saturation = 0.0f;
  else
    res.saturation = delta / res.value;

  if (res.saturation == 0.0f)
    res.hue = 0.0f; // Achromatic: When s = 0, h is undefined but who cares
  else             // Chromatic
    if (res.value == c.red) // between yellow and magenta [degrees]
      res.hue = 60.0f * (c.green - c.blue) / delta;
    else if (res.value == c.green) // between cyan and yellow
      res.hue = 120.0 + 60.0f * (c.blue - c.red) / delta;
    else // between magenta and cyan
      res.hue = 240.0f + 60.0f * (c.red - c.green) / delta;

  if (res.hue < 0.0f) res.hue += 360.0f;
  // return a list of values as an rgb object would not be sensible
  res.value /= 255.0f;
  res.alpha = c.alpha;
  return res;
}

RGBA iim_hsva2rgba(HSVA c)
{
  float red=0.0f,green=0.0f,blue=0.0f,hueTemp=0.0f;
  if (c.saturation == 0.0f) // color is on black-and-white center line
  {
    red   = c.value; // achromatic: shades of gray
    green = c.value; // supposedly invalid for h=0 but who cares
    blue  = c.value;
  }
  else { // chromatic color
    if (c.hue == 360.0f)      // 360 degrees same as 0 degrees
      hueTemp=0.0f;
    else 
      hueTemp = c.hue;

    hueTemp = hueTemp/60.0f;  // h is now in [0,6)
    float i = floor(hueTemp); // largest integer <= h
    float f = hueTemp-i;      // fractional part of h

    float p = c.value*(1.0f - c.saturation);
    float q = c.value*(1.0f-(c.saturation*f));
    float t = c.value*(1.0f-(c.saturation*(1.0-f)));

    switch((int)i) {
      case 0:
        red   = c.value;
        green = t;
        blue  = p;
        break;
      case 1:
        red   = q;
        green = c.value;
        blue  = p;
        break;
      case 2:
        red   = p;
        green = c.value;
        blue  = t;
        break;
      case 3:
        red   = p;
        green = q;
        blue  = c.value;
        break;
      case 4:
        red   = t;
        green = p;
        blue  = c.value;
        break;
      case 5:
        red   = c.value;
        green = p;
        blue  = q;
        break;
    }
  }
  RGBA ret;
  ret.red   = (Uint8)(red   * 255.0f);
  ret.green = (Uint8)(green * 255.0f);
  ret.blue  = (Uint8)(blue  * 255.0f);
  ret.alpha = c.alpha;
  return ret;
}

IIM_Surface *iim_surface_shift_hue(IIM_Surface *isrc, float hue_offset)
{
  SDL_Surface *src = isrc->surf;
  SDL_PixelFormat *fmt = src->format;
  SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                          fmt->Rmask, fmt->Gmask,
                                          fmt->Bmask, fmt->Amask);
  SDL_LockSurface(src);
  SDL_LockSurface(ret);
  for (int y=src->h; y--;)
  {
    for (int x=src->w; x--;)
    {
      RGBA rgba = iim_surface_get_rgba(src,x,y);
      HSVA hsva = iim_rgba2hsva(rgba);
      hsva.hue += hue_offset;
      if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
      if (hsva.hue < 0.0f) hsva.hue += 360.0f;
      rgba = iim_hsva2rgba(hsva);
      iim_surface_set_rgba(ret,x,y,rgba);
    }
  }
  SDL_UnlockSurface(ret);
  SDL_UnlockSurface(src);
  SDL_Surface *ret2 = SDL_DisplayFormatAlpha(ret);
	SDL_SetAlpha(ret2, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
	SDL_FreeSurface(ret);
  ret = isrc->surf;
  isrc->surf = SDL_DisplayFormatAlpha(ret);
  SDL_SetAlpha(isrc->surf, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
  SDL_FreeSurface(ret);
  return IIM_RegisterImg(ret2, true);
}

IIM_Surface *iim_surface_set_value(IIM_Surface *isrc, float value)
{
  SDL_Surface *src = isrc->surf;
  SDL_PixelFormat *fmt = src->format;
  SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                          fmt->Rmask, fmt->Gmask,
                                          fmt->Bmask, fmt->Amask);
  SDL_LockSurface(src);
  SDL_LockSurface(ret);
  for (int y=src->h; y--;)
    {
      for (int x=src->w; x--;)
	{
	  RGBA rgba = iim_surface_get_rgba(src,x,y);
	  HSVA hsva = iim_rgba2hsva(rgba);
	  hsva.value = value;
	  if (hsva.value > 1.0f) hsva.value = 1.0f;
	  if (hsva.value < 0.0f) hsva.value = 0.0f;
	  rgba = iim_hsva2rgba(hsva);
	  iim_surface_set_rgba(ret,x,y,rgba);
	}
    }
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(ret);
  SDL_Surface *ret2 = SDL_DisplayFormatAlpha(ret);
  SDL_SetAlpha(ret2, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
  SDL_FreeSurface(ret);
  return IIM_RegisterImg(ret2, true);
}

void iim_surface_convert_to_gray(IIM_Surface *isrc)
{
  SDL_Surface *src = isrc->surf;
  SDL_PixelFormat *fmt = src->format;
  SDL_LockSurface(src);
  for (int y=src->h; y--;)
  {
    for (int x=src->w; x--;)
    {
      RGBA rgba = iim_surface_get_rgba(src,x,y);
      Uint8 l = iim_rgba2gray(rgba);
      rgba.red = rgba.blue = rgba.green = l;
      iim_surface_set_rgb(src,x,y,rgba);
    }
  }
  SDL_UnlockSurface(src);
  isrc->surf = SDL_DisplayFormat(src);
  SDL_FreeSurface(src);
}
