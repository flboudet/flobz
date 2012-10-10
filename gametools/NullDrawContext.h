#ifndef _NULLDRAWCONTEXT_H_
#define _NULLDRAWCONTEXT_H_

#include "drawcontext.h"

class NullSurface : public IosSurface
{
public:
    NullSurface(int w, int h) {
        this->w = w;
        this->h = h;
    }
public:
	virtual bool isOpaque() const { return false; }
	virtual bool haveAbility(int ability) const { return true; }
	virtual void dropAbility(int ability) {}
    virtual RGBA readRGBA(int x, int y) {
        RGBA rgba(0, 0, 0, 0);
        return rgba;
    }
    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL) {
        return new NullSurface(w, h);
    }
    virtual IosSurface *shiftHSV(float h, float s, float v) {
        return new NullSurface(w, h);
    }
    virtual IosSurface *setValue(float value) {
        return new NullSurface(w, h);
    }
    virtual IosSurface *setAlpha(float value) {
        return new NullSurface(w, h);
    }
    virtual IosSurface * resizeAlpha(int width, int height) {
        return new NullSurface(width, height);
    }
    virtual IosSurface * mirrorH() {
        return new NullSurface(w, h);
    }
    virtual void convertToGray() {}
    // DrawTarget implementation
    virtual void setClipRect(IosRect *rect) {}
    virtual void setBlendMode(ImageBlendMode mode) {}
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y) {}
	virtual void fillRect(const IosRect *rect, const RGBA &color) {}
    virtual void putString(IosFont *font, int x, int y, const char *text, const RGBA &color) {}
};

class NullFont : public IosFont
{
public:
    virtual int getTextWidth(const char *text) { return 16; }
    virtual int getHeight()   { return 16; }
    virtual int getLineSkip() { return 16; }
};

class NullImageLibrary : public ImageLibrary
{
public:
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility) {
        return new NullSurface(w, h);
    }
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility) {
        return new NullSurface(32, 32);
    }
    virtual IosFont    * createFont(const char *path, int size) {
        return new NullFont();
    }
};

class NullDrawContext : public DrawContext
{
public:
    NullDrawContext(int w, int h) {
        this->w = w;
        this->h = h;
    }
    virtual void flip() {}
    virtual int getHeight() const { return h; }
    virtual int getWidth() const  { return w; }
    virtual ImageLibrary &getImageLibrary() {
        return m_imageLibrary;
    }
    // Query for drawcontext abilities
    virtual bool hasScaleAbility() const { return false; }
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list) {
        return IMAGE_NO_ABILITY;
    }
    // DrawTarget implementation
    virtual void setClipRect(IosRect *rect) {}
    virtual void setBlendMode(ImageBlendMode mode) {}
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {}
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y) {}
	virtual void fillRect(const IosRect *rect, const RGBA &color) {}
    virtual void putString(IosFont *font, int x, int y, const char *text) {}
private:
    NullImageLibrary m_imageLibrary;
};

#endif // _NULLDRAWCONTEXT_H_

