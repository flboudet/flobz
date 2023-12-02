//
//  OpenGLDrawContext.h
//  flobopop
//
//  Created by Florent Boudet on 13/04/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _OPENGL_DRAW_CONTEXT_H_
#define _OPENGL_DRAW_CONTEXT_H_

#include <memory>
#include "config.h"
#define GL_GLEXT_PROTOTYPES

#ifdef HAVE_GL_GL_H
#include <GL/gl.h>
#elif defined (HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
#elif defined (IOS)
#include <OpenGLES/ES1/gl.h>
#elif defined (ANDROID)
#include <GLES/gl.h>
#endif

#ifdef HAVE_GL_GLEXT_H
#include <GL/glext.h>
#elif defined (HAVE_OPENGL_GLEXT_H)
#include <OpenGL/glext.h>
#elif defined (IOS)
#include <OpenGLES/ES1/glext.h>
#elif defined (ANDROID)
#include <GLES/glext.h>
#endif

#include "drawcontext.h"
#include "DataPathManager.h"

class OpenGLDrawContext;

class OpenGLRawImage
{
public:
    OpenGLRawImage(GLint format, int width, int height, int p2width, int p2height, void *data, size_t dataSize=0, bool compressed = false)
    : format(format), width(width), height(height), p2width(p2width), p2height(p2height), data(data), dataSize(dataSize), compressed(compressed)
    {}
    OpenGLRawImage() {}
    virtual ~OpenGLRawImage() {}
    GLint format;
    int width, height;
    int p2width, p2height;
    void *data;
    size_t dataSize;
    bool compressed;
};

class MallocedOpenGLRawImage : public OpenGLRawImage
{
public:
    MallocedOpenGLRawImage(GLint format, int width, int height, int p2width, int p2height, void *data, size_t dataSize=0, bool compressed = false)
    : OpenGLRawImage(format, width, height, p2width, p2height, data, dataSize, compressed)
    {}
    virtual ~MallocedOpenGLRawImage()
    { free(data); }
};

class OpenGLBackendUtil
{
public:
    OpenGLBackendUtil() : m_defaultFBO(0) {}
    virtual unsigned short * utf8ToUnicode(const char *utf8Text, unsigned short *unicodeTextBuffer, size_t unicodeTextBufferSize) = 0;
    virtual OpenGLRawImage * loadImage(ImageType type, const char *path) = 0;
    virtual void ensureContextIsActive() = 0;
    virtual void flip() = 0;

    // Eventually, to be removed because data access
    // should be encapsulated by the OpenGLBackendUtil...
    // ...but this is not the case for fonts yet!
    virtual DataPathManager * getdataPathManager() const = 0;

    static GLenum toGL(ImageType type) {
        switch (type) {
            case IMAGE_RGB:
                return GL_RGB;
            case IMAGE_RGBA:
                return GL_RGBA;
        }
        return GL_RGBA;
    }
    GLuint m_defaultFBO;
};

class OpenGLImageLibrary : public ImageLibrary
{
public:
    OpenGLImageLibrary(OpenGLDrawContext *owner, OpenGLBackendUtil *backendUtil);
    virtual ~OpenGLImageLibrary() {}
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility abilities);
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility abilities);
    virtual IosFont    * createFont(const char *path, int size);
private:
    OpenGLDrawContext *m_owner;
    OpenGLBackendUtil *m_backendUtil;
};

class OpenGLDrawContext : public DrawContext
{
public:
    OpenGLDrawContext() : m_clipRectPtr(NULL) {}
    virtual ~OpenGLDrawContext() {}
    void init(OpenGLBackendUtil *backendUtil, int width, int height, int viewportWidth = 0, int viewportHeight = 0);
public:
    virtual int getHeight() const;
    virtual int getWidth() const;
    virtual ImageLibrary & getImageLibrary();
    virtual void flip();
public:
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode) {}
    virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text, const RGBA &color);
	virtual void putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text, const RGBA &color);
    void putStringCenteredXY(IosFont *font, int x, int y, const char *text, const RGBA &color);
    // Special operations
    virtual void setOffset(int offX, int offY);
	// Query for drawcontext abilities
    virtual bool hasScaleAbility() const { return true; }
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list);
public:
    // Performance measurement
    void startFrame();
    void endFrame();
    // Special properties
    float getPixelRatioX() const { return (float)w / (float)m_viewportWidth; }
    float getPixelRatioY() const { return (float)h / (float)m_viewportHeight; }

    void unrefGlObjects();
    void freeGlObjects();
private:
    OpenGLBackendUtil *m_backendUtil;
private:
    std::unique_ptr<OpenGLImageLibrary> iimLib;
    IosRect m_clipRect, *m_clipRectPtr;
    int m_viewportWidth, m_viewportHeight;
    GLfloat m_offsetX, m_offsetY;
public:
	GLfloat matrix[16];
};

#endif // _OPENGL_DRAW_CONTEXT_H_

