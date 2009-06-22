#ifndef _SDL_IOSFONT_H_
#define _SDL_IOSFONT_H_

#include <string>
#include <map>
#include <list>

#include "drawcontext.h"
#ifdef MACOSX
#include <SDL/SDL.h>
#include <SDL_ttf/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_ttf.h>
#endif

class SDL_IosFont : public IosFont
{
public:
    SDL_IosFont(const char *path, int size, IosFontFx fx);
    virtual ~SDL_IosFont();
    virtual int getTextWidth(const char *text);
    virtual int getHeight();
    virtual int getLineSkip();
    IosSurface * render(const char *text);
private:
    IosSurface * getFromCache(const char *text);
    void storeInCache(const char *text, IosSurface *surf);
    void precomputeFX();
    SDL_Surface *fontFX(SDL_Surface *src);
protected:
    virtual IosSurface *createSurface(SDL_Surface *src) = 0;
private:
    struct CachedSurface;
    typedef std::map<std::string, CachedSurface> CachedSurfacesMap;
    typedef std::list<CachedSurface *> CachedSurfacesList;
    struct CachedSurface {
        CachedSurface(IosSurface *surf) : surf(surf) {}
        IosSurface *surf;
        CachedSurfacesMap::iterator  mapIter;
        CachedSurfacesList::iterator listIter;
    };
private:
    IosFontFx m_fx;
    int m_height;
    TTF_Font *m_font;
    CachedSurfacesMap m_cacheMap;
    CachedSurfacesList m_cacheList;
    RGBA m_precomputed[8][8][64][16];
};

#endif // _SDL_IOSFONT_H_
