#ifndef _SDL_IOSFONT_H_
#define _SDL_IOSFONT_H_

#include <string>
#include <map>
#include <list>

#include "config.h"

#include "drawcontext.h"

#include <SDL/SDL.h>

#ifdef HAVE_SDL_SDL_TTF_H
#include <SDL/SDL_ttf.h>
#else
#ifdef HAVE_SDL_TTF_SDL_TTF_H
#include <SDL_ttf/SDL_ttf.h>
#endif
#endif

class SDL_IosFont : public IosFont
{
public:
    SDL_IosFont(const char *path, int size);
    virtual ~SDL_IosFont();
    virtual int getTextWidth(const char *text);
    virtual int getHeight();
    virtual int getLineSkip();
    IosSurface * render(const char *text, const RGBA &color);
private:
    IosSurface * getFromCache(const char *text, const RGBA &color);
    void storeInCache(const char *text, const RGBA &color, IosSurface *surf);
    void precomputeFX();
    SDL_Surface *fontFX(SDL_Surface *src);
protected:
    virtual IosSurface *createSurface(SDL_Surface *src) = 0;
private:
    struct CachedSurface;
    struct CachedSurfaceKey {
        CachedSurfaceKey(const char *text, const RGBA &color)
            : _text(text), _color(color) {}
        bool operator<(const CachedSurfaceKey &k) const {
            if (_text < k._text)
                return true;
            if (_text > k._text)
                return false;
            if (_color.red < k._color.red)
                return true;
            if (_color.red > k._color.red)
                return false;
            if (_color.green < k._color.green)
                return true;
            if (_color.green > k._color.green)
                return false;
            if (_color.blue < k._color.blue)
                return true;
            return false;
        }
        std::string _text;
        RGBA _color;
    };
    typedef std::map<CachedSurfaceKey, CachedSurface> CachedSurfacesMap;
    typedef std::list<CachedSurface *> CachedSurfacesList;
    struct CachedSurface {
        CachedSurface(IosSurface *surf) : surf(surf) {}
        IosSurface *surf;
        CachedSurfacesMap::iterator  mapIter;
        CachedSurfacesList::iterator listIter;
    };
private:
    int m_height;
    TTF_Font *m_font;
    CachedSurfacesMap m_cacheMap;
    CachedSurfacesList m_cacheList;
    RGBA m_precomputed[8][8][64][16];
};

#endif // _SDL_IOSFONT_H_
