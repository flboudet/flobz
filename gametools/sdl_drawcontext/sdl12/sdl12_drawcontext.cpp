#include <vector>
#include "drawcontext.h"
#include "sdl12_drawcontext.h"
#include "IosImgProcess.h"
#ifdef MACOSX
#include <SDL_image/SDL_image.h>
#else
#include <SDL_image.h>
#endif
#include "SDL_IosFont.h"
#include "ios_fc.h"
static IosFont *DBG_FONT = NULL;

using namespace std;

#define IOSRECTPTR_TO_SDL(iosrectptr, sdlrect) \
    ((iosrectptr == NULL) ? NULL : \
    ( sdlrect.h = iosrectptr->h, sdlrect.w = iosrectptr->w, \
        sdlrect.x = iosrectptr->x, sdlrect.y = iosrectptr->y, \
        &sdlrect) \
        )

int SplitString(const string& input,
               const char delimiter, vector<string>& results,
                      bool includeEmpties)
{
    int iPos   = 0;
    int newPos = -1;
    int isize  = (int)input.size();

    if (isize == 0) return 0;
    vector<int> positions;
    newPos = input.find(delimiter, 0);

    if (newPos < 0) {
        results.push_back(input);
        return 1;
    }

    int numFound = 0;
    while (newPos >= iPos) {
        numFound++;
        positions.push_back(newPos);
        iPos = newPos;
        newPos = input.find(delimiter, iPos + 1);
    }

    if (numFound == 0) {
        results.push_back(input);
        return 1;
    }

    for (unsigned int i=0; i <= positions.size(); ++i) {
        string s("");
        if (i == 0)
            s = input.substr(0, positions[0]);
        else {
            int offset = positions[i-1] + 1;
            if (i == positions.size())
                s = input.substr(offset);
            else
                s = input.substr(positions[i-1] + 1,
                        positions[i] - positions[i-1] - 1);
        }
        if (includeEmpties || (s.size() > 0)) {
            results.push_back(s);
        }
    }
    return numFound+1;
}

// DrawTarget common functions for SDL12_IosSurface and SDL12_DrawContext
inline static void renderCopy_(SDL_Surface *dest, IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    SDL_BlitSurface(sSurf->m_surf, IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                    dest, IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

inline static void renderCopyFlipped_(SDL_Surface *dest,
                                     IosSurface *surf,
                                     IosRect *srcRect,
                                     IosRect *dstRect)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    if (sSurf->m_flippedSurf == NULL) {
        sSurf->m_flippedSurf = iim_sdlsurface_mirror_h(sSurf->m_surf);
    }
    SDL_Rect sSrcRect, sDstRect;
    SDL_BlitSurface(sSurf->m_flippedSurf, IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                    dest, IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

inline static void renderRotatedCentered_(SDL_Surface *dest,
                                          IosSurface *surf,
                                          int angle, int x, int y)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    while (angle < 0) angle+=8640;
    angle /= 10;
    angle %= 36;
    if (!sSurf->m_rotated[angle]) {
        // Generated rotated image.
        sSurf->m_rotated[angle] = iim_sdlsurface_rotate(sSurf->m_surf, angle * 10);
    }
    x -= surf->w/2;
    y -= surf->h/2;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surf->w;
    rect.h = surf->h;
    SDL_BlitSurface(sSurf->m_rotated[angle], NULL, dest, &rect);
}

inline static void setClipRect_(SDL_Surface *surf, IosRect *rect)
{
    SDL_Rect sRect;
    SDL_SetClipRect(surf, IOSRECTPTR_TO_SDL(rect, sRect));
}

inline static void fillRect_(SDL_Surface *surf, const IosRect *rect, const RGBA &color)
{
    SDL_Rect srect;
    SDL_FillRect(surf,
                 IOSRECTPTR_TO_SDL(rect, srect),
                 (surf->format->Rmask & ((color.red)      |
					 (color.red<<8)   |
					 (color.red<<16)  |
					 (color.red<<24))) |
                 (surf->format->Gmask & ((color.green)    |
					 (color.green<<8) |
					 (color.green<<16)|
					 (color.green<<24)))|
                 (surf->format->Bmask & ((color.blue)    |
					 (color.blue<<8) |
					 (color.blue<<16)|
					 (color.blue<<24)))|
                 (surf->format->Amask & ((color.alpha)    |
					 (color.alpha<<8) |
					 (color.alpha<<16)|
					 (color.alpha<<24))));
}

// IosFont implementation
class SDL12_IosFont : public SDL_IosFont
{
public:
    SDL12_IosFont(const char *path, int size, IosFontFx fx);
protected:
    virtual IosSurface *createSurface(SDL_Surface *src);
};

SDL12_IosFont::SDL12_IosFont(const char *path, int size, IosFontFx fx)
  : SDL_IosFont(path, size, fx)
{}

IosSurface *SDL12_IosFont::createSurface(SDL_Surface *src)
{
    return new SDL12_IosSurface(src);
}

// IosSurface implementation

SDL12_IosSurface::SDL12_IosSurface(SDL_Surface *surf)
    : m_surf(surf), m_flippedSurf(NULL)
{
    w = m_surf->w;
    h = m_surf->h;
    for (int i = 0 ; i < 36 ; i++)
        m_rotated[i] = NULL;
}

SDL12_IosSurface::~SDL12_IosSurface()
{
    for (int i = 0 ; i < 36 ; i++)
        if (m_rotated[i] != NULL)
            SDL_FreeSurface(m_rotated[i]);
    if (m_surf != NULL)
        SDL_FreeSurface(m_surf);
}

void SDL12_IosSurface::setAlpha(unsigned char alpha)
{
    SDL_SetAlpha(m_surf, 0, alpha);
}

void SDL12_IosSurface::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopy_(m_surf, surf, srcRect, dstRect);
}

void SDL12_IosSurface::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopyFlipped_(m_surf, surf, srcRect, dstRect);
}

