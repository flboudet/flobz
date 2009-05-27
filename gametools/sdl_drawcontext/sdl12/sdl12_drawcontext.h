#ifndef _SDL12_DRAWCONTEXT_H_
#define _SDL12_DRAWCONTEXT_H_

#include "drawcontext.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class SDL12_IosSurface : public IosSurface
{
public:
    SDL12_IosSurface(SDL_Surface *surf);
    virtual ~SDL12_IosSurface();
    void setAlpha(unsigned char alpha);
public:
    // DrawTarget implementation
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
public:
    SDL_Surface *m_surf;
    SDL_Surface *m_flippedSurf;
    SDL_Surface *m_rotated[36];
};

class SDL12_IIMLibrary : public IIMLibrary
{
public:
    virtual IosSurface * create_DisplayFormat(int w, int h);
    virtual IosSurface * create_DisplayFormatAlpha(int w, int h);
    virtual IosSurface * load_Absolute_DisplayFormatAlpha(const char *path);
    virtual RGBA         getRGBA(IosSurface *surf, int x, int y);
    virtual IosSurface * shiftHue(IosSurface *surf, float hue_offset);
    virtual IosSurface * shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset);
    virtual IosSurface * shiftHSV(IosSurface *surf, float h, float s, float v);
    virtual IosSurface * setValue(IosSurface *surf, float value);
    virtual IosSurface * resizeAlpha(IosSurface *surf, int width, int height);
    virtual IosSurface * mirrorH(IosSurface *surf);
    virtual void         convertToGray(IosSurface *surf);
    virtual IosFont    * createFont(const char *path, int size, IosFontFx fx = Font_STD);
private:
    friend class SDL12_DrawContext;
};

class SDL12_DrawContext : public DrawContext
{
public:
    SDL12_DrawContext(int w, int h, bool fullscreen, const char *caption);
    virtual ~SDL12_DrawContext() {}
    virtual void flip();
    virtual int getHeight() const;
    virtual int getWidth() const;
    virtual IIMLibrary & getIIMLibrary();
    // DrawTarget implementation
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
    // Specific methods
    void setFullScreen(bool fullscreen);
private:
    SDL_Surface *display;
    SDL12_IIMLibrary m_iimLib;
};

#endif // _SDL12_DRAWCONTEXT_H_

