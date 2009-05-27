#ifndef _SDL13_DRAWCONTEXT_H_
#define _SDL13_DRAWCONTEXT_H_

#include <map>
#include <queue>

#include "drawcontext.h"

#ifdef MACOSX
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

class SDL13_DrawContext;

class SDL13_IosSurface : public IosSurface
{
public:
    SDL13_IosSurface(SDL_Surface *surf, SDL13_DrawContext &drawContext, bool alpha=true);
    virtual ~SDL13_IosSurface();
    void setAlpha(unsigned char alpha);
public:
    SDL_TextureID getTexture();
    SDL_TextureID getFlippedTexture();
    SDL_TextureID getTexture(int angle);
private:
    void releaseTexture();
public:
    // DrawTarget implementation
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
public:
    bool m_alpha;
    SDL_Surface *m_surf;
    SDL_TextureID m_tex;
    SDL_Surface *m_flippedSurf;
    SDL_TextureID m_texFlipped;
    SDL_Surface *m_rotated[36];
    SDL_TextureID m_texRotated[36];
private:
    SDL13_DrawContext &m_drawContext;
};

class SDL13_IIMLibrary : public IIMLibrary
{
private:
    SDL13_IIMLibrary(SDL13_DrawContext &drawContext) : m_drawContext(drawContext) {}
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
    SDL13_DrawContext &m_drawContext;
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
    virtual void putString(IosFont *font, int x, int y, const char *text);
    // Specific methods
    void setFullScreen(bool fullscreen);
private:
    SDL_WindowID wid;
    SDL13_IIMLibrary m_iimLib;
    SDL_DisplayMode m_mode;
    SDL_Rect m_clipRect, *m_clipRectPtr;
    friend class SDL13_IIMLibrary;
    friend class SDL13_IosSurface;
};

#endif // _SDL13_DRAWCONTEXT_H_

