#include <iostream>
#include "drawcontext.h"
#include "sdl13_drawcontext.h"
#include "IosImgProcess.h"
#ifdef MACOSX
#include <SDL_image/SDL_image.h>
#else
#include <SDL_image.h>
#endif

using namespace std;

#define IOSRECTPTR_TO_SDL(iosrectptr, sdlrect) \
    ((iosrectptr == NULL) ? NULL : \
    ( sdlrect.h = iosrectptr->h, sdlrect.w = iosrectptr->w, \
        sdlrect.x = iosrectptr->x, sdlrect.y = iosrectptr->y, \
        &sdlrect) \
        )

// DrawTarget common functions for SDL13_IosSurface and SDL13_DrawContext
inline static void renderCopy_(SDL_Surface *dest, IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL_BlitSurface(sSurf->m_surf, IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                    dest, IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

inline static void renderCopyFlipped_(SDL_Surface *dest,
                                     IosSurface *surf,
                                     IosRect *srcRect,
                                     IosRect *dstRect)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    if (sSurf->m_flippedSurf == NULL) {
        sSurf->m_flippedSurf = iim_sdlsurface_mirror_h(sSurf->m_surf);
    }
    SDL_Rect sSrcRect, sDstRect;
    SDL_BlitSurface(sSurf->m_flippedSurf, IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                    dest, IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

inline static void renderRotatedCentered_(SDL_Surface *dest,
                                          IosSurface *surf,
                                          int angle, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    while (angle < 0) angle+=8640;
    angle /= 10;
    angle %= 36;
    if (!sSurf->m_rotated[angle]) {
        // Generated rotated image.
        sSurf->m_rotated[angle] = iim_sdlsurface_rotate(sSurf->m_surf, angle * 10);
    }
    x -= surf->w/2;
    y -= surf->h/2;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surf->w;
    rect.h = surf->h;
    SDL_BlitSurface(sSurf->m_rotated[angle], NULL, dest, &rect);
}

inline static void setClipRect_(SDL_Surface *surf, IosRect *rect)
{
    SDL_Rect sRect;
    SDL_SetClipRect(surf, IOSRECTPTR_TO_SDL(rect, sRect));
}

inline static void fillRect_(SDL_Surface *surf, const IosRect *rect, const RGBA &color)
{
    SDL_Rect srect;
    SDL_FillRect(surf,
                 IOSRECTPTR_TO_SDL(rect, srect),
                 (surf->format->Rmask & ((color.red)      |
					 (color.red<<8)   |
					 (color.red<<16)  |
					 (color.red<<24))) |
                 (surf->format->Gmask & ((color.green)    |
					 (color.green<<8) |
					 (color.green<<16)|
					 (color.green<<24)))|
                 (surf->format->Bmask & ((color.blue)    |
					 (color.blue<<8) |
					 (color.blue<<16)|
					 (color.blue<<24)))|
                 (surf->format->Amask & ((color.alpha)    |
					 (color.alpha<<8) |
					 (color.alpha<<16)|
					 (color.alpha<<24))));
}

// IosFont implementation
SDL13_IosFont::SDL13_IosFont(const char *path, int size, IosFontFx fx, SDL13_DrawContext &drawContext)
    : m_fx(fx), m_height(size), m_drawContext(drawContext)
{
    m_font = TTF_OpenFont(path, size);
    m_height = TTF_FontHeight(m_font);
    precomputeFX();
}

SDL13_IosFont::~SDL13_IosFont()
{
    TTF_CloseFont(m_font);
    for (CachedSurfacesMap::iterator iter = m_cacheMap.begin() ;
         iter != m_cacheMap.end() ; iter++) {
        delete (iter->second);
    }
}

int SDL13_IosFont::getTextWidth(const char *text)
{
    int w,h;
    TTF_SizeUTF8(m_font, text, &w, &h);
    return w;
}

int SDL13_IosFont::getHeight()
{
    return m_height;
}

SDL13_IosSurface * SDL13_IosFont::render(const char *text)
{
    SDL13_IosSurface *result = getFromCache(text);
    if (result == NULL) {
        static const SDL_Colour white = {255,255,255};
        SDL_Surface *image = TTF_RenderUTF8_Blended(m_font, text, white);
        image = fontFX(image);
        result = new SDL13_IosSurface(image, m_drawContext);
        storeInCache(text, result);
    }
    return result;
}

SDL13_IosSurface * SDL13_IosFont::getFromCache(const char *text)
{
    CachedSurfacesMap::iterator iter = m_cacheMap.find(text);
    if (iter == m_cacheMap.end())
        return NULL;
    return iter->second;
}

void SDL13_IosFont::storeInCache(const char *text, SDL13_IosSurface *surf)
{
    m_cacheMap[text] = surf;
}

// Precompute the font effect function values from discretized alpha and positions.
void SDL13_IosFont::precomputeFX()
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
SDL_Surface *SDL13_IosFont::fontFX(SDL_Surface *src)
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

// IosSurface implementation

SDL13_IosSurface::SDL13_IosSurface(SDL_Surface *surf, SDL13_DrawContext &drawContext)
    : m_surf(surf), m_tex(0), m_flippedSurf(NULL), m_texFlipped(0), m_drawContext(drawContext)
{
    w = m_surf->w;
    h = m_surf->h;
    for (int i = 0 ; i < 36 ; i++) {
        m_rotated[i] = NULL;
        m_texRotated[i] = 0;
    }
}

SDL13_IosSurface::~SDL13_IosSurface()
{
    releaseTexture();
    for (int i = 0 ; i < 36 ; i++)
    {
        if (m_rotated[i] != NULL)
            SDL_FreeSurface(m_rotated[i]);
        if (m_texRotated[i] != 0)
            SDL_DestroyTexture(m_texRotated[i]);
    }
    if (m_surf != NULL)
        SDL_FreeSurface(m_surf);
}

SDL_TextureID SDL13_IosSurface::getTexture()
{
    if (m_tex == 0) {
        //if (! SDL_ISPIXELFORMAT_ALPHA(m_surf->format)) {
        //    cout << "No alpha!" << endl;
        //}
        //if (SDL_ISPIXELFORMAT_INDEXED(m_surf->format)) {
        //    cout << "Indexed!" << endl;
        //}
        m_tex = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ARGB8888, m_surf);
        SDL_SetTextureBlendMode(m_tex, SDL_BLENDMODE_BLEND);
    }
    return m_tex;
}

SDL_TextureID SDL13_IosSurface::getFlippedTexture()
{
    if (m_texFlipped == 0) {
        if (m_flippedSurf == NULL) {
            m_flippedSurf = iim_sdlsurface_mirror_h(m_surf);
        }
        m_texFlipped = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ARGB8888, m_flippedSurf);
        SDL_SetTextureBlendMode(m_texFlipped, SDL_BLENDMODE_BLEND);
    }
    return m_texFlipped;
}

SDL_TextureID SDL13_IosSurface::getTexture(int angle)
{
    if (m_texRotated[angle] == 0) {
        if (!m_rotated[angle]) {
            // Generated rotated image.
            m_rotated[angle] = iim_sdlsurface_rotate(m_surf, angle * 10);
        }
        m_texRotated[angle] = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ARGB8888, m_rotated[angle]);
        SDL_SetTextureBlendMode(m_texRotated[angle], SDL_BLENDMODE_BLEND);
    }
    return m_texRotated[angle];
}

