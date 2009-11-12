#ifndef OPENGL_DRAWCONTEXT_H
#define OPENGL_DRAWCONTEXT_H

#include "drawcontext.h"

#include <GL/gl.h>

class OpenGL_DrawContext;

class OpenGL_IosSurface : public IosSurface
{
public:
    OpenGL_IosSurface(OpenGL_DrawContext &drawContext, bool alpha=true);
    virtual ~OpenGL_IosSurface();
    void setAlpha(unsigned char alpha);

private:
    void releaseTexture();
public:
    // DrawTarget implementation
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
private:
    bool m_alpha;
    OpenGL_DrawContext &m_drawContext;
};

class OpenGL_IIMLibrary : public IIMLibrary
{
private:
    OpenGL_IIMLibrary(OpenGL_DrawContext &drawContext) : m_drawContext(drawContext) {}
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
    OpenGL_DrawContext &m_drawContext;
    friend class OpenGL_DrawContext;
};

class OpenGL_DrawContext : public DrawContext
{
public:
    OpenGL_DrawContext(int w, int h, bool fullscreen, const char *caption);
    virtual ~OpenGL_DrawContext() {}
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
    OpenGL_IIMLibrary m_iimLib;
    friend class OpenGL_IIMLibrary;
    friend class OpenGL_IosSurface;
};

#endif // OPENGL_DRAWCONTEXT_H

