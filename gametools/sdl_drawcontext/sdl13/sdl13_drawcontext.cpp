#include <iostream>
#include "drawcontext.h"
#include "sdl13_drawcontext.h"
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

// DrawTarget common functions for SDL13_IosSurface and SDL13_DrawContext
inline static void renderCopy_(SDL_Surface *dest, IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL_BlitSurface(sSurf->m_surf, IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                    dest, IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

inline static void renderCopyFlipped_(SDL_Surface *dest,
                                     IosSurface *surf,
                                     IosRect *srcRect,
                                     IosRect *dstRect)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
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
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
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
class SDL13_IosFont : public SDL_IosFont
{
public:
    SDL13_IosFont(const char *path, int size, IosFontFx fx, SDL13_DrawContext &drawContext);
protected:
    virtual IosSurface *createSurface(SDL_Surface *src);
private:
    SDL13_DrawContext &m_drawContext;
};

SDL13_IosFont::SDL13_IosFont(const char *path, int size, IosFontFx fx, SDL13_DrawContext &drawContext)
  : SDL_IosFont(path, size, fx), m_drawContext(drawContext)
{}

IosSurface *SDL13_IosFont::createSurface(SDL_Surface *src)
{
    return new SDL13_IosSurface(src, m_drawContext);
}

// IosSurface implementation

SDL13_IosSurface::SDL13_IosSurface(SDL_Surface *surf, SDL13_DrawContext &drawContext, bool alpha)
    : m_alpha(alpha), m_surf(surf), m_tex(0), m_flippedSurf(NULL), m_texFlipped(0), m_drawContext(drawContext)
{
    w = m_surf->w;
    h = m_surf->h;
    for (int i = 0 ; i < 36 ; i++) {
        m_rotated[i] = NULL;
        m_texRotated[i] = 0;
    }
}

SDL13_IosSurface::~SDL13_IosSurface()
{
    releaseTexture();
    for (int i = 0 ; i < 36 ; i++)
    {
        if (m_rotated[i] != NULL)
            SDL_FreeSurface(m_rotated[i]);
        if (m_texRotated[i] != 0)
            SDL_DestroyTexture(m_texRotated[i]);
    }
    if (m_surf != NULL)
        SDL_FreeSurface(m_surf);
}

SDL_TextureID SDL13_IosSurface::getTexture()
{
    if (m_tex == 0) {
        m_tex = SDL_CreateTextureFromSurface(
            m_alpha ? SDL_PIXELFORMAT_ABGR8888/*SDL_PIXELFORMAT_ARGB8888*/ : /*SDL_PIXELFORMAT_RGB888*/SDL_PIXELFORMAT_BGR24,
            m_surf);
        SDL_SetTextureBlendMode(
            m_tex, m_alpha ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    }
    return m_tex;
}

SDL_TextureID SDL13_IosSurface::getFlippedTexture()
{
    if (m_texFlipped == 0) {
        if (m_flippedSurf == NULL) {
            m_flippedSurf = iim_sdlsurface_mirror_h(m_surf);
        }
        m_texFlipped = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ABGR8888/*SDL_PIXELFORMAT_ARGB8888*/, m_flippedSurf);
        SDL_SetTextureBlendMode(m_texFlipped, SDL_BLENDMODE_BLEND);
    }
    return m_texFlipped;
}

SDL_TextureID SDL13_IosSurface::getTexture(int angle)
{
    if (m_texRotated[angle] == 0) {
        if (!m_rotated[angle]) {
            // Generated rotated image.
            m_rotated[angle] = iim_sdlsurface_rotate(m_surf, angle * 10);
        }
        m_texRotated[angle] = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ABGR8888/*SDL_PIXELFORMAT_ARGB8888*/, m_rotated[angle]);
        SDL_SetTextureBlendMode(m_texRotated[angle], SDL_BLENDMODE_BLEND);
    }
    return m_texRotated[angle];
}

void SDL13_IosSurface::releaseTexture()
{
    if (m_tex != 0) {
        SDL_DestroyTexture(m_tex);
        m_tex = 0;
    }
}

void SDL13_IosSurface::setAlpha(unsigned char alpha)
{
    releaseTexture();
    SDL_SetAlpha(m_surf, 0, alpha);
}

void SDL13_IosSurface::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    releaseTexture();
    renderCopy_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    releaseTexture();
    renderCopyFlipped_(m_surf, surf, srcRect, dstRect);
}

void SDL13_IosSurface::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    releaseTexture();
    renderRotatedCentered_(m_surf, surf, angle, x, y);
}

void SDL13_IosSurface::setClipRect(IosRect *rect)
{
    setClipRect_(m_surf, rect);
}

void SDL13_IosSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    releaseTexture();
    fillRect_(m_surf, rect, color);
}

// IIMLibrary implementation

