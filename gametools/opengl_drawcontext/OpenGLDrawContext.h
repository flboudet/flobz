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

#ifdef IOS
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "drawcontext.h"

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
    virtual IosFont    * createFont(const char *path, int size, IosFontFx fx = Font_STD);
private:
    OpenGLDrawContext *m_owner;
    OpenGLBackendUtil *m_backendUtil;
};

class OpenGLDrawContext : public DrawContext
{
public:
    void init(OpenGLBackendUtil *backendUtil, int width, int height);
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
    virtual void putString(IosFont *font, int x, int y, const char *text);
	virtual void putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text);
    void putStringCenteredXY(IosFont *font, int x, int y, const char *text);
	// Query for drawcontext abilities
    virtual bool hasScaleAbility() const { return true; }
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list);
public:
    // Performance measurement
    void startFrame();
    void endFrame();
private:
    OpenGLBackendUtil *m_backendUtil;
private:
    std::auto_ptr<OpenGLImageLibrary> iimLib;
public:
	GLfloat matrix[16];
};

#endif // _OPENGL_DRAW_CONTEXT_H_

