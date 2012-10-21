#ifndef _SDL12_OPENGL_DRAWCONTEXT_H
#define _SDL12_OPENGL_DRAWCONTEXT_H

#include "opengl_drawcontext/OpenGLDrawContext.h"
#include "opengl_drawcontext/StandardTextureLoader.h"
#include "DataPathManager.h"
#include <SDL/SDL.h>

class SDL12_SurfaceRawImage : public OpenGLRawImage
{
public:
    SDL12_SurfaceRawImage(SDL_Surface *s);
    virtual ~SDL12_SurfaceRawImage();
private:
    SDL_Surface *m_s;
};

class SDL12_OpenGLBackendUtil : public StandardTextureLoader
{
public:
    SDL12_OpenGLBackendUtil(DataPathManager *dataPathManager);
    virtual void ensureContextIsActive();
    virtual void flip();
};

class SDL12_OpenGL_DrawContext : public OpenGLDrawContext {
public:
    SDL12_OpenGL_DrawContext(DataPathManager *dataPathManager,
                             int w, int h, bool fullscreen,
                             const char *caption,
                             int viewportWidth = 0, int viewportHeight = 0);
    // Specific methods
    void setFullScreen(bool fullscreen);
    void resize(int w, int h, bool fullscreen);
private:
    void initDisplay(bool fullScreen, int w, int h);
private:
    SDL12_OpenGLBackendUtil m_backendUtil;
    SDL_Surface *display;
    std::string m_caption;
};

#endif // _SDL12_OPENGL_DRAWCONTEXT_H

