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
                                                   const char *caption)
    : m_backendUtil(dataPathManager), m_caption(caption)
{
    this->w = w; this->h = h;
    initDisplay(fullscreen);
    atexit(SDL_Quit);
    init(&m_backendUtil, w, h);
}

void SDL12_OpenGL_DrawContext::initDisplay(bool fullscreen)
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
        initDisplay(fullscreen);
    }
    /* Workaround for cursor showing in MacOS X fullscreen mode */
    SDL_ShowCursor(SDL_ENABLE);
    SDL_ShowCursor(SDL_DISABLE);
}

