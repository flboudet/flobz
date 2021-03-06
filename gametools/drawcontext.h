#ifndef _DRAWCONTEXT_H_
#define _DRAWCONTEXT_H_

#include "ios_fc.h"
#include "rgba.h"
#include <stdlib.h>
#include <string>

enum ImageType {
	IMAGE_RGBA = 1,
	IMAGE_RGB = 2
};

enum ImageBlendMode {
    IMAGE_COPY,
    IMAGE_BLEND
};

// Image Special Powers (Abilities)
typedef int ImageSpecialAbility;
const ImageSpecialAbility IMAGE_NO_ABILITY = 0;
const ImageSpecialAbility IMAGE_READ = 1;

struct IosRect
{
    //IosRect() {}
    //IosRect(int x, int y, int w, int h)
    //    : x(x), y(y), w(w), h(h) {}
    int x, y;
    int w, h;
    inline bool hasIntersection(IosRect &rect) const;
    inline bool hasIntersection(IosRect &rect, IosRect &result) const;
    inline bool equals(const IosRect &rect) const;
};

bool IosRect::hasIntersection(IosRect &rect) const
{
    int Amin, Amax, Bmin, Bmax;
    /* Horizontal intersection */
    Amin = x;
    Amax = Amin + w;
    Bmin = rect.x;
    Bmax = Bmin + rect.w;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    if (Amax <= Amin)
        return false;
    /* Vertical intersection */
    Amin = y;
    Amax = Amin + h;
    Bmin = rect.y;
    Bmax = Bmin + rect.h;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    if (Amax <= Amin)
        return false;
    return true;
}

bool IosRect::hasIntersection(IosRect &rect, IosRect &result) const
{
    int Amin, Amax, Bmin, Bmax;
    /* Horizontal intersection */
    Amin = x;
    Amax = Amin + w;
    Bmin = rect.x;
    Bmax = Bmin + rect.w;
    if (Bmin > Amin)
        Amin = Bmin;
    result.x = Amin;
    if(Bmax < Amax)
        Amax = Bmax;
    result.w = Amax - Amin > 0 ? Amax - Amin : 0;
    /* Vertical intersection */
    Amin = y;
    Amax = Amin + h;
    Bmin = rect.y;
    Bmax = Bmin + rect.h;
    if(Bmin > Amin)
        Amin = Bmin;
    result.y = Amin;
    if(Bmax < Amax)
        Amax = Bmax;
    result.h = Amax - Amin > 0 ? Amax - Amin : 0;
    return (result.w && result.h);
}

bool IosRect::equals(const IosRect &rect) const
{
    return ((h == rect.h) && (w == rect.w)
            && (x == rect.x) && (y == rect.y));
}

struct IosRectC
{
    IosRectC(int x, int y, int w, int h)
    {
        r.x = x; r.y = y; r.w = w; r.h = h;
    }
    IosRect r;
};

class IosSurface;
class IosFont;

class DrawTarget
{
public:
    virtual ~DrawTarget() {}

    virtual void setClipRect(IosRect *rect) = 0;
    virtual void setBlendMode(ImageBlendMode mode) = 0;

    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) = 0;
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) = 0;
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y) = 0;

	virtual void fillRect(const IosRect *rect, const RGBA &color) = 0;

    virtual void putString(IosFont *font, int x, int y, const char *text, const RGBA &color) = 0;
	virtual void putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text, const RGBA &color) { putString(font, x,y,text,color); } // Default DrawTarget have shadow disabled.
    void putStringCenteredXY(IosFont *font, int x, int y, const char *text, const RGBA &color);

public:
    int h, w;
	std::string name;
};

#define IOS_ALPHA_TRANSPARENT 0
#define IOS_ALPHA_OPAQUE      255
class IosSurface : public DrawTarget
{
public:
    IosSurface() : m_enableExceptionOnDeletion(false) {}
    ~IosSurface() {
        if (m_enableExceptionOnDeletion)
            throw ios_fc::Exception("IosSurface forbidden deletion");
    }
public:
	virtual bool isOpaque() const = 0;

	virtual bool haveAbility(int ability) const = 0;
	virtual void dropAbility(int ability) = 0;
    virtual RGBA readRGBA(int x, int y) = 0;

    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL) = 0;
    virtual IosSurface *shiftHSV(float h, float s, float v) = 0;
    virtual IosSurface *setValue(float value) = 0;
    virtual IosSurface *setAlpha(float alpha) = 0;

    virtual IosSurface * resizeAlpha(int width, int height) = 0;
    virtual IosSurface * mirrorH() = 0;
    virtual void         convertToGray() = 0;
    // Deletion protection
    void enableExceptionOnDeletion(bool enable) {m_enableExceptionOnDeletion = enable;}
private:
    bool m_enableExceptionOnDeletion;
};

class IosFont
{
public:
    virtual ~IosFont() {}
    virtual int getTextWidth(const char *text) = 0;
    virtual int getHeight() = 0;
    virtual int getLineSkip() = 0;
};

class ImageLibrary
{
public:
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility = 0) = 0;
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0) = 0;
    virtual IosFont    * createFont(const char *path, int size) = 0;
protected:
    virtual ~ImageLibrary() {}
};

/**
 * List of operations that can be performed on a surface.
 * Used for guessRequiredImageAbility()
 */
struct ImageOperationList
{
    ImageOperationList()
        : shiftHue(false), shiftHSV(false), setValue(false),
          resizeAlpha(false), mirrorH(false), convertToGray(false)
    {}
    bool shiftHue;
    bool shiftHSV;
    bool setValue;
    bool resizeAlpha;
    bool mirrorH;
    bool convertToGray;
};

class DrawContext : public DrawTarget
{
public:
    DrawContext() {}
    virtual ~DrawContext() {}
    virtual void flip() = 0;
    virtual int getHeight() const = 0;
    virtual int getWidth() const = 0;
    virtual void resize(int w, int h, bool fullscreen) = 0;
    virtual ImageLibrary &getImageLibrary() = 0;
    // Special operations
    virtual void setOffset(int offX, int offY) {}
    // Query for drawcontext abilities
    virtual bool hasScaleAbility() const { return false; }
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list) = 0;
};


// Implementation
inline void DrawTarget::putStringCenteredXY(IosFont *font, int x, int y, const char *text, const RGBA &color)
{
    putString(font, x - font->getTextWidth(text) / 2,
              y - font->getHeight() / 2, text, color);
}

#endif // _DRAWCONTEXT_H_