void SDL13_IosSurface::releaseTexture()
{
    if (m_tex != 0) {
        SDL_DestroyTexture(m_tex);
        m_tex = 0;
    }
}

void SDL13_IosSurface::setAlpha(unsigned char alpha)
{
    releaseTexture();
    SDL_SetAlpha(m_surf, 0, alpha);
}

void SDL13_IosSurface::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    releaseTexture();
    renderCopy_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    releaseTexture();
    renderCopyFlipped_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    releaseTexture();
    renderRotatedCentered_(m_surf, surf, angle, x, y);
}

void SDL13_IosSurface::setClipRect(IosRect *rect)
{
    setClipRect_(m_surf, rect);
}

void SDL13_IosSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    releaseTexture();
    fillRect_(m_surf, rect, color);
}

// IIMLibrary implementation

IosSurface * SDL13_IIMLibrary::create_DisplayFormat(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgb(w, h), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::create_DisplayFormatAlpha(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgba(w, h), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::load_Absolute_DisplayFormatAlpha(const char *path)
{
    SDL_Surface *tmpsurf, *retsurf;
    tmpsurf = IMG_Load (path);
    if (tmpsurf==NULL) {
        return NULL;
    }
    retsurf = tmpsurf;
    if (retsurf==NULL) {
        perror("Texture conversion failed (is Display initialized?)\n");
        SDL_FreeSurface (tmpsurf);
        return NULL;
    }
    // SDL_SetAlpha (retsurf, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
    // SDL_FreeSurface (tmpsurf);
    return new SDL13_IosSurface(retsurf, m_drawContext);
}

RGBA SDL13_IIMLibrary::getRGBA(IosSurface *surf, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return iim_surface_get_rgba(sSurf->m_surf, x, y);
}

IosSurface * SDL13_IIMLibrary::shiftHue(IosSurface *surf, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue(sSurf->m_surf, hue_offset), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL13_IosSurface *sMask = static_cast<SDL13_IosSurface *>(mask);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue_masked(sSurf->m_surf, sMask->m_surf, hue_offset), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::shiftHSV(IosSurface *surf, float h, float s, float v)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hsv(sSurf->m_surf, h, s, v), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::setValue(IosSurface *surf, float value)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_set_value(sSurf->m_surf, value), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::resizeAlpha(IosSurface *surf, int width, int height)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_resize_alpha(sSurf->m_surf, width, height), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::mirrorH(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_mirror_h(sSurf->m_surf), m_drawContext);
}

void SDL13_IIMLibrary::convertToGray(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    iim_sdlsurface_convert_to_gray(sSurf->m_surf);
}

IosFont *SDL13_IIMLibrary::createFont(const char *path, int size, IosFontFx fx)
{
    return new SDL13_IosFont(path, size, fx, m_drawContext);
}

// DrawContext implementation

SDL13_DrawContext::SDL13_DrawContext(int w, int h, bool fullscreen, const char *caption)
    : m_iimLib(*this), m_clipRectPtr(NULL)
{
    SDL_DisplayMode wishedMode, possibleMode;
    wishedMode.format = SDL_PIXELFORMAT_ARGB8888;
    wishedMode.w = 640;
    wishedMode.h = 480;
    wishedMode.refresh_rate = 0;
    wishedMode.driverdata = NULL;
    if (SDL_GetClosestDisplayMode(&wishedMode, &possibleMode) == NULL) {
        cout << "Impossible display mode" << endl;
        exit(0);
    }
    SDL_SetFullscreenDisplayMode(&possibleMode);
    SDL_WindowID wid = SDL_CreateWindow("Test SDL 1.3",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        w, h,/*SDL_WINDOW_FULLSCREEN |*/ SDL_WINDOW_OPENGL);
    SDL_CreateRenderer(wid, 1, SDL_RENDERER_ACCELERATED);
    SDL_ShowWindow(wid);
    if (SDL_SelectRenderer(wid) != 0) {
        cout << "Window doesn't have a renderer" << endl;
        exit(0);
    }
    SDL_GetWindowSize(wid, &(this->w), &(this->h));
    atexit(SDL_Quit);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowTitle(wid, caption);
    SDL_SetRenderDrawBlendMode(SDL_BLENDMODE_BLEND);
    SDL_GetDisplayMode(SDL_GetCurrentVideoDisplay(), &m_mode);

    TTF_Init();
}

void SDL13_DrawContext::flip()
{
    SDL_RenderPresent();
}

int SDL13_DrawContext::getHeight() const
{
    return h;
}

int SDL13_DrawContext::getWidth() const
{
    return w;
}

IIMLibrary & SDL13_DrawContext::getIIMLibrary()
{
    return m_iimLib;
}

static void mySDL_RenderCopy(SDL_TextureID id, SDL_Rect *srcRect, SDL_Rect *dstRect,
                             IosRect *isrcRect, IosRect *idstRect)
{
    cout << "RenderCopy " << id << endl;
    if (srcRect == NULL)
        cout << "    srcRect: " << "NULL" << endl;
    else
        cout << "    srcRect: " << srcRect->x << "," << srcRect-> y << " ; "
             << srcRect->w << "," << srcRect->h << endl;
    if (dstRect == NULL)
        cout << "    dstRect: " << "NULL" << endl;
    else {
        cout << "    dstRect: " << dstRect->x << "," << dstRect-> y << " ; "
             << dstRect->w << "," << dstRect->h << endl;
        cout << "    idstRect:" << idstRect->x << "," << idstRect-> y << " ; "
             << idstRect->w << "," << idstRect->h << endl;
    }
    SDL_RenderCopy(id, srcRect, dstRect);
}

#define MAX(a, b) ( a < b ? b : a)
#define MIN(a, b) ( a < b ? a : b)

// DrawTarget implementation
void SDL13_DrawContext::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    if (m_clipRectPtr == NULL) {
        SDL_RenderCopy(sSurf->getTexture(),
                       IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                       IOSRECTPTR_TO_SDL(dstRect, sDstRect));
    }
    else {
        if (IOSRECTPTR_TO_SDL(srcRect, sSrcRect) == NULL) {
            sSrcRect.x = 0;
            sSrcRect.y = 0;
            sSrcRect.h = sSurf->h;
            sSrcRect.w = sSurf->w;
        }
        if (IOSRECTPTR_TO_SDL(dstRect, sDstRect) == NULL) {
            sDstRect.x = 0;
            sDstRect.y = 0;
            sDstRect.h = this->h;
            sDstRect.w = this->w;
        }
        // SrcRect computation
        SDL_Rect sSrcResult;
        m_clipRect.x -= sDstRect.x - sSrcRect.x;
        m_clipRect.y -= sDstRect.y - sSrcRect.y;
        if (! SDL_IntersectRect(&sSrcRect, &m_clipRect, &sSrcResult))
            return;
        m_clipRect.x += sDstRect.x - sSrcRect.x;
        m_clipRect.y += sDstRect.y - sSrcRect.y;
        // DstRect computation
        SDL_Rect sDstResult;
        if (! SDL_IntersectRect(&sDstRect, &m_clipRect, &sDstResult))
            return;
        SDL_RenderCopy(sSurf->getTexture(), &sSrcResult, &sDstResult);
    }
}

void SDL13_DrawContext::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL_Rect sSrcRect, sDstRect;
    SDL_RenderCopy(sSurf->getFlippedTexture(),
                   IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                   IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

void SDL13_DrawContext::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    while (angle < 0) angle+=8640;
    angle /= 10;
    angle %= 36;
    x -= surf->w/2;
    y -= surf->h/2;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surf->w;
    rect.h = surf->h;
    SDL_RenderCopy(sSurf->getTexture(angle), NULL, &rect);
}

void SDL13_DrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    SDL_Rect sRect;
    SDL_SetRenderDrawColor(color.red, color.green, color.blue, color.alpha);
    SDL_RenderFill(IOSRECTPTR_TO_SDL(rect, sRect));
}

void SDL13_DrawContext::setClipRect(IosRect *rect)
{
    m_clipRectPtr = IOSRECTPTR_TO_SDL(rect, m_clipRect);
}

void SDL13_DrawContext::putString(IosFont *font, int x, int y, const char *text)
{
    if (strcmp(text, "") == 0)
        return;
    SDL13_IosFont *sFont = static_cast<SDL13_IosFont *>(font);
    SDL13_IosSurface *surf = sFont->render(text);
    IosRect dstRect = { x, y, surf->w, surf->h };
    renderCopy(surf, NULL, &dstRect);
}