void SDL12_IosSurface::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    renderRotatedCentered_(m_surf, surf, angle, x, y);
}

void SDL12_IosSurface::setClipRect(IosRect *rect)
{
    setClipRect_(m_surf, rect);
}

void SDL12_IosSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    fillRect_(m_surf, rect, color);
}

// IIMLibrary implementation

IosSurface * SDL12_IIMLibrary::create_DisplayFormat(int w, int h)
{
    return new SDL12_IosSurface(iim_sdlsurface_create_rgb(w, h));
}

IosSurface * SDL12_IIMLibrary::create_DisplayFormatAlpha(int w, int h)
{
    return new SDL12_IosSurface(iim_sdlsurface_create_rgba(w, h));
}

IosSurface * SDL12_IIMLibrary::load_Absolute_DisplayFormatAlpha(const char *path)
{
    SDL_Surface *tmpsurf, *retsurf;
    tmpsurf = IMG_Load (path);
    if (tmpsurf==NULL) {
        return NULL;
    }
    retsurf = SDL_DisplayFormatAlpha (tmpsurf);
    if (retsurf==NULL) {
        perror("Texture conversion failed (is Display initialized?)\n");
        SDL_FreeSurface (tmpsurf);
        return NULL;
    }
    SDL_SetAlpha (retsurf, SDL_SRCALPHA | (useGL?0:SDL_RLEACCEL), SDL_ALPHA_OPAQUE);
    SDL_FreeSurface (tmpsurf);
    return new SDL12_IosSurface(retsurf);
}

RGBA SDL12_IIMLibrary::getRGBA(IosSurface *surf, int x, int y)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return iim_surface_get_rgba(sSurf->m_surf, x, y);
}

IosSurface * SDL12_IIMLibrary::shiftHue(IosSurface *surf, float hue_offset)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return new SDL12_IosSurface(iim_sdlsurface_shift_hue(sSurf->m_surf, hue_offset));
}

IosSurface * SDL12_IIMLibrary::shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    SDL12_IosSurface *sMask = static_cast<SDL12_IosSurface *>(mask);
    return new SDL12_IosSurface(iim_sdlsurface_shift_hue_masked(sSurf->m_surf, sMask->m_surf, hue_offset));
}

IosSurface * SDL12_IIMLibrary::shiftHSV(IosSurface *surf, float h, float s, float v)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return new SDL12_IosSurface(iim_sdlsurface_shift_hsv(sSurf->m_surf, h, s, v));
}

