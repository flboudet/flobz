#include "IosImgProcess.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h> // Conforms to ANSI "C 89", is that OK?

/** Image loading... free... conversion */

bool fullscreen = true;
bool useGL = false;

#define MAXIMG 8192
IIM_Surface  imgList[MAXIMG];
int          imgListSize = 0;
int          imgListIndex = 0;

IIM_Surface * IIM_Load_Absolute_DisplayFormat (const char *path)
{
  SDL_Surface *tmpsurf, *retsurf;
  tmpsurf = IMG_Load (path);
  if (tmpsurf==NULL) {
    return NULL;
  }
  retsurf = SDL_DisplayFormat (tmpsurf);
  SDL_FreeSurface (tmpsurf);
  return IIM_RegisterImg(retsurf, false);
}

IIM_Surface * IIM_Load_Absolute_DisplayFormatAlpha (const char * path)
{
  SDL_Surface *tmpsurf, *retsurf;
  tmpsurf = IMG_Load (path);
  if (tmpsurf==NULL) {
    return NULL;
  }
  retsurf = SDL_DisplayFormatAlpha (tmpsurf);
  if (retsurf==NULL) {
    perror("Texture conversion failed (is Display initialized?)\n");
    SDL_FreeSurface (tmpsurf);
    return NULL;
  }
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
        if (img->surf != NULL) SDL_FreeSurface(imgList[i].surf);
        imgList[i].surf = NULL;
        //fprintf(stderr,"Pictures array freed %d\n",i);
        for (int j=1; j<36; ++j)
            if (img->rotated[j] != NULL) IIM_Free(img->rotated[j]);
        if (img->fliph != NULL) IIM_Free(img->fliph);
        return;
    }
  }
}

