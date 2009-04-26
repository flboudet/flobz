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

// IosSurface implementation

SDL13_IosSurface::SDL13_IosSurface(SDL_Surface *surf)
    : m_surf(surf), m_tex(0), m_flippedSurf(NULL)
{
    w = m_surf->w;
    h = m_surf->h;
    for (int i = 0 ; i < 36 ; i++)
        m_rotated[i] = NULL;
}

SDL_TextureID SDL13_IosSurface::getTexture()
{
    if (m_tex == 0)
        m_tex = SDL_CreateTextureFromSurface(0, m_surf);
}

void SDL13_IosSurface::setAlpha(unsigned char alpha)
{
    if (m_tex != 0) {
        SDL_DestroyTexture(m_tex);
        m_tex = 0;
    }
    SDL_SetAlpha(m_surf, 0, alpha);
}

void SDL13_IosSurface::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopy_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopyFlipped_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    renderRotatedCentered_(m_surf, surf, angle, x, y);
}

void SDL13_IosSurface::setClipRect(IosRect *rect)
{
    setClipRect_(m_surf, rect);
}

void SDL13_IosSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    fillRect_(m_surf, rect, color);
}

// IIMLibrary implementation

IosSurface * SDL13_IIMLibrary::create_DisplayFormat(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgb(w, h));
}

IosSurface * SDL13_IIMLibrary::create_DisplayFormatAlpha(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgba(w, h));
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
    return new SDL13_IosSurface(retsurf);
}

RGBA SDL13_IIMLibrary::getRGBA(IosSurface *surf, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return iim_surface_get_rgba(sSurf->m_surf, x, y);
}

IosSurface * SDL13_IIMLibrary::shiftHue(IosSurface *surf, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue(sSurf->m_surf, hue_offset));
}

IosSurface * SDL13_IIMLibrary::shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL13_IosSurface *sMask = static_cast<SDL13_IosSurface *>(mask);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue_masked(sSurf->m_surf, sMask->m_surf, hue_offset));
}

IosSurface * SDL13_IIMLibrary::shiftHSV(IosSurface *surf, float h, float s, float v)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hsv(sSurf->m_surf, h, s, v));
}

IosSurface * SDL13_IIMLibrary::setValue(IosSurface *surf, float value)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_set_value(sSurf->m_surf, value));
}

IosSurface * SDL13_IIMLibrary::resizeAlpha(IosSurface *surf, int width, int height)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_resize_alpha(sSurf->m_surf, width, height));
}

IosSurface * SDL13_IIMLibrary::mirrorH(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_mirror_h(sSurf->m_surf));
}

void SDL13_IIMLibrary::convertToGray(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    iim_sdlsurface_convert_to_gray(sSurf->m_surf);
}

// DrawContext implementation

SDL13_DrawContext::SDL13_DrawContext(int w, int h, bool fullscreen, const char *caption)
{
    SDL_WindowID wid = SDL_CreateWindow("Test SDL 1.3",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        w, h,/*SDL_WINDOW_FULLSCREEN |*/ SDL_WINDOW_OPENGL);
    SDL_CreateRenderer(wid, 0, SDL_RENDERER_ACCELERATED);
    SDL_ShowWindow(wid);
    if (SDL_SelectRenderer(wid) != 0) {
        cout << "Window doesn't have a renderer" << endl;
        exit(0);
    }
    SDL_GetWindowSize(wid, &(this->w), &(this->h));
    atexit(SDL_Quit);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowTitle(wid, caption);
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

// DrawTarget implementation
void SDL13_DrawContext::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL_RenderCopy(sSurf->getTexture(),
                   IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                   IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

void SDL13_DrawContext::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    //renderCopyFlipped_(display, surf, srcRect, dstRect);
}

void SDL13_DrawContext::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    //renderRotatedCentered_(display, surf, angle, x, y);
}

void SDL13_DrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    SDL_Rect sRect;
    SDL_SetRenderDrawColor(color.red, color.green, color.blue, color.alpha);
    SDL_RenderFill(IOSRECTPTR_TO_SDL(rect, sRect));
}

void SDL13_DrawContext::setClipRect(IosRect *rect)
{
    //setClipRect_(display, rect);
}
