#include <math.h>
#include "IosImgProcess.h"
#include "SDL_IosFont.h"

using namespace std;

// IosFont implementation
SDL_IosFont::SDL_IosFont(const char *path, int size, IosFontFx fx)
    : m_fx(fx), m_height(size)
{
    m_font = TTF_OpenFont(path, size);
    m_height = TTF_FontHeight(m_font);
    precomputeFX();
}

SDL_IosFont::~SDL_IosFont()
{
    TTF_CloseFont(m_font);
    for (CachedSurfacesMap::iterator iter = m_cacheMap.begin() ;
         iter != m_cacheMap.end() ; iter++) {
        delete (iter->second);
    }
}

int SDL_IosFont::getTextWidth(const char *text)
{
    int w,h;
    TTF_SizeUTF8(m_font, text, &w, &h);
    return w;
}

int SDL_IosFont::getHeight()
{
    return m_height;
}

int SDL_IosFont::getLineSkip()
{
    return TTF_FontLineSkip(m_font);
}

IosSurface * SDL_IosFont::render(const char *text)
{
    IosSurface *result = getFromCache(text);
    if (result == NULL) {
        static const SDL_Colour white = {255,255,255};
        SDL_Surface *image = TTF_RenderUTF8_Blended(m_font, text, white);
        image = fontFX(image);
        result = createSurface(image);
        storeInCache(text, result);
    }
    return result;
}

IosSurface * SDL_IosFont::getFromCache(const char *text)
{
    CachedSurfacesMap::iterator iter = m_cacheMap.find(text);
    if (iter == m_cacheMap.end())
        return NULL;
    return iter->second;
}

void SDL_IosFont::storeInCache(const char *text, IosSurface *surf)
{
    m_cacheMap[text] = surf;
}

// Precompute the font effect function values from discretized alpha and positions.
void SDL_IosFont::precomputeFX()
{
    for (Uint8 _s_alpha=0; _s_alpha<8; ++_s_alpha) {
        for (Uint8 _f_alpha=0; _f_alpha<8; ++_f_alpha) {
            for (Uint8 _cx=0; _cx<64; ++_cx) {
                for (Uint8 _cy=0; _cy<16; ++_cy) {
                    Uint8 s_alpha = (Uint8)floor(_s_alpha * 36.428572);
                    Uint8 f_alpha = (Uint8)floor(_f_alpha * 36.428572);
                    float cx = (float)_cx / 64.0;
                    float cy = (float)_cy / 16.0;
                    float l = 0.5 + 0.5 * sin(cy * 6.29f - 1.7f);
                    l *= l;

                    HSVA hsva;
                    hsva.alpha = 0.0;
                    if (m_fx == Font_STD) {
                        hsva.hue = 55.0 - 20.0 * cx;
                        hsva.saturation = 1.25 - l;
                        hsva.value = 1.0;
                    }
                    else if (m_fx == Font_STORY) {
                        hsva.hue = 55.0 - 20.0 * cx;
                        hsva.saturation = 1.0;//5 - l;
                        hsva.value = 1.4 - cy;
                    }
                    else if (m_fx == Font_GREY) {
                        hsva.hue = 45.0 + 30.0 * cx;
                        hsva.saturation = 1.25 - l;
                        hsva.value = 1.0;
                    }
                    else { // Font_DARK
                        hsva.hue = 55.0 - 20.0 * cx;
                        hsva.saturation = 1.25 - l;
                        hsva.value = 0.5;
                    }

                    if (hsva.value > 1.)        hsva.value = 1.f;
                    if (hsva.saturation > 1.)   hsva.saturation = 1.f;
                    if (hsva.saturation < 0.0f) hsva.saturation = .0f;

                    RGBA rgba = iim_hsva2rgba(hsva);

                    rgba.red   = ((unsigned int)rgba.red   * (unsigned int)f_alpha) / 255;
                    rgba.green = ((unsigned int)rgba.green * (unsigned int)f_alpha) / 255;
                    rgba.blue  = ((unsigned int)rgba.blue  * (unsigned int)f_alpha) / 255;
                    rgba.alpha = ((unsigned int)s_alpha * (255 - (unsigned int)f_alpha)
                            + (unsigned int)f_alpha  * (unsigned int)f_alpha) / 255;

                    m_precomputed[_s_alpha][_f_alpha][_cx][_cy] = rgba;
                }
            }
        }
    }
}