IosSurface * SDL13_IIMLibrary::create_DisplayFormat(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgb(w, h), m_drawContext, false);
}

IosSurface * SDL13_IIMLibrary::create_DisplayFormatAlpha(int w, int h)
{
    return new SDL13_IosSurface(iim_sdlsurface_create_rgba(w, h), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::load_Absolute_DisplayFormatAlpha(const char *path)
{
    SDL_Surface *tmpsurf, *retsurf;
    tmpsurf = IMG_Load (path);
    if (tmpsurf==NULL) {
        return NULL;
    }
#define ENABLE_WORKAROUND
#ifdef ENABLE_WORKAROUND
    retsurf = iim_sdlsurface_create_rgba(tmpsurf->w, tmpsurf->h);
    if (tmpsurf->format->Amask != 0)
        SDL_SetAlpha(tmpsurf, 0, SDL_ALPHA_OPAQUE);
    SDL_BlitSurface(tmpsurf, NULL, retsurf, NULL);
    if (retsurf==NULL) {
        perror("Texture conversion failed (is Display initialized?)\n");
        SDL_FreeSurface (tmpsurf);
        return NULL;
    }
    SDL_SetAlpha (retsurf, 0, SDL_ALPHA_OPAQUE);
    SDL_FreeSurface (tmpsurf);
#else
    retsurf = tmpsurf;
#endif
    SDL_SetSurfaceBlendMode(retsurf, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceScaleMode(retsurf, SDL_TEXTURESCALEMODE_FAST);
    return new SDL13_IosSurface(retsurf, m_drawContext);
}

RGBA SDL13_IIMLibrary::getRGBA(IosSurface *surf, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return iim_surface_get_rgba(sSurf->m_surf, x, y);
}

IosSurface * SDL13_IIMLibrary::shiftHue(IosSurface *surf, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue(sSurf->m_surf, hue_offset), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::shiftHueMasked(IosSurface *surf, IosSurface *mask, float hue_offset)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL13_IosSurface *sMask = static_cast<SDL13_IosSurface *>(mask);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hue_masked(sSurf->m_surf, sMask->m_surf, hue_offset), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::shiftHSV(IosSurface *surf, float h, float s, float v)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_shift_hsv(sSurf->m_surf, h, s, v), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::setValue(IosSurface *surf, float value)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_set_value(sSurf->m_surf, value), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::resizeAlpha(IosSurface *surf, int width, int height)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_resize_alpha(sSurf->m_surf, width, height), m_drawContext);
}

IosSurface * SDL13_IIMLibrary::mirrorH(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    return new SDL13_IosSurface(iim_sdlsurface_mirror_h(sSurf->m_surf), m_drawContext);
}

void SDL13_IIMLibrary::convertToGray(IosSurface *surf)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    iim_sdlsurface_convert_to_gray(sSurf->m_surf);
}

IosFont *SDL13_IIMLibrary::createFont(const char *path, int size, IosFontFx fx)
{
    IosFont *result = new SDL13_IosFont(path, size, fx, m_drawContext);
    DBG_FONT = result;
    return result;
}

// DrawContext implementation

SDL13_DrawContext::SDL13_DrawContext(int w, int h, bool fullscreen, const char *caption)
    : m_iimLib(*this), m_clipRectPtr(NULL)
{
    cerr << "Building DrawContext..." << endl;
    SDL_DisplayMode wishedMode, possibleMode;
    wishedMode.format = SDL_PIXELFORMAT_ARGB8888;
    wishedMode.w = w;
    wishedMode.h = h;
    wishedMode.refresh_rate = 0;
    wishedMode.driverdata = NULL;
    if (SDL_GetClosestDisplayMode(&wishedMode, &possibleMode) == NULL) {
        cout << "Impossible display mode" << endl;
        exit(0);
    }
    cerr << "Building DrawContext 2..." << endl;
    SDL_SetFullscreenDisplayMode(&possibleMode);
    cerr << "Building DrawContext 3..." << endl;
    wid = SDL_CreateWindow("Test SDL 1.3",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        w, h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
    cerr << "Building DrawContext 3,5..." << endl;
    SDL_CreateRenderer(wid, 0,
                       SDL_RENDERER_PRESENTVSYNC |
                       SDL_RENDERER_PRESENTFLIP3 |
                       SDL_RENDERER_ACCELERATED);
    cerr << "Building DrawContext 3,7..." << endl;
    SDL_ShowWindow(wid);
    if (SDL_SelectRenderer(wid) != 0) {
        cout << "Window doesn't have a renderer" << endl;
        exit(0);
    }
    cerr << "Building DrawContext 4..." << endl;
    SDL_GetWindowSize(wid, &(this->w), &(this->h));
    atexit(SDL_Quit);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetWindowTitle(wid, caption);
    SDL_SetRenderDrawBlendMode(SDL_BLENDMODE_BLEND);
    SDL_GetDisplayMode(SDL_GetCurrentVideoDisplay(), &m_mode);
    cerr << "Building DrawContext 5..." << endl;
    TTF_Init();
}

void SDL13_DrawContext::setFullScreen(bool fullscreen)
{
    cout << "SetFullScreen " << fullscreen << endl;
    SDL_SetWindowFullscreen(wid, fullscreen ? 1 : 0);
}

void SDL13_DrawContext::flip()
{
//#define BENCHMARKS
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
  SDL_RenderPresent();
}

int SDL13_DrawContext::getHeight() const
{
    return h;
}

int SDL13_DrawContext::getWidth() const
{
    return w;
}

IIMLibrary & SDL13_DrawContext::getIIMLibrary()
{
    return m_iimLib;
}

static void mySDL_RenderCopy(SDL_TextureID id, SDL_Rect *srcRect, SDL_Rect *dstRect,
                             IosRect *isrcRect, IosRect *idstRect)
{
    cout << "RenderCopy " << id << endl;
    if (srcRect == NULL)
        cout << "    srcRect: " << "NULL" << endl;
    else
        cout << "    srcRect: " << srcRect->x << "," << srcRect-> y << " ; "
             << srcRect->w << "," << srcRect->h << endl;
    if (dstRect == NULL)
        cout << "    dstRect: " << "NULL" << endl;
    else {
        cout << "    dstRect: " << dstRect->x << "," << dstRect-> y << " ; "
             << dstRect->w << "," << dstRect->h << endl;
        cout << "    idstRect:" << idstRect->x << "," << idstRect-> y << " ; "
             << idstRect->w << "," << idstRect->h << endl;
    }
    SDL_RenderCopy(id, srcRect, dstRect);
}

#define MAX(a, b) ( a < b ? b : a)
#define MIN(a, b) ( a < b ? a : b)

// DrawTarget implementation
void SDL13_DrawContext::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL_Rect sSrcRect, sDstRect;
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    if (m_clipRectPtr == NULL) {
        SDL_RenderCopy(sSurf->getTexture(),
                       IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                       IOSRECTPTR_TO_SDL(dstRect, sDstRect));
    }
    else {
        if (IOSRECTPTR_TO_SDL(srcRect, sSrcRect) == NULL) {
            sSrcRect.x = 0;
            sSrcRect.y = 0;
            sSrcRect.h = sSurf->h;
            sSrcRect.w = sSurf->w;
        }
        if (IOSRECTPTR_TO_SDL(dstRect, sDstRect) == NULL) {
            sDstRect.x = 0;
            sDstRect.y = 0;
            sDstRect.h = this->h;
            sDstRect.w = this->w;
        }
        // SrcRect computation
        SDL_Rect sSrcResult;
        m_clipRect.x -= sDstRect.x - sSrcRect.x;
        m_clipRect.y -= sDstRect.y - sSrcRect.y;
        if (! SDL_IntersectRect(&sSrcRect, &m_clipRect, &sSrcResult))
            return;
        m_clipRect.x += sDstRect.x - sSrcRect.x;
        m_clipRect.y += sDstRect.y - sSrcRect.y;
        // DstRect computation
        SDL_Rect sDstResult;
        if (! SDL_IntersectRect(&sDstRect, &m_clipRect, &sDstResult))
            return;
        SDL_RenderCopy(sSurf->getTexture(), &sSrcResult, &sDstResult);
    }
}

void SDL13_DrawContext::renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    SDL_Rect sSrcRect, sDstRect;
    SDL_RenderCopy(sSurf->getFlippedTexture(),
                   IOSRECTPTR_TO_SDL(srcRect, sSrcRect),
                   IOSRECTPTR_TO_SDL(dstRect, sDstRect));
}

void SDL13_DrawContext::renderRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    SDL13_IosSurface *sSurf = static_cast<SDL13_IosSurface *>(surf);
    while (angle < 0) angle+=8640;
    angle /= 10;
    angle %= 36;
    x -= surf->w/2;
    y -= surf->h/2;
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = surf->w;
    rect.h = surf->h;
    SDL_RenderCopy(sSurf->getTexture(angle), NULL, &rect);
}

void SDL13_DrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    SDL_Rect sRect;
    SDL_SetRenderDrawColor(color.red, color.green, color.blue, color.alpha);
    SDL_RenderFill(IOSRECTPTR_TO_SDL(rect, sRect));
}

void SDL13_DrawContext::setClipRect(IosRect *rect)
{
    m_clipRectPtr = IOSRECTPTR_TO_SDL(rect, m_clipRect);
}

void SDL13_DrawContext::putString(IosFont *font, int x, int y, const char *text)
{
    SDL13_IosFont *sFont = static_cast<SDL13_IosFont *>(font);
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
