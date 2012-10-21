#include "GTLog.h"
#include "sdl12_opengl_drawcontext.h"

SDL12_OpenGLBackendUtil::SDL12_OpenGLBackendUtil(DataPathManager *dataPathManager)
    : StandardTextureLoader(dataPathManager)
{
}

void SDL12_OpenGLBackendUtil::ensureContextIsActive()
{
}

void SDL12_OpenGLBackendUtil::flip()
{
    SDL_GL_SwapBuffers();
}

SDL12_OpenGL_DrawContext::SDL12_OpenGL_DrawContext(DataPathManager *dataPathManager,
                                                   int w, int h, bool fullscreen,
                                                   const char *caption,
                                                   int viewportWidth, int viewportHeight)
    : m_backendUtil(dataPathManager), m_caption(caption)
{
    if (viewportWidth != 0)
        initDisplay(fullscreen, viewportWidth, viewportHeight);
    else
        initDisplay(fullscreen, w, h);
    this->w = w; this->h = h;
    atexit(SDL_Quit);
    init(&m_backendUtil, w, h, viewportWidth, viewportHeight);
}

void SDL12_OpenGL_DrawContext::initDisplay(bool fullscreen, int w, int h)
{
    // Wait for vertical retrace on swapbuffer
#ifdef SDL_GL_SWAP_CONTROL
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif
    display = SDL_SetVideoMode(w, h, 0, SDL_OPENGL|SDL_GL_DOUBLEBUFFER|(fullscreen?SDL_FULLSCREEN:0));
    if (display == NULL) {
        fprintf(stderr, "SDL_SetVideoMode error: %s\n",
                SDL_GetError());
        exit(1);
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption(m_caption.c_str(), NULL);
}

void SDL12_OpenGL_DrawContext::setFullScreen(bool fullscreen)
{
    if (SDL_WM_ToggleFullScreen(display) == 0) {
        initDisplay(fullscreen, w, h);
    }
    /* Workaround for cursor showing in MacOS X fullscreen mode */
    SDL_ShowCursor(SDL_ENABLE);
    SDL_ShowCursor(SDL_DISABLE);
}

void SDL12_OpenGL_DrawContext::resize(int w, int h, bool fullscreen) {
    OpenGLDrawContext::freeGlObjects();
    this->w = w;
    this->h = h;
    initDisplay(fullscreen, w, h);
}

