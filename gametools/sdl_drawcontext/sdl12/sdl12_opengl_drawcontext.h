#ifndef _SDL12_OPENGL_DRAWCONTEXT_H
#define _SDL12_OPENGL_DRAWCONTEXT_H

#include "opengl_drawcontext/OpenGLDrawContext.h"
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

class SDL12_OpenGLBackendUtil : public OpenGLBackendUtil
{
public:
    SDL12_OpenGLBackendUtil(DataPathManager *dataPathManager);
    virtual unsigned short * utf8ToUnicode(const char *utf8Text, unsigned short *unicodeTextBuffer, size_t unicodeTextBufferSize);
    virtual OpenGLRawImage * loadImage(ImageType type, const char *path);
    virtual void ensureContextIsActive();
    virtual void flip();
    virtual DataPathManager * getdataPathManager() const { return m_dataPathManager; }
private:
    OpenGLRawImage * loadImagePNG(ImageType type, const char *path);
    OpenGLRawImage * loadImageJPG(ImageType type, const char *path);
private:
    DataPathManager *m_dataPathManager;
};

class SDL12_OpenGL_DrawContext : public OpenGLDrawContext {
public:
    SDL12_OpenGL_DrawContext(DataPathManager *dataPathManager,
                             int w, int h, bool fullscreen,
                             const char *caption);
private:
    SDL12_OpenGLBackendUtil m_backendUtil;
    SDL_Surface *display;
};

#endif // _SDL12_OPENGL_DRAWCONTEXT_H

