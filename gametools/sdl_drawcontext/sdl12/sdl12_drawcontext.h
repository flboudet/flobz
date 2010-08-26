#ifndef _SDL12_DRAWCONTEXT_H_
#define _SDL12_DRAWCONTEXT_H_

#include <string>
#include "drawcontext.h"
#include "DataPathManager.h"
#include <SDL/SDL.h>

class SDL12_IosSurface : public IosSurface
{
public:
    SDL12_IosSurface(SDL_Surface *surf);
    virtual ~SDL12_IosSurface();
    //void setAlpha(unsigned char alpha);
public:
    // DrawTarget implementation
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
public:
    // IosSurface implementation
    virtual bool isOpaque() const;

	virtual bool haveAbility(int ability) const;
	virtual void dropAbility(int ability);
    virtual RGBA readRGBA(int x, int y);

    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL);
    virtual IosSurface *shiftHSV(float h, float s, float v);
    virtual IosSurface *setValue(float value);

    virtual IosSurface * resizeAlpha(int width, int height);
    virtual IosSurface * mirrorH();
    virtual void         convertToGray();
public:
    SDL_Surface *m_surf;
    SDL_Surface *m_flippedSurf;
    SDL_Surface *m_rotated[36];
    ImageBlendMode m_blendMode;
};

class SDL12_ImageLibrary : public ImageLibrary
{
public:
    SDL12_ImageLibrary(DataPathManager &dataPathManager);
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility = 0);
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    virtual IosFont    * createFont(const char *path, int size, IosFontFx fx = Font_STD);
private:
    DataPathManager &m_dataPathManager;
    friend class SDL12_DrawContext;
};

class SDL12_DrawContext : public DrawContext
{
public:
    SDL12_DrawContext(DataPathManager &dataPathManager,
                      int w, int h, bool fullscreen, const char *caption);
    virtual ~SDL12_DrawContext() {}
    virtual void flip();
    virtual int getHeight() const;
    virtual int getWidth() const;
    virtual ImageLibrary & getImageLibrary();
    // DrawTarget implementation
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
    // Specific methods
    void setFullScreen(bool fullscreen);
private:
    void initDisplay(bool fullscreen);

    DataPathManager &m_dataPathManager;
    std::string m_caption;
    ImageBlendMode m_blendMode;
    SDL_Surface *display;
    SDL12_ImageLibrary m_imageLib;
};

#endif // _SDL12_DRAWCONTEXT_H_

