#ifndef _SDL13_DRAWCONTEXT_H_
#define _SDL13_DRAWCONTEXT_H_

#include <map>
#include <queue>

#include "drawcontext.h"
#include "DataPathManager.h"

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
    virtual void setBlendMode(ImageBlendMode mode);
public:
    SDL_Texture * getTexture();
    SDL_Texture * getFlippedTexture();
    SDL_Texture * getTexture(int angle);
private:
    void releaseTexture();
public:
    // DrawTarget implementation
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text, const RGBA &color);
    // IosSurface implementation
    virtual bool isOpaque() const;

	virtual bool haveAbility(int ability) const;
	virtual void dropAbility(int ability);
    virtual RGBA readRGBA(int x, int y);

    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL);
    virtual IosSurface *shiftHSV(float h, float s, float v);
    virtual IosSurface *setValue(float value);
    virtual IosSurface *setAlpha(float alpha);

    virtual IosSurface * resizeAlpha(int width, int height);
    virtual IosSurface * mirrorH();
    virtual void         convertToGray();
public:
    bool m_alpha;
    SDL_Surface *m_surf;
    SDL_Texture * m_tex;
    SDL_Surface *m_flippedSurf;
    SDL_Texture * m_texFlipped;
    SDL_Surface *m_rotated[36];
    SDL_Texture * m_texRotated[36];
private:
    SDL13_DrawContext &m_drawContext;
    ImageBlendMode m_blendMode;
};

class SDL13_IIMLibrary : public ImageLibrary
{
private:
    SDL13_IIMLibrary(DataPathManager &dataPathManager, SDL13_DrawContext &drawContext);
public:
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility = 0);
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    virtual IosFont    * createFont(const char *path, int size);
private:
    DataPathManager   &m_dataPathManager;
    SDL13_DrawContext &m_drawContext;
    friend class SDL13_DrawContext;
};

class SDL13_DrawContext : public DrawContext
{
public:
    SDL13_DrawContext(DataPathManager &dataPathManager,
                      int w, int h, bool fullscreen, const char *caption);
    virtual ~SDL13_DrawContext() {}
    virtual void flip();
    virtual int getHeight() const;
    virtual int getWidth() const;
    virtual void resize(int w, int h, bool fullscreen) {}
    virtual ImageLibrary & getImageLibrary();
    // DrawTarget implementation
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text, const RGBA &color);
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list);
    // Specific methods
    void setFullScreen(bool fullscreen);
private:
    DataPathManager &m_dataPathManager;
    SDL_Window *wid;
    SDL_Renderer *m_renderer;
    SDL13_IIMLibrary m_iimLib;
    SDL_DisplayMode m_mode;
    SDL_Rect m_clipRect, *m_clipRectPtr;
    ImageBlendMode m_blendMode;
    friend class SDL13_IIMLibrary;
    friend class SDL13_IosSurface;
};

#endif // _SDL13_DRAWCONTEXT_H_
