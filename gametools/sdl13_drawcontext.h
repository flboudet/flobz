#ifndef _SDL13_DRAWCONTEXT_H_
#define _SDL13_DRAWCONTEXT_H_

#include "drawcontext.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

class SDL13_IosSurface : public IosSurface
{
public:
    SDL13_IosSurface(SDL_Surface *surf);
    void setAlpha(unsigned char alpha);
public:
    SDL_TextureID getTexture();
public:
    // DrawTarget implementation
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
public:
    SDL_Surface *m_surf;
    SDL_TextureID m_tex;
    SDL_Surface *m_flippedSurf;
    SDL_Surface *m_rotated[36];
};

class SDL13_IIMLibrary : public IIMLibrary
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
private:
    friend class SDL13_DrawContext;
};

class SDL13_DrawContext : public DrawContext
{
public:
    SDL13_DrawContext(int w, int h, bool fullscreen, const char *caption);
    virtual ~SDL13_DrawContext() {}
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
private:
    SDL13_IIMLibrary m_iimLib;
};

#endif // _SDL13_DRAWCONTEXT_H_