IIM_Surface * IIM_RegisterImg(SDL_Surface *img, bool isAlpha)
{
    IIM_Surface *_this = NULL;
    /* We still have empty slots */
    if (imgListSize < MAXIMG) {
        _this = &imgList[imgListSize++];
        //fprintf(stderr,"Pictures array using %d\n",imgListSize-1);
    } else {
        // try to find another place
        int orig = imgListIndex;
        for (int i=(orig+1)%MAXIMG; i!=orig; i=(i+1)%MAXIMG)
        {
            if (imgList[i].surf == NULL) {
                _this = &imgList[i];
                imgListIndex = i;
                //fprintf(stderr,"Pictures array reusing %d\n",imgListIndex);
                break;
            }
        }
        if (_this == NULL) {
            fprintf(stderr,"Pictures array limit exceeded, aborting...\n");
            exit(0);
        }
    }
  _this->isAlpha = isAlpha;
  _this->surf    = img;
  _this->w       = img->w;
  _this->h       = img->h;
  for (int i=0; i<36; ++i)
    _this->rotated[i] = NULL;
  _this->rotated[0] = _this;
  _this->fliph = NULL;
  return _this;
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

Uint8 iim_surface_get_alpha(SDL_Surface *surface, Uint32 x, Uint32 y)
{
  Uint32 temp, pixel;

  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;
  pixel=*(Uint32*)((char*)surface->pixels+index);

  /* Get Alpha component */
  temp=pixel&fmt->Amask; /* Isolate alpha component */
  temp=temp>>fmt->Ashift;/* Shift it down to 8-bit */
  temp=temp<<fmt->Aloss; /* Expand to a full 8-bit number */
  return (Uint8)temp;
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

void iim_surface_set_alpha(SDL_Surface *surface,
                          Uint32 x, Uint32 y, Uint8 alpha)
{
  Uint32 pixel = 0;
  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;

  /* Get Alpha component */
  pixel = alpha >> fmt->Aloss;
  pixel <<= fmt->Ashift;

  *(Uint32*)((char*)surface->pixels+index) = pixel;
}

/* pre: SDL_Locked(surface) */
void iim_surface_blend_rgba(SDL_Surface *surface,
                          Uint32 x, Uint32 y, RGBA c)
{
  Uint32 temp, pixel = 0;
  int index = x*surface->format->BytesPerPixel + y*surface->pitch;
  SDL_PixelFormat *fmt = surface->format;

  RGBA src = iim_surface_get_rgba(surface, x,y);
  c.red   = ((int)src.red   * (255 - (int)c.alpha) + (int)c.red   * (int)c.alpha) / 255;
  c.green = ((int)src.green * (255 - (int)c.alpha) + (int)c.green * (int)c.alpha) / 255;
  c.blue  = ((int)src.blue  * (255 - (int)c.alpha) + (int)c.blue  * (int)c.alpha) / 255;
  c.alpha = ((int)src.alpha  * (255 - (int)c.alpha) + (int)c.alpha  * (int)c.alpha) / 255;

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
      hueTemp = 0.0f;
    else {
      hueTemp = c.hue / 60.0f;  // h is now in [0,6)
      // hueTemp = hueTemp / 60.0f;
    }
    float i = floor(hueTemp); // largest integer <= h
    float f = hueTemp - i;    // fractional part of h

    float p = c.value * (1.0f - c.saturation);
    float q = c.value * (1.0f - (c.saturation * f));
    float t = c.value * (1.0f - (c.saturation * (1.0-f)));

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

/**
 * Shift the saturation of a surface
 */
IIM_Surface *iim_surface_shift_hsv(IIM_Surface *isrc, float h, float s, float v)
{
    SDL_Surface *src = isrc->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);
    for (int y=src->h; y--;)
    {
        for (int x=src->w; x--;)
        {
            RGBA rgba = iim_surface_get_rgba(src,x,y);
            HSVA hsva = iim_rgba2hsva(rgba);
            hsva.hue += h;
            if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
            if (hsva.hue < 0.0f) hsva.hue += 360.0f;
            hsva.saturation += s;
            if (hsva.saturation > 1.) hsva.saturation = 1.f;
            if (hsva.saturation < 0.0f) hsva.saturation = .0f;
            hsva.value += v;
            if (hsva.value > 1.) hsva.value = 1.f;
            if (hsva.value < 0.0f) hsva.value = .0f;
            rgba = iim_hsva2rgba(hsva);
            iim_surface_set_rgba(ret,x,y,rgba);
        }
    }
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

/**
 * Shift the hue of a surface
 */
IIM_Surface *iim_surface_shift_hue(IIM_Surface *isrc, float hue_offset)
{
    SDL_Surface *src = isrc->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);
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
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

/**
 * Shift the hue of a surface with a 1-bit mask
 */
IIM_Surface *iim_surface_shift_hue_masked(IIM_Surface *isrc, IIM_Surface *imask, float hue_offset)
{
    SDL_Surface *src = isrc->surf;
    SDL_Surface *mask = imask->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);
    bool masklocked = false;
    if(SDL_MUSTLOCK(mask)) masklocked = (SDL_LockSurface(mask) == 0);
    for (int y=src->h; y--;)
    {
        for (int x=src->w; x--;)
        {
            RGBA rgba = iim_surface_get_rgba(src,x,y);
            RGBA mrgba = iim_surface_get_rgba(mask,x,y);
            if ((mrgba.red == 0) && (mrgba.green == 0) && (mrgba.blue == 0) && (mrgba.alpha > 128)) {
                iim_surface_set_rgba(ret,x,y,rgba);
            }
            else {
                HSVA hsva = iim_rgba2hsva(rgba);
                hsva.hue += hue_offset;
                if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
                if (hsva.hue < 0.0f) hsva.hue += 360.0f;
                rgba = iim_hsva2rgba(hsva);
                iim_surface_set_rgba(ret,x,y,rgba);
            }
        }
    }
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    if(masklocked) SDL_UnlockSurface(mask);
    return IIM_RegisterImg(ret, true);
}

/**
 * Change the value (luminosity) of each pixel in a surface
 */
IIM_Surface *iim_surface_set_value(IIM_Surface *isrc, float value)
{
    SDL_Surface *src = isrc->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);
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
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

/**
* Resize a surface
 */
IIM_Surface *iim_surface_resize(IIM_Surface *isrc, int width, int height)
{
    SDL_Surface *src = isrc->surf;
    int w = isrc->w;
    int h = isrc->h;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, width, height, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);

    int sizew = w/width;
    int sizeh = h/height;
    if (sizew<1) sizew = 1;
    if (sizeh<1) sizeh = 1;
    for (int y=height-1; y>=0; y--)
    {
        for (int x=width-1; x>=0; x--)
        {
            Uint32 red = 0;
            Uint32 green = 0;
            Uint32 blue = 0;
            Uint32 alpha = 0;
            int x1 = (x*w)/width;
            int y1 = (y*h)/height;
            for (int i=sizew; i>0; i--)
            {
                for (int j=sizeh; j>0; j--)
                {
                    RGBA rgba = iim_surface_get_rgba(src,x1+i-1,y1+j-1);
                    red += rgba.red;
                    green += rgba.green;
                    blue += rgba.blue;
                    //alpha += rgba.alpha;
                }
            }
            red /= (sizew*sizeh);
            green /= (sizew*sizeh);
            blue /= (sizew*sizeh);
            //alpha /= (sizew*sizeh);
            alpha = 255;
            RGBA finalrgba = {(Uint8)red,(Uint8)green,(Uint8)blue,(Uint8)alpha};
            iim_surface_set_rgba(ret,x,y,finalrgba);
        }
    }
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

/**
 * Resize a surface
 */
IIM_Surface *iim_surface_resize_alpha(IIM_Surface *isrc, int width, int height)
{
    SDL_Surface *src = isrc->surf;
    int w = isrc->w;
    int h = isrc->h;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, width, height, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);

    int sizew = w/width;
    int sizeh = h/height;
    if (sizew<1) sizew = 1;
    if (sizeh<1) sizeh = 1;
    for (int y=height-1; y>=0; y--)
    {
        for (int x=width-1; x>=0; x--)
        {
            Uint32 red = 0;
            Uint32 green = 0;
            Uint32 blue = 0;
            Uint32 alpha = 0;
            int x1 = (x*w)/width;
            int y1 = (y*h)/height;
            for (int i=sizew; i>0; i--)
            {
                for (int j=sizeh; j>0; j--)
                {
                    RGBA rgba = iim_surface_get_rgba(src,x1+i-1,y1+j-1);
                    red += rgba.red;
                    green += rgba.green;
                    blue += rgba.blue;
                    alpha += rgba.alpha;
                }
            }
            red /= (sizew*sizeh);
            green /= (sizew*sizeh);
            blue /= (sizew*sizeh);
            alpha /= (sizew*sizeh);
            //alpha = 255;
            RGBA finalrgba = {(Uint8)red,(Uint8)green,(Uint8)blue,(Uint8)alpha};
            iim_surface_set_rgba(ret,x,y,finalrgba);
        }
    }
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

IIM_Surface *iim_surface_mirror_h(IIM_Surface *isrc)
{
    SDL_Surface *src = isrc->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);

    for (int y=src->h-1 ; y >= 0 ; y--)
    {
        for (int x=src->w-1 , ix = 0 ; x >= 0 ; x--, ix++)
        {
            iim_surface_set_rgba(ret, x, y, iim_surface_get_rgba(src, ix, y));
        }
    }

    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

/**
* Duplicate a surface
 */
IIM_Surface *iim_surface_duplicate(IIM_Surface *isrc)
{
    return IIM_RegisterImg(SDL_DisplayFormatAlpha(isrc->surf), true);
}

/**
 * rotate a surface into a surface of the same size (may lost datas)
 */
IIM_Surface *iim_rotate(IIM_Surface *isrc, int degrees)
{
    SDL_Surface *src = isrc->surf;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w, src->h, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    float radians = degrees * 3.1415f / 180.0f;
    float cosa    = cos(radians);
    float sina    = sin(radians);
    float cx = src->w / 2;
    float cy = src->h / 2;
    float xmax = src->w-1;
    float ymax = src->h-1;

    bool srclocked = false;
    if(SDL_MUSTLOCK(src)) srclocked = (SDL_LockSurface(src) == 0);
    bool retlocked = false;
    if(SDL_MUSTLOCK(ret)) retlocked = (SDL_LockSurface(ret) == 0);

    for (int y=src->h; y--;)
    {
        for (int x=src->w; x--;)
        {
            float vx  = (float)x - cx;
            float vy  = (float)y - cy;
            float fromX = (cosa * vx - sina * vy) + cx;
            float fromY = (sina * vx + cosa * vy) + cy;
            RGBA rgba = {0,0,0,0};
            if ((fromX>=0)&&(fromY>=0)&&(fromX<src->w)&&(fromY<src->h-1)) {

              float ix = floor(fromX);
              float iy = floor(fromY);

              float ex = fromX - ix;
              float ey = fromY - iy;
              float mex = 1.0f - ex;
              float mey = 1.0f - ey;

              int iix = (int)ix;
              iix = (iix < 0) ? 0 : iix;
              int iix2 = iix+1;
              iix2 = (iix2 > xmax) ? xmax : iix2;
              int iiy = (int)iy;
              iiy = (iiy < 0) ? 0 : iiy;
              int iiy2 = iiy+1;
              iiy2 = (iiy2 > ymax) ? ymax : iiy2;

              RGBA c11 = iim_surface_get_rgba(src,iix,iiy);
              RGBA c12 = iim_surface_get_rgba(src,iix,iiy2);
              RGBA c21 = iim_surface_get_rgba(src,iix2,iiy);
              RGBA c22 = iim_surface_get_rgba(src,iix2,iiy2);

              rgba.red = (int)(mex*mey*c11.red + mex*ey*c12.red + ex*mey*c21.red + ex*ey*c22.red);
              rgba.green = (int)(mex*mey*c11.green + mex*ey*c12.green + ex*mey*c21.green + ex*ey*c22.green);
              rgba.blue = (int)(mex*mey*c11.blue + mex*ey*c12.blue + ex*mey*c21.blue + ex*ey*c22.blue);
              rgba.alpha = (int)(mex*mey*c11.alpha + mex*ey*c12.alpha + ex*mey*c21.alpha + ex*ey*c22.alpha);
            }
            iim_surface_set_rgba(ret,x,y,rgba);
        }
    }
    if(retlocked) SDL_UnlockSurface(ret);
    if(srclocked) SDL_UnlockSurface(src);
    return IIM_RegisterImg(ret, true);
}

void iim_surface_convert_to_gray(IIM_Surface *isrc)
{
  SDL_Surface *src = isrc->surf;
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

IIM_Surface *iim_surface_create_rgba(int width, int height)
{
    SDL_Surface *ret, *tmp;
    Uint32 rmask, gmask, bmask, amask;
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, width, height, 32,
                                       rmask, gmask, bmask, amask);
    ret = SDL_DisplayFormatAlpha(tmp);
    SDL_FreeSurface(tmp);
    return IIM_RegisterImg(ret, true);
}

IIM_Surface *iim_surface_create_rgb(int width, int height)
{
    SDL_Surface *ret, *tmp;
    Uint32 rmask, gmask, bmask, amask;
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                               rmask, gmask, bmask, amask);
    ret = SDL_DisplayFormat(tmp);
    SDL_FreeSurface(tmp);
    return IIM_RegisterImg(ret, true);
}

int IIM_BlitSurface(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect)
{
  return SDL_BlitSurface(src->surf, src_rect, dst, dst_rect);
}

void IIM_BlitSurfaceAlpha(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect, Uint8 alpha)
{
  assert(src != NULL);
  assert(src->surf != NULL);
  SDL_SetAlpha(src->surf, SDL_SRCALPHA|(useGL?0:SDL_RLEACCEL), alpha);
  SDL_BlitSurface(src->surf, src_rect, dst, dst_rect);
}

int IIM_BlitFlippedSurface(IIM_Surface *src, IIM_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect)
{
    if (src->fliph == NULL) {
        // Generate flipped image.
        src->fliph = iim_surface_mirror_h(src);
    }
    return SDL_BlitSurface(src->fliph->surf, src_rect, dst, dst_rect);
}

void IIM_BlitRotatedSurfaceCentered(IIM_Surface *src, int degrees, SDL_Surface *dst, int x, int y)
{
  while (degrees < 0) degrees+=8640;
  degrees /= 10;
  degrees %= 36;
  if (!src->rotated[degrees]) {
    // Generated rotated image.
    src->rotated[degrees] = iim_rotate(src, degrees * 10);
  }
  x -= src->w/2;
  y -= src->h/2;
  IIM_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = src->w;
  rect.h = src->h;
  IIM_BlitSurface(src->rotated[degrees], NULL, dst, &rect);
}