/**
 * Apply nice FX to a SDL_Surface containing a font
 */
SDL_Surface *SDL_IosFont::fontFX(SDL_Surface *src)
{
    const int SHADOW_X = 1;
    const int SHADOW_Y = 1;
    if (src == NULL)  return NULL;
    SDL_PixelFormat *fmt = src->format;
    SDL_Surface *ret = SDL_CreateRGBSurface(src->flags, src->w + SHADOW_X, src->h + SHADOW_Y, 32,
                                            fmt->Rmask, fmt->Gmask,
                                            fmt->Bmask, fmt->Amask);
    SDL_LockSurface(src);
    SDL_LockSurface(ret);
    unsigned int dc_x = (unsigned int)(65535. / ((float)src->w + SHADOW_X+1));
    unsigned int dc_y = (unsigned int)(65535. / ((float)src->h + SHADOW_Y+1));
    unsigned int c_y  = 0;
    // OUTSIDE OF TEXT, JUST SHADOW
    for (int y=src->h + SHADOW_Y - 1; y >= src->h; --y)
    {
        unsigned int c_x  = 0;

        for (int x=SHADOW_X; x--;)
        {
            unsigned int s_alpha = iim_surface_get_alpha(src,
                    x-SHADOW_X, y-SHADOW_Y) >> 5;
            unsigned int f_alpha = 0;

            RGBA rgba = m_precomputed[s_alpha][f_alpha][c_x>>10][c_y>>12];
            iim_surface_set_rgba(ret,x,y,rgba);

            c_x += dc_x;
        }

        c_y += dc_y;
    }

    // Draw text
    for (int y=src->h - 1; y >= SHADOW_Y; --y)
    {
        unsigned int c_x  = 0;

        // OUTSIDE OF TEXT, JUST SHADOW
        for (int x=src->w + SHADOW_X - 1; x >= src->w; --x)
        {
            unsigned int s_alpha = iim_surface_get_alpha(src,
                    x-SHADOW_X, y-SHADOW_Y) >> 5;
            unsigned int f_alpha = 0;

            RGBA rgba = m_precomputed[s_alpha][f_alpha][c_x>>10][c_y>>12];
            iim_surface_set_rgba(ret,x,y,rgba);

            c_x += dc_x;
        }

        // SHADOW + TEXT
        for (int x=src->w - 1; x >= SHADOW_X; --x)
        {
            unsigned int s_alpha = iim_surface_get_alpha(src,
                    x-SHADOW_X, y-SHADOW_Y) >> 5;
            unsigned int f_alpha = iim_surface_get_alpha(src, x, y) >> 5;

            RGBA rgba = m_precomputed[s_alpha][f_alpha][c_x>>10][c_y>>12];
            iim_surface_set_rgba(ret,x,y,rgba);

            c_x += dc_x;
        }

        // OUTSIDE OF SHADOW, JUST TEXT
        for (int x=SHADOW_X - 1; x >= 0; --x)
        {
            unsigned int s_alpha = 0;
            unsigned int f_alpha = iim_surface_get_alpha(src, x, y) >> 5;

            RGBA rgba = m_precomputed[s_alpha][f_alpha][c_x>>10][c_y>>12];
            iim_surface_set_rgba(ret,x,y,rgba);

            c_x += dc_x;
        }

        c_y += dc_y;
    }

    // OUTSIDE OF SHADOW, JUST TEXT
    for (int y=SHADOW_Y; y--;)
    {
        unsigned int c_x  = 0;

        for (int x=SHADOW_X; x--;)
        {
            unsigned int s_alpha = 0;
            unsigned int f_alpha = iim_surface_get_alpha(src, x, y) >> 5;

            RGBA rgba = m_precomputed[s_alpha][f_alpha][c_x >> 10][c_y >> 12];
            iim_surface_set_rgba(ret,x,y,rgba);

            c_x += dc_x;
        }

        c_y += dc_y;
    }

    SDL_UnlockSurface(ret);
    SDL_UnlockSurface(src);
    SDL_FreeSurface(src);
    return ret;
}

