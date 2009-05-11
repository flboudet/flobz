#ifndef _DRAWCONTEXT_H_
#define _DRAWCONTEXT_H_

#include "rgba.h"

typedef struct IosRect
{
    int x, y;
    int w, h;
} IosRect;

class IosSurface;
class IosFont;

class DrawTarget
{
public:
    virtual ~DrawTarget() {}
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y) {}
    virtual void setClipRect(IosRect *rect) {}
    virtual void fillRect(const IosRect *rect, const RGBA &color) {}
    virtual void putString(IosFont *font, int x, int y, const char *text) {}
    void putStringCenteredXY(IosFont *font, int x, int y, const char *text);
public:
    int h, w;
};

#define IOS_ALPHA_TRANSPARENT 0
#define IOS_ALPHA_OPAQUE      255
class IosSurface : public DrawTarget
{
public:
    virtual void setAlpha(unsigned char alpha) = 0;
};

enum IosFontFx
{
    Font_STD,
    Font_STORY,
    Font_GREY,
    Font_DARK
};

class IosFont
{
public:
    virtual ~IosFont() {}
    virtual int getTextWidth(const char *text) = 0;
    virtual int getHeight() = 0;
    virtual int getLineSkip() = 0;
};

class IIMLibrary
{
public:
    virtual IosSurface * create_DisplayFormat(int w, int h) = 0;
    virtual IosSurface * create_DisplayFormatAlpha(int w, int h) = 0;
    virtual IosSurface * load_Absolute_DisplayFormatAlpha(const char *path) = 0;
    virtual RGBA         getRGBA(IosSurface *surf, int x, int y) = 0;
    virtual IosSurface * shiftHue(IosSurface *surf, float hue_offset) = 0;
    virtual IosSurface * shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset) = 0;
    virtual IosSurface * shiftHSV(IosSurface *surf, float h, float s, float v) = 0;
    virtual IosSurface * setValue(IosSurface *surf, float value) = 0;
    virtual IosSurface * resizeAlpha(IosSurface *surf, int width, int height) = 0;
    virtual IosSurface * mirrorH(IosSurface *surf) = 0;
    virtual void         convertToGray(IosSurface *surf) = 0;
    virtual IosFont    * createFont(const char *path, int size, IosFontFx fx = Font_STD) = 0;
protected:
    virtual ~IIMLibrary() {}
};

class DrawContext : public DrawTarget
{
public:
    DrawContext() {}
    virtual ~DrawContext() {}
    virtual void flip() = 0;
    virtual int getHeight() const = 0;
    virtual int getWidth() const = 0;
    virtual IIMLibrary & getIIMLibrary() = 0;
};


// Implementation
inline void DrawTarget::putStringCenteredXY(IosFont *font, int x, int y, const char *text)
{
    putString(font, x - font->getTextWidth(text) / 2,
              y - font->getHeight() / 2, text);
}

#endif // _DRAWCONTEXT_H_