IosSurface * SDL12_IIMLibrary::setValue(IosSurface *surf, float value)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return new SDL12_IosSurface(iim_sdlsurface_set_value(sSurf->m_surf, value));
}

IosSurface * SDL12_IIMLibrary::resizeAlpha(IosSurface *surf, int width, int height)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return new SDL12_IosSurface(iim_sdlsurface_resize_alpha(sSurf->m_surf, width, height));
}

IosSurface * SDL12_IIMLibrary::mirrorH(IosSurface *surf)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    return new SDL12_IosSurface(iim_sdlsurface_mirror_h(sSurf->m_surf));
}

void SDL12_IIMLibrary::convertToGray(IosSurface *surf)
{
    SDL12_IosSurface *sSurf = static_cast<SDL12_IosSurface *>(surf);
    iim_sdlsurface_convert_to_gray(sSurf->m_surf);
}

IosFont *SDL12_IIMLibrary::createFont(const char *path, int size, IosFontFx fx)
{
    IosFont *result = new SDL12_IosFont(path, size, fx);
    DBG_FONT = result;
    return result;
}

// DrawContext implementation

SDL12_DrawContext::SDL12_DrawContext(int w, int h, bool fullscreen, const char *caption)
{
    display = SDL_SetVideoMode(w, h, 0, SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
    if (display == NULL) {
        fprintf(stderr, "SDL_SetVideoMode error: %s\n",
                SDL_GetError());
        exit(1);
    }
    this->h = display->h;
    this->w = display->w;
    atexit(SDL_Quit);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption(caption, NULL);
    TTF_Init();
}

void SDL12_DrawContext::setFullScreen(bool fullscreen)
{
    SDL_WM_ToggleFullScreen(display);
    /* Workaround for cursor showing in MacOS X fullscreen mode */
    SDL_ShowCursor(SDL_ENABLE);
    SDL_ShowCursor(SDL_DISABLE);
}

void SDL12_DrawContext::flip()
{
#define BENCHMARKS
#ifdef BENCHMARKS
  static double nFrames = 0.0;
  static double t0 = 0.0;
  static char fps[] = "FPS: .....   ";
  static double minFPS = 100000.0;
  static double t1 = 0.0;

  double t2 = ios_fc::getTimeMs();
  double curFPS = 1000.0 / (t2 - t1);
  if (curFPS < minFPS) minFPS = curFPS;
  t1 = t2;

  if (nFrames > 60.0) {
      sprintf(fps, "FPS: %3.1f >> %3.1f", (1000.0 * nFrames / (t2-t0)), minFPS);
      t0 = t2;
      nFrames = 0.0;
      minFPS = 1000000.0;
  }

  nFrames += 1.0;
  if (DBG_FONT != NULL) {
      setClipRect(NULL);
      putString (DBG_FONT, 16, 16, fps);
  }
#endif
  SDL_Flip(display);
}

int SDL12_DrawContext::getHeight() const
{
    return display->h;
}

int SDL12_DrawContext::getWidth() const
{
    return display->w;
}

IIMLibrary & SDL12_DrawContext::getIIMLibrary()
{
    return m_iimLib;
}

// DrawTarget implementation
void SDL12_DrawContext::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopy_(display, surf, srcRect, dstRect);
}

void SDL12_DrawContext::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    renderCopyFlipped_(display, surf, srcRect, dstRect);
}

void SDL12_DrawContext::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    renderRotatedCentered_(display, surf, angle, x, y);
}

void SDL12_DrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    fillRect_(display, rect, color);
}

void SDL12_DrawContext::setClipRect(IosRect *rect)
{
    setClipRect_(display, rect);
}

void SDL12_DrawContext::putString(IosFont *font, int x, int y, const char *text)
{
    SDL12_IosFont *sFont = static_cast<SDL12_IosFont *>(font);
        vector<string> lines;
    int num = SplitString(text, '\n', lines, true);
    int skip = sFont->getLineSkip();
    for (vector<string>::iterator iter = lines.begin() ;
         iter != lines.end() ; iter++) {
        if (strcmp(iter->c_str(), "") == 0)
            continue;
        IosSurface *surf = sFont->render(iter->c_str());
        IosRect dstRect = { x, y, surf->w, surf->h };
        renderCopy(surf, NULL, &dstRect);
        y += skip;
    }
}

