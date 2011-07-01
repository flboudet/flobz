//
//  OpenGLDrawContext.cpp
//  flobopop
//
//  Created by Florent Boudet on 13/04/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <math.h>
#include <string>
#include "OpenGLDrawContext.h"
#include "GTLog.h"
#include "ios_mutex.h"
#include "ios_filepath.h"
#include "GLFreeType.h"

using namespace  flobopop;


#define GL_GET_ERROR() \
while(1) {\
GLenum err = glGetError(); \
if (err != GL_NO_ERROR) \
GTLogTrace("OpenGL Error: %s:%d, %d\n", __FILE__, __LINE__, err); \
break; \
}

static float gray_alpha = 1.0f; // XXX: hack pour le menu pause...

class Benchmarking {
	int num;
	unsigned int frame;
	double sub;
	double tot[64]; // <- max methods = 64;
	double frames[64];
	double start;
	int num_ignore;
	double max_fps;

public:
	Benchmarking() : num(0),start(-1.0) {}
	void set_num_methods(int num) {
		if (this->num > 0) return;
		this->num = num;
		for (int i=0;i<num;++i) {
			tot[i] = 0.00001;
			frames[i] = 0;
		}
		sub = 0.0;
		frame = 0;
		num_ignore = 100;
		max_fps = 10.0;
	}
	inline unsigned int current_method() {
		return frame / 10;
	}
	inline void start_frame() {
		if (this->num == 0) return;
		if (num_ignore-->0) return;
		glFlush();
		start = ios_fc::getTimeMs();
	}
	inline void end_frame() {
		if (this->num == 0) return;
		if (num_ignore-->0) return;
		if (this->start < 0) return; // first frame...
		glFlush();
		double end = ios_fc::getTimeMs();
		sub += (end - start);
		int i = frame%10;
		if (i == 9) {
			int j = current_method();
			//glFlush();
			//double end_end = ios_fc::getTimeMs();
			//sub += (end_end - end);
			tot[j]    = tot[j]*0.97 + sub; // Give more weight to latest frames.
			frames[j] = frames[j]*0.97 + 10.0;
			sub = 0;
		}
		++frame;
		if (frame == 10*num) frame = 0;
	}
	inline void draw() {
		if (this->num == 0) return;
		for (int i=0; i<num; ++i) {
			double fps = 1000.0*(frames[i]/tot[i]);
			if (fps > max_fps) max_fps = fps*1.1;
			GLfloat x = 0, y = 30 + i * 30;
			GLfloat w = 320.0*fps/max_fps, h = 20;
			GLfloat vertices[8];
			GLushort faces[4] = {0,1,2,3};
			vertices[0] = x;   vertices[1] = y;
			vertices[2] = x+w; vertices[3] = y;
			vertices[4] = x;   vertices[5] = y+h;
			vertices[6] = x+w; vertices[7] = y+h;
			switch (i) {
				case 0: glColor4f(1, 0, 0, .5); break;
				case 1: glColor4f(0, 0, 1, .5); break;
				case 2: glColor4f(0, 1, 0, .5); break;
				case 3: glColor4f(1, 0, 1, .5); break;
			}
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
			glEnableClientState(GL_VERTEX_ARRAY);
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, &faces[0]);
			glDisable(GL_BLEND);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
		glColor4f(1,1,1,1);
	}
};
static Benchmarking BENCH;


//-- RGB<->HSV conversion

static int max3(int i1, int i2, int i3)
{
    if ((i1>=i2)&&(i1>=i3)) return i1;
    if ((i2>=i3)&&(i2>=i1)) return i2;
    return i3;
}

static int min3(int i1, int i2, int i3)
{
    if ((i1<=i2)&&(i1<=i3)) return i1;
    if ((i2<=i3)&&(i2<=i1)) return i2;
    return i3;
}

//-- RGB, each 0 to 255
//-- H = 0.0 to 360.0 (corresponding to 0..360.0 degrees around hexcone)
//-- S = 0.0 (shade of gray) to 1.0 (pure color)
//-- V = 0.0 (black) to 1.0 (white)

//-- Based on C Code in "Computer Graphics -- Principles and Practice,"
//-- Foley et al, 1996, pp. 592,593.
static inline HSVA image_rgba2hsva(RGBA c)
{
    HSVA res;
    float minVal = (float)min3(c.red, c.blue, c.green);
    res.value    = (float)max3(c.red, c.green, c.blue);
    float delta  = res.value - minVal;

    // -- Calculate saturation: saturation is 0 if r, g and b are all 0
    if (res.value == 0.0f)
        res.saturation = 0.0f;
    else
        res.saturation = delta / res.value;

    if (res.saturation == 0.0f)
        res.hue = 0.0f; // Achromatic: When s = 0, h is undefined but who cares
    else             // Chromatic
        if (res.value == c.red) // between yellow and magenta [degrees]
            res.hue = 60.0f * (c.green - c.blue) / delta;
        else if (res.value == c.green) // between cyan and yellow
            res.hue = 120.0 + 60.0f * (c.blue - c.red) / delta;
        else // between magenta and cyan
            res.hue = 240.0f + 60.0f * (c.red - c.green) / delta;

    if (res.hue < 0.0f) res.hue += 360.0f;
    // return a list of values as an rgb object would not be sensible
    res.value /= 255.0f;
    res.alpha = c.alpha;
    return res;
}

static inline RGBA image_hsva2rgba(HSVA c)
{
    float red=0.0f,green=0.0f,blue=0.0f,hueTemp=0.0f;
    if (c.saturation == 0.0f) // color is on black-and-white center line
    {
        red   = c.value; // achromatic: shades of gray
        green = c.value; // supposedly invalid for h=0 but who cares
        blue  = c.value;
    }
    else { // chromatic color
        if (c.hue == 360.0f)      // 360 degrees same as 0 degrees
            hueTemp = 0.0f;
        else {
            hueTemp = c.hue / 60.0f;  // h is now in [0,6)
            // hueTemp = hueTemp / 60.0f;
        }
        float i = floor(hueTemp); // largest integer <= h
        float f = hueTemp - i;    // fractional part of h

        float p = c.value * (1.0f - c.saturation);
        float q = c.value * (1.0f - (c.saturation * f));
        float t = c.value * (1.0f - (c.saturation * (1.0-f)));

        switch((int)i) {
            case 0:
                red   = c.value;
                green = t;
                blue  = p;
                break;
            case 1:
                red   = q;
                green = c.value;
                blue  = p;
                break;
            case 2:
                red   = p;
                green = c.value;
                blue  = t;
                break;
            case 3:
                red   = p;
                green = q;
                blue  = c.value;
                break;
            case 4:
                red   = t;
                green = p;
                blue  = c.value;
                break;
            case 5:
                red   = c.value;
                green = p;
                blue  = q;
                break;
        }
    }
    RGBA ret;
    ret.red   = (GLubyte)(red   * 255.0f);
    ret.green = (GLubyte)(green * 255.0f);
    ret.blue  = (GLubyte)(blue  * 255.0f);
    ret.alpha = c.alpha;
    return ret;
}

/**
 * Shift the saturation of a surface
 */
static GLubyte *image_rgba_shift_hsv(GLubyte *src, int src_w, int src_h, float h, float s, float v)
{
    GLubyte *ret = (GLubyte*)malloc(src_h*src_w*4);
    for (int y=src_h; y--;) {
        for (int x=src_w; x--;) {
            GLubyte *c = src + 4*(src_w*y+x);
            GLubyte *r = ret + 4*(src_w*y+x);
            RGBA rgba;
            rgba.red = c[0];
            rgba.green = c[1];
            rgba.blue = c[2];
            rgba.alpha = c[3];
            HSVA hsva = image_rgba2hsva(rgba);
            hsva.hue += h;
            if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
            if (hsva.hue < 0.0f) hsva.hue += 360.0f;
            hsva.saturation += s;
            if (hsva.saturation > 1.) hsva.saturation = 1.f;
            if (hsva.saturation < 0.0f) hsva.saturation = .0f;
            hsva.value += v;
            if (hsva.value > 1.) hsva.value = 1.f;
            if (hsva.value < 0.0f) hsva.value = .0f;
            rgba = image_hsva2rgba(hsva);
            r[0] = rgba.red;
            r[1] = rgba.green;
            r[2] = rgba.blue;
            r[3] = rgba.alpha;
        }
    }
    return ret;
}

/**
 * Shift the hue of a surface
 */
static GLubyte *image_rgba_shift_hue(GLubyte *src, int src_w, int src_h, float hue_offset)
{
    GLubyte *ret = (GLubyte*)malloc(src_h*src_w*4);
    for (int y=src_h; y--;) {
        for (int x=src_w; x--;) {
            GLubyte *c = src + 4*(src_w*y+x);
            GLubyte *r = ret + 4*(src_w*y+x);
            RGBA rgba;
            rgba.red = c[0];
            rgba.green = c[1];
            rgba.blue = c[2];
            rgba.alpha = c[3];
            HSVA hsva = image_rgba2hsva(rgba);
            hsva.hue += hue_offset;
            if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
            if (hsva.hue < 0.0f) hsva.hue += 360.0f;
            rgba = image_hsva2rgba(hsva);
            r[0] = rgba.red;
            r[1] = rgba.green;
            r[2] = rgba.blue;
            r[3] = rgba.alpha;
        }
    }
    return ret;
}

/**
 * Shift the hue of a surface
 */
static GLubyte *image_rgba_shift_hue_masked(GLubyte *src, int src_w, int src_h,
                                            GLubyte *mask, int mask_w, int mask_h, float hue_offset)
{
    GLubyte *ret = (GLubyte*)malloc(src_h*src_w*4);
    for (int y=src_h; y--;) {
        for (int x=src_w; x--;) {
            GLubyte *c = src + 4*(src_w*y+x);
            GLubyte *r = ret + 4*(src_w*y+x);
            GLubyte *m = mask + 4*(mask_w*y+x);
            if ((x > mask_w) || (y > mask_h) || (m[3] != 0)) {
                r[0] = c[0];
                r[1] = c[1];
                r[2] = c[2];
                r[3] = c[3];
            }
            else {
                RGBA rgba;
                rgba.red = c[0];
                rgba.green = c[1];
                rgba.blue = c[2];
                rgba.alpha = c[3];
                HSVA hsva = image_rgba2hsva(rgba);
                hsva.hue += hue_offset;
                if (hsva.hue > 360.0f) hsva.hue -= 360.0f;
                if (hsva.hue < 0.0f) hsva.hue += 360.0f;
                rgba = image_hsva2rgba(hsva);
                r[0] = rgba.red;
                r[1] = rgba.green;
                r[2] = rgba.blue;
                r[3] = rgba.alpha;
            }
        }
    }
    return ret;
}




static int toBpp(ImageType type) {
	switch (type) {
		case IMAGE_RGB:
			return 3;
		case IMAGE_RGBA:
			return 4;
	}
	return 4;
}
/*
 static float stubColor[][4] = {
 {1., 0.5, 0.5, 1.},
 {0.5, 0.5, 1., 1.},
 {0.5, 1., 0.5, 1.},
 {1., 1., 0.5, 1.},
 {1., 0.5, 1., 1.},
 {1., 1., 0.5, 1.},
 {1., 0, 0, 1.},
 {0, 0, 1., 1.},
 {0, 1., 0, 1.}
 };
 */
static __inline__ int power_of_2(int input)
{
    int value = 1;
    while (value < input) {
        value <<= 1;
    }
    return value;
}

/*
 GLFont(const char *fname, unsigned int h, float letter_spacing) { init(fname, h, letter_spacing); }
 GLFont() {}
 ~GLFont() {}
 float letter_spacing;
 //The init function will create a font of
 //of the height h from the file fname.
 void init(const char * fname, unsigned int h, float letter_spacing);
 //Free all the resources assosiated with the font.
 void clean();
 //The flagship function of the library - this thing will print
 //out text at window coordinates x,y, using the font ft_font.
 //The current modelview matrix will also be applied to the text.
 void print(float x, float y, const wchar_t *fmt, ...) ;
 void printCentered(float x, float y, const wchar_t *fmt, ...);
 */
static std::wstring toWString(const std::string &s0)
{
    wchar_t *s1 = new wchar_t[s0.size() + 1];
    for (unsigned int j=0; j<s0.size(); ++j) s1[j] = s0[j];
    s1[s0.size()] = 0;
    std::wstring s2 = s1;
    delete[] s1;
    return s2;
}
static std::string toString(const std::wstring &s0)
{
    char *s1 = new char[s0.size() + 1];
    for (unsigned int j=0; j<s0.size(); ++j) s1[j] = s0[j];
    s1[s0.size()] = 0;
    std::string s2 = s1;
    delete[] s1;
    return s2;
}



static void iglBindFramebufferOES(GLuint i, GLfloat *matrix) {
    static int gCurrentFBO = 0;
    static GLfloat *gCurrentMatrix = NULL;
	if (i != gCurrentFBO) {
#ifdef OPENGLES
        glBindFramebufferOES(GL_FRAMEBUFFER_OES, i);
#else
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, i);
#endif
    }
	if ((matrix) && (matrix != gCurrentMatrix)) {
        glLoadMatrixf(matrix);
        gCurrentMatrix = matrix;
    }
	gCurrentFBO = i;
}


class OpenGLIosFont : public IosFont
{
private:
    OpenGLBackendUtil *m_backendUtil;
	GLFont *mCustomFont;
	int mSize;
	IosFontFx mFX;
    unsigned short mWorkingBuffer[256];

public:
	OpenGLIosFont(OpenGLBackendUtil *backendUtil, const char *path, int size, IosFontFx fx = Font_STD)
    : m_backendUtil(backendUtil)
    {
		mCustomFont = new GLFont(path, size, 0.0f);
		mSize = size;
		mFX = fx;
	}

	virtual ~OpenGLIosFont() {
		mCustomFont->clean();
		delete mCustomFont;
	}
    virtual int getTextWidth(const char *text);
    virtual int getHeight();
    virtual int getLineSkip();

	void print(float x, float y, const char *txt) {
		/*if (gCurrentMatrix) {
         glPushMatrix();
         glLoadMatrixf(gCurrentMatrix);
         }
         switch (mFX) {
         case Font_STD: glColor4ub(0xEF,0xA2,0x43,0xFF); break;
         case Font_DARK: glColor4ub(0x43,0x89,0xEF,0xFF); break;
         case Font_GREY: glColor4ub(0xCC,0xCC,0xCC,0xFF); break;
         }
         mCustomFont->printUnicode(x,y, utf8ToUnicode(txt));
         glColor4ub(0xFF,0xFF,0xFF,0xFF);
         if (gCurrentMatrix)
         glPopMatrix();*/
		printWithShadow(x,y,1.0f,2.0f,txt);
	}
	void printWithShadow(float x, float y, float sx, float sy, const char *txt) {
		// TODO: ensure correct matrix

		glEnable(GL_BLEND); GL_GET_ERROR();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GL_GET_ERROR();
		unsigned short *utxt = m_backendUtil->utf8ToUnicode(txt, mWorkingBuffer, sizeof(mWorkingBuffer));
		glColor4ub(0,0,0, 0xc0);
        //mCustomFont->printUnicode(x+sx+0.5f,y+sy, utxt);
        mCustomFont->printUnicode(x+sx/*-0.5f*/,y+sy, utxt);
        /*mCustomFont->printUnicode(x+sx-0.5f,y+sy-0.5f, utxt);
         mCustomFont->printUnicode(x+sx+0.5f,y+sy-0.5f, utxt);*/

		switch (mFX) {
			case Font_STD: glColor4ub(0xEF,0xA2,0x43,0xFF); break;
			case Font_DARK: glColor4ub(0x43,0x89,0xEF,0xFF); break;
			case Font_GREY: glColor4ub(0xCC,0xCC,0xCC,0xFF); break;
			default:
				glColor4ub(0xe0,0xe0,0xe0, 0xff);

		}
        mCustomFont->printUnicode(x,y, utxt);

		glColor4ub(0xFF,0xFF,0xFF,0xFF);
	}
	void printCentered(float x, float y, const char *txt) {
        // TODO: ensure correct matrix
        mCustomFont->printCenteredUnicode(x,y, m_backendUtil->utf8ToUnicode(txt, mWorkingBuffer, sizeof(mWorkingBuffer)));
	}
};

int OpenGLIosFont::getTextWidth(const char *text)
{
    return mCustomFont->getWidthUnicode(m_backendUtil->utf8ToUnicode(text, mWorkingBuffer, sizeof(mWorkingBuffer)));
}

int OpenGLIosFont::getHeight()
{
    return mSize;
}

int OpenGLIosFont::getLineSkip()
{
    return 10;
}

// OpenGL Texture
class OpenGLTexture
{
public:
    OpenGLTexture(OpenGLDrawContext *owner, GLint format, int w, int h);
    OpenGLTexture(OpenGLDrawContext *owner, OpenGLRawImage *rawImage, bool preserveRawData=false);
	~OpenGLTexture();
    RGBA readRGBA(int x, int y);
    void setOpaque(bool opaque) { m_opaque = opaque; }
    void clearPreserveRawData();
    GLubyte *getRawImageData() const;
private:
	friend class IosGLSurfaceRef;
    OpenGLDrawContext *m_owner;
    ios_fc::SharedPtr<OpenGLRawImage> m_rawImage;
    bool m_preserveRawData;
    GLuint m_fbo;
	int m_nRef;
	GLenum m_format;
    GLuint m_texture;
    bool m_textureOK;
	int m_w,m_h;
	int m_p2w,m_p2h;

	GLfloat m_matrix[16];
    bool m_opaque;

	inline void incRef() {
		++m_nRef;
	}
	inline void decRef() {
		--m_nRef;
	}
	inline bool noRef() const { return m_nRef <= 0; }

	inline GLuint getTexture() {
        // If the texture has already been created, return the texture
		if (m_textureOK)
            return m_texture;
        // Otherwise, generate the texture and return its identifiant
        glEnable(GL_TEXTURE_2D); GL_GET_ERROR();
        glGenTextures(1, &m_texture); GL_GET_ERROR();
        glBindTexture(GL_TEXTURE_2D, m_texture); GL_GET_ERROR();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   GL_GET_ERROR();
        if (m_rawImage.get() != NULL) {
            if (! m_rawImage->compressed) {
                glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_p2w, m_p2h, 0, m_format, GL_UNSIGNED_BYTE, m_rawImage->data);
                GL_GET_ERROR();
            }
            else {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, m_format, m_p2w, m_p2h, 0, m_rawImage->dataSize, m_rawImage->data);
                GL_GET_ERROR();
            }
            if (! m_preserveRawData)
                m_rawImage = NULL;
        }
        else {
            GLubyte *emptyData;
            switch (m_format) {
                case GL_RGB:
                    emptyData = (GLubyte *)calloc(3 * m_p2w * m_p2h, 1);
                    break;
                default:
                    emptyData = (GLubyte *)calloc(4 * m_p2w * m_p2h, 1);
                    break;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_p2w, m_p2h, 0, m_format, GL_UNSIGNED_BYTE, emptyData);
            free(emptyData);
        }
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GL_GET_ERROR();
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GL_GET_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GL_GET_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GL_GET_ERROR();
        m_textureOK = true;
        return m_texture;
	}

	inline GLuint getFBO() {
        // If the texture framebuffer exists, return it
		if (m_fbo)
            return m_fbo;
        // Else setup Texture Framebuffer
        bindTexture(); // Makes sure texture is ready
#ifdef OPENGLES
        glGenFramebuffersOES(1, &m_fbo); GL_GET_ERROR();
        iglBindFramebufferOES(m_fbo,NULL); GL_GET_ERROR();
        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, m_texture, 0); GL_GET_ERROR();
        if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
            GTLogTrace("Failed to make complete framebuffer object %x",  glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        }
#else
        glGenFramebuffersEXT(1, &m_fbo); GL_GET_ERROR();
        iglBindFramebufferOES(m_fbo,NULL); GL_GET_ERROR();
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture, 0); GL_GET_ERROR();
        if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
            GTLogTrace("Failed to make complete framebuffer object %x",  glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
        }
#endif
        // Setup OpenGL projection
        glPushMatrix();
        //glViewport(0, 0, this->m_w, this->m_h);
        /*glLoadIdentity();
         glOrthof(0.0, (GLfloat) this->m_w, (GLfloat) this->m_h, 0.0, 0.0, 1.0);*/
        glTranslatef(0.0f, 480.0f, 0.0f);
        glScalef(m_owner->getPixelRatioX(),-m_owner->getPixelRatioY(),1);
#ifdef DISABLED
        /*glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();*/
        //glViewport(0, 0, m_w, m_h);
#ifdef OPENGLES
        glOrthof(0.0, (GLfloat) m_w, (GLfloat) m_h, 0.0, 0.0, 1.0); GL_GET_ERROR();
#else
        glOrtho(0.0, (GLfloat) m_w, (GLfloat) m_h, 0.0, 0.0, 1.0); GL_GET_ERROR();
#endif
#endif
        glGetFloatv(GL_MODELVIEW_MATRIX,&m_matrix[0]);
        glPopMatrix(); GL_GET_ERROR();
        return m_fbo;
	}

	inline void bindTexture() {
		glBindTexture(GL_TEXTURE_2D, getTexture()); GL_GET_ERROR();
		glEnable(GL_TEXTURE_2D); GL_GET_ERROR();
	}
	inline void unbindTexture() {
		// glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D); GL_GET_ERROR();
	}
};

OpenGLTexture::OpenGLTexture(OpenGLDrawContext *owner, GLint format, int w, int h)
: m_owner(owner), m_preserveRawData(false), m_format(format), m_opaque(m_format==GL_RGB?true:false)
{
    m_h = h;
    m_w = w;
    m_p2w = power_of_2(w);
    m_p2h = power_of_2(h);
    m_texture = 0;
    m_textureOK = false;
    m_fbo = 0;
    m_nRef = 0;
}

OpenGLTexture::OpenGLTexture(OpenGLDrawContext *owner, OpenGLRawImage *rawImage, bool preserveRawData)
: m_owner(owner), m_rawImage(rawImage), m_preserveRawData(preserveRawData), m_format(rawImage->format), m_opaque(m_format==GL_RGB?true:false)
{
    m_h = rawImage->height;
    m_w = rawImage->width;
    m_p2w = rawImage->p2width;
    m_p2h = rawImage->p2height;
    m_texture = 0;
    m_textureOK = false;
    m_fbo = 0;
    m_nRef = 0;
}

OpenGLTexture::~OpenGLTexture() {
    if (m_fbo) {
#ifdef OPENGLES
        glDeleteFramebuffersOES(1, &m_fbo); GL_GET_ERROR();
#else
        glDeleteFramebuffersEXT(1, &m_fbo); GL_GET_ERROR();
#endif
    }
    if (m_texture) {
        glDeleteTextures(1, &m_texture); GL_GET_ERROR();
    }
}

RGBA OpenGLTexture::readRGBA(int x, int y)
{
    RGBA c;
    if (m_format == GL_RGB) {
        GLubyte *base = (GLubyte *)(m_rawImage->data) + (x + y * m_p2w) * 3;
        c.red = base[0];
        c.green = base[1];
        c.blue = base[2];
        c.alpha = 255;
    }
    else if (m_format == GL_RGBA) {
        GLubyte *base = (GLubyte *)(m_rawImage->data) + (x + y * m_p2w) * 3;
        c.red = base[1];
        c.green = base[2];
        c.blue = base[3];
        c.alpha = base[0]; // XXX, WHY THE HELL IS THE BUFFER IN ARGB? but it works... on my simulator...
    }
    return c;
}

void OpenGLTexture::clearPreserveRawData()
{
    m_preserveRawData = false;
    if (m_textureOK)
        m_rawImage = NULL;
}

GLubyte *OpenGLTexture::getRawImageData() const
{
    if (m_rawImage.get() == NULL)
        return NULL;
    return (GLubyte *)(m_rawImage->data);
}






static void fixRects(IosRect *srcRect, IosRect *dstRect, const IosSurface *surf, const DrawTarget *targ, IosRect **pSrcRect, IosRect **pDstRect)
{
	static IosRect cSrcRect;
	static IosRect cDstRect;
	*pSrcRect = (srcRect != NULL) ? srcRect :
    (cSrcRect.x = 0, cSrcRect.y = 0, cSrcRect.h = surf->h, cSrcRect.w = surf->w, &cSrcRect);
	*pDstRect = (dstRect != NULL) ? dstRect :
    (cDstRect.x = 0, cDstRect.y = 0, cDstRect.h = targ->h, cDstRect.w = targ->w, &cDstRect);
}

class IosGLSurfaceRef : public IosSurface
{
public:
	OpenGLTexture *m_ref;
	ImageBlendMode m_mode;
	bool gray, hflip;
public:
	IosGLSurfaceRef(OpenGLDrawContext *owner, OpenGLBackendUtil *backendUtil, OpenGLTexture *s);
	IosGLSurfaceRef(IosGLSurfaceRef *s);
	virtual ~IosGLSurfaceRef();
	virtual bool isOpaque() const;
	virtual bool haveAbility(ImageSpecialAbility a) const;
	virtual void dropAbility(ImageSpecialAbility a);
    virtual RGBA readRGBA(int x, int y);
	void copyDrawMode(IosGLSurfaceRef *ref);
    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL);
    virtual IosSurface *shiftHSV(float h, float s, float v);
    virtual IosSurface *setValue(float value);
    virtual IosSurface *setAlpha(float alpha);
    virtual IosSurface * resizeAlpha(int width, int height);
    virtual IosSurface * mirrorH();
    virtual void convertToGray();

	// Draw Target Implementation
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
	virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
	virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
	virtual void putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text);

	//
	// OpenGL code
	//
	inline void bindSurface();
    inline void unbindSurface();
	void applyBlendMode(ImageBlendMode mode);
	enum ToGlDrawMode { NORMAL = 0, X_FLIP = 1};
	void drawToGL(IosRect *pSrcRect, IosRect *pDstRect,  ToGlDrawMode mode = NORMAL, float sx=0.0f,float sy=0.0f);

private:
    OpenGLDrawContext *m_owner;
    OpenGLBackendUtil *m_backendUtil;
protected:
    float m_alpha;
};

IosGLSurfaceRef::IosGLSurfaceRef(OpenGLDrawContext *owner, OpenGLBackendUtil *backendUtil, OpenGLTexture *s)
    : m_ref(s), m_owner(owner), m_backendUtil(backendUtil), m_alpha(1.0)
{
    this->h = m_ref->m_h;
    this->w = m_ref->m_w;
    m_ref->incRef();
    m_mode = IMAGE_BLEND;
    gray = false;
    hflip = false;
}

IosGLSurfaceRef::IosGLSurfaceRef(IosGLSurfaceRef *s)
    : m_ref(s->m_ref), m_backendUtil(s->m_backendUtil), m_alpha(1.0)
{
    this->h = m_ref->m_h;
    this->w = m_ref->m_w;
    m_ref->incRef();
    copyDrawMode(s);
}

IosGLSurfaceRef::~IosGLSurfaceRef() {
    m_ref->decRef();
    if (m_ref->noRef()) delete m_ref;
}

bool IosGLSurfaceRef::isOpaque() const
{ return m_ref->m_opaque; }

bool IosGLSurfaceRef::haveAbility(ImageSpecialAbility a) const
{
    if (a == IMAGE_READ)
        return m_ref->m_rawImage.get() != NULL;
    return false;
}

void IosGLSurfaceRef::dropAbility(ImageSpecialAbility a)
{
    if (a == IMAGE_READ)
        m_ref->clearPreserveRawData();
}

RGBA IosGLSurfaceRef::readRGBA(int x, int y)
{
    return m_ref->readRGBA(x, y);
}

void IosGLSurfaceRef::copyDrawMode(IosGLSurfaceRef *ref)
{
    this->hflip = ref->hflip;
    this->m_mode = ref->m_mode;
    this->gray = ref->gray;
}

IosSurface *IosGLSurfaceRef::shiftHue(float hue_offset, IosSurface *mask)
{
    GLubyte *rawImage = m_ref->getRawImageData();
    if ((rawImage != NULL) && (m_ref->m_format == GL_RGBA) && (hue_offset != 0)) {
        GLubyte *newData;
        if (mask == NULL)
            newData = image_rgba_shift_hue(rawImage, m_ref->m_p2w, m_ref->m_p2h, hue_offset);
        else {
            IosGLSurfaceRef *maskSurf = static_cast<IosGLSurfaceRef*>(mask);
            GLubyte *rawMask = maskSurf->m_ref->getRawImageData();
            newData = image_rgba_shift_hue_masked(rawImage, m_ref->m_p2w, m_ref->m_p2h,
                                                  rawMask, maskSurf->m_ref->m_p2w, maskSurf->m_ref->m_p2h,hue_offset);
        }
        OpenGLRawImage *newRawImage = new MallocedOpenGLRawImage(GL_RGBA, m_ref->m_w, m_ref->m_h, m_ref->m_p2w, m_ref->m_p2h, newData);
        OpenGLTexture* retSurf = new OpenGLTexture(m_owner, newRawImage);
        IosGLSurfaceRef *ret = new IosGLSurfaceRef(m_owner, m_backendUtil, retSurf);
        ret->copyDrawMode(this);
        return ret;
    }
    return new IosGLSurfaceRef(this);
}

IosSurface *IosGLSurfaceRef::shiftHSV(float h, float s, float v)
{
    return new IosGLSurfaceRef(this); // TODO
}

IosSurface *IosGLSurfaceRef::setValue(float value)
{
    GLubyte *rawImage = m_ref->getRawImageData();
    if ((rawImage != NULL) && (m_ref->m_format == GL_RGBA) && (value != 0)) {
        GLubyte *newData = image_rgba_shift_hsv(rawImage, m_ref->m_p2w, m_ref->m_p2h, 0, 0, value);
        OpenGLRawImage *newRawImage = new MallocedOpenGLRawImage(GL_RGBA, m_ref->m_w, m_ref->m_h, m_ref->m_p2w, m_ref->m_p2h, newData);
        OpenGLTexture* retSurf = new OpenGLTexture(m_owner, newRawImage);
        IosGLSurfaceRef *ret = new IosGLSurfaceRef(m_owner, m_backendUtil, retSurf);
        ret->copyDrawMode(this);
        return ret;
    }
    return new IosGLSurfaceRef(this);
}

IosSurface *IosGLSurfaceRef::setAlpha(float alpha)
{
    IosGLSurfaceRef *result = new IosGLSurfaceRef(this);
    result->m_alpha = alpha;
    return result;
}

IosSurface * IosGLSurfaceRef::resizeAlpha(int width, int height)
{
    return new IosGLSurfaceRef(this); // TODO... easy fix with drawToGl supporting scaling...
}

IosSurface * IosGLSurfaceRef::mirrorH()
{
    IosGLSurfaceRef *ret = new IosGLSurfaceRef(this);
    ret->hflip = !ret->hflip;
    return ret;
}

void IosGLSurfaceRef::convertToGray()
{
    gray = true;
    gray_alpha = 1.0f;
}

// Draw Target Implementation

void IosGLSurfaceRef::setClipRect(IosRect *rect)
{
}

void IosGLSurfaceRef::setBlendMode(ImageBlendMode mode)
{
    m_mode = mode;
}

void IosGLSurfaceRef::draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    m_backendUtil->ensureContextIsActive();
    bindSurface();
    IosRect *pSrcRect, *pDstRect;
    fixRects(srcRect, dstRect, surf, this, &pSrcRect, &pDstRect);
    IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef*>(surf);
    ipSurf->applyBlendMode(m_mode);
    ipSurf->drawToGL(pSrcRect, pDstRect);
    unbindSurface();
}

void IosGLSurfaceRef::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    m_backendUtil->ensureContextIsActive();
    bindSurface();
    IosRect *pSrcRect, *pDstRect;
    fixRects(srcRect, dstRect, surf, this, &pSrcRect, &pDstRect);
    IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef*>(surf);
    ipSurf->applyBlendMode(m_mode);
    ipSurf->drawToGL(pSrcRect, pDstRect, X_FLIP);
    unbindSurface();
}

void IosGLSurfaceRef::drawRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    /*m_ref->bindFBO();
     IosRect *pSrcRect, *pDstRect;
     fixRects(NULL, NULL, surf, this, &pSrcRect, &pDstRect);
     IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef*>(surf);
     ipSurf->draw(surf, pSrcRect, pDstRect);*/
    // TODO: Rotate!
}

void IosGLSurfaceRef::fillRect(const IosRect *rect, const RGBA &color)
{
    m_backendUtil->ensureContextIsActive();
    bindSurface();
    // Let's get lazy!
    glClearColor(color.red/255.0f, color.green/255.0f, color.blue/255.0f, color.alpha/255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // TODO (That's totally out of spec, but FloboPop doesn't need more than this)
    unbindSurface();
}

void IosGLSurfaceRef::putString(IosFont *font, int x, int y, const char *text)
{
    m_backendUtil->ensureContextIsActive();
    bindSurface();
    OpenGLIosFont *ifont = static_cast<OpenGLIosFont*> (font);
    ifont->print(x,y,text);
    unbindSurface();
}

void IosGLSurfaceRef::putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text)
{
    m_backendUtil->ensureContextIsActive();
    bindSurface();
    OpenGLIosFont *ifont = static_cast<OpenGLIosFont*> (font);
    ifont->printWithShadow(x,y,shadow_x,shadow_y,text);
    unbindSurface();
}

//
// OpenGL code
//
void IosGLSurfaceRef::bindSurface()
{
    iglBindFramebufferOES(m_ref->getFBO(), m_ref->m_matrix);
}

void IosGLSurfaceRef::unbindSurface()
{
    iglBindFramebufferOES(m_backendUtil->m_defaultFBO, m_owner->matrix);
}

void IosGLSurfaceRef::applyBlendMode(ImageBlendMode mode)
{
    m_ref->bindTexture();
    if (m_ref->m_opaque) {
        glDisable(GL_BLEND);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); GL_GET_ERROR();
        return;
    }
    switch (mode) {
        case IMAGE_BLEND:
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); GL_GET_ERROR();
            glEnable(GL_BLEND); GL_GET_ERROR();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GL_GET_ERROR();
            return;
        case IMAGE_COPY:
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); GL_GET_ERROR();
            glDisable(GL_BLEND);
            return;
    }
}

void IosGLSurfaceRef::drawToGL(IosRect *pSrcRect, IosRect *pDstRect, ToGlDrawMode mode, float sx,float sy)
{
    //if (mode == NORMAL)
    //    mode = SCALED;
#ifdef BENCH_BLIT
    BENCH.set_num_methods(2);
    switch(BENCH.current_method()) {
        case 0:
            mode = NORMAL;
            break;
        case 1:
            mode = SCALED;
            if (sx == 0.0f) { sx = sy = 1.0f; }
            break;
    }
#endif
    sx = sy = 1.0f; // Because X_FLIP supports scaling images..

    if (gray) {
        // MEGA EFFECT POUR LA PAUSE (Bidouille qui ne marche que pour une image plein ecran...
        // Code tres pas beau, mais le resultat est beau ;-)
        static bool odd = true;
        double t;
        static double st;
        if (odd) {
            t = ios_fc::getTimeMs() / 1000.0;
            st = t;
            if (gray_alpha > 0.08f)
                gray_alpha -= 0.02f;
            else
                gray_alpha = 0.08f;
        }
        else t = st;
        odd = !odd;

        float zoom_in = 0.2f + 0.35f * (1.0f+sin(1.1*t)) * 0.5f + 0.25f*sin(t*.11)*cos(t*.14)*sin(1.f+t*.13);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f,1.0f,1.0f,gray_alpha);
        sx *= 1.0f + zoom_in;
        sy *= 1.0f + zoom_in;
        if (pDstRect) {
            float added = 320.0f * zoom_in;
            pDstRect->x -= added / (4.0f + 2.0f*sin(t*.3));
            pDstRect->y -= added / (4.0f + 3.0f*cos(t*.3));
        }
    }

    if ((mode != X_FLIP) && hflip)
        mode = X_FLIP;

    if ((mode == X_FLIP) || hflip) {
        GLfloat texcoord[8];
        GLfloat vertices[8];
        GLushort faces[4] = {0,1,2,3};
        texcoord[2] = (GLfloat)(pSrcRect->x) / m_ref->m_p2w;             texcoord[1] = (GLfloat)(pSrcRect->y) / m_ref->m_p2h;
        texcoord[0] = (GLfloat)(pSrcRect->x+pSrcRect->w) / m_ref->m_p2w; texcoord[3] = (GLfloat)(pSrcRect->y) / m_ref->m_p2h;
        texcoord[6] = (GLfloat)(pSrcRect->x) / m_ref->m_p2w;             texcoord[5] = (GLfloat)(pSrcRect->y+pSrcRect->h) / m_ref->m_p2h;
        texcoord[4] = (GLfloat)(pSrcRect->x+pSrcRect->w) / m_ref->m_p2w; texcoord[7] = (GLfloat)(pSrcRect->y+pSrcRect->h) / m_ref->m_p2h;
        vertices[0] = pDstRect->x;                vertices[1] = pDstRect->y;
        vertices[2] = pDstRect->x+pDstRect->w*sx; vertices[3] = pDstRect->y;
        vertices[4] = pDstRect->x;                vertices[5] = pDstRect->y+pDstRect->h*sy;
        vertices[6] = pDstRect->x+pDstRect->w*sx; vertices[7] = pDstRect->y+pDstRect->h/sy;
        glTexCoordPointer(2, GL_FLOAT, 0, &texcoord[0]);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, &faces[0]);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLfloat texcoord[8];
        GLfloat vertices[8];
        GLushort faces[4] = {0,1,2,3};
        texcoord[0] = (GLfloat)(pSrcRect->x) / m_ref->m_p2w;
        texcoord[1] = (GLfloat)(pSrcRect->y) / m_ref->m_p2h;
        texcoord[2] = (GLfloat)(pSrcRect->x+pSrcRect->w) / m_ref->m_p2w;
        texcoord[3] = (GLfloat)(pSrcRect->y) / m_ref->m_p2h;
        texcoord[4] = (GLfloat)(pSrcRect->x) / m_ref->m_p2w;
        texcoord[5] = (GLfloat)(pSrcRect->y+pSrcRect->h) / m_ref->m_p2h;
        texcoord[6] = (GLfloat)(pSrcRect->x+pSrcRect->w) / m_ref->m_p2w;
        texcoord[7] = (GLfloat)(pSrcRect->y+pSrcRect->h) / m_ref->m_p2h;
        vertices[0] = pDstRect->x;
        vertices[1] = pDstRect->y;
        vertices[2] = pDstRect->x+pDstRect->w;
        vertices[3] = pDstRect->y;
        vertices[4] = pDstRect->x;
        vertices[5] = pDstRect->y+pDstRect->h;
        vertices[6] = pDstRect->x+pDstRect->w;
        vertices[7] = pDstRect->y+pDstRect->h;
        glColor4f(1.0f,1.0f,1.0f,m_alpha);

        glTexCoordPointer(2, GL_FLOAT, 0, &texcoord[0]);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &vertices[0]);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, &faces[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); GL_GET_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); GL_GET_ERROR();
    }
ret:
    if (gray) glColor4ub(0xff,0xff,0xff,0xff);
    m_ref->unbindTexture();
}



OpenGLImageLibrary::OpenGLImageLibrary(OpenGLDrawContext *owner, OpenGLBackendUtil *backendUtil)
: m_owner(owner), m_backendUtil(backendUtil)
{}

IosSurface * OpenGLImageLibrary::createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility)
{
	OpenGLTexture *s = new OpenGLTexture(m_owner, m_backendUtil->toGL(type), w, h);
	return new IosGLSurfaceRef(m_owner, m_backendUtil, s);
}

IosSurface * OpenGLImageLibrary::loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    OpenGLRawImage *image = m_backendUtil->loadImage(type, path);
    OpenGLTexture *tex = new OpenGLTexture(m_owner, image, (specialAbility & IMAGE_READ));
    IosSurface *result = new IosGLSurfaceRef(m_owner, m_backendUtil, tex);
	result->name = path;
    return result;
}

IosFont * OpenGLImageLibrary::createFont(const char *path, int size, IosFontFx fx)
{
    return new OpenGLIosFont(m_backendUtil, path, size, fx);
}


void OpenGLDrawContext::init(OpenGLBackendUtil *backendUtil, int width, int height, int viewportWidth, int viewportHeight)
{
    m_backendUtil = backendUtil;
    m_offsetX = 0.f;
    m_offsetY = 0.f;
    //glMutex = new ios_fc::Mutex();
    // Retrieve the correct view size
    this->h = height;
    this->w = width;
    m_viewportWidth = (viewportWidth == 0 ? width : viewportWidth);
    m_viewportHeight = (viewportHeight == 0 ? height : viewportHeight);

    // Setup OpenGL projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
#ifdef OPENGLES
    glOrthof(0.0, (GLfloat) this->w, (GLfloat) this->h, 0.0, 0.0, 1.0); GL_GET_ERROR();

#else
    glOrtho(0.0, (GLfloat) this->w, (GLfloat) this->h, 0.0, 0.0, 1.0); GL_GET_ERROR();
#endif
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

	// Setup some defaults.
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); GL_GET_ERROR();

    // Setup Image Library
    iimLib.reset(new OpenGLImageLibrary(this, backendUtil));
}

int OpenGLDrawContext::getHeight() const
{
    return this->h;
}

int OpenGLDrawContext::getWidth() const
{
    return this->w;
}

ImageLibrary & OpenGLDrawContext::getImageLibrary()
{
    return *iimLib;
}

void OpenGLDrawContext::flip()
{
    m_backendUtil->ensureContextIsActive();
    BENCH.draw();
#ifdef DISABLED
    iglBindFramebufferOES(defaultFramebuffer, matrix);
    GL_GET_ERROR();
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    GL_GET_ERROR();
#endif
    m_backendUtil->flip();
    //
    //glClearColor(0.5f, 0.5f, 0.5f, 1.0f); GL_GET_ERROR();
    //glClear(GL_COLOR_BUFFER_BIT); GL_GET_ERROR();
}

void OpenGLDrawContext::draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    m_backendUtil->ensureContextIsActive();
    glTranslatef(m_offsetX, m_offsetY, 0.0f);
	IosRect *pSrcRect, *pDstRect;
	fixRects(srcRect, dstRect, surf, this, &pSrcRect, &pDstRect);
    IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef *> (surf);
	ipSurf->applyBlendMode(IMAGE_BLEND);
    if (m_clipRectPtr == NULL) {
        ipSurf->drawToGL(pSrcRect, pDstRect);
    }
    else {
        // SrcRect computation
        IosRect sSrcResult;
        float scalex = 1.f, scaley = 1.f;
        if ((pSrcRect->h != 0 && pDstRect->h != 0) && (pSrcRect->h != pDstRect->h))
            scaley = (float)pSrcRect->h / (float)pDstRect->h;
        if ((pSrcRect->w != 0 && pDstRect->w != 0) && (pSrcRect->w != pDstRect->w))
            scalex = (float)pSrcRect->w / (float)pDstRect->w;
        IosRect srcClip = {m_clipRect.x - pDstRect->x + pSrcRect->x,
            m_clipRect.y - pDstRect->y + pSrcRect->y,
            scalex*m_clipRect.w,
            scaley*m_clipRect.h };
        if (! pSrcRect->hasIntersection(srcClip, sSrcResult))
            goto END;
        // DstRect computation
        IosRect sDstResult;
        if (! pDstRect->hasIntersection(m_clipRect, sDstResult))
            goto END;
        ipSurf->drawToGL(&sSrcResult, &sDstResult);
    }
END:
    glTranslatef(-m_offsetX, -m_offsetY, 0.0f);
}

void OpenGLDrawContext::drawRotatedCentered(IosSurface *surf, int angle, int x, int y) {
	// TODO: Rotate.
    m_backendUtil->ensureContextIsActive();
	IosRect *pSrcRect, *pDstRect;
    IosRect dstRect = {x, y, surf->w, surf->h};
	fixRects(NULL, &dstRect, surf, this, &pSrcRect, &pDstRect);
	IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef *> (surf);
	ipSurf->applyBlendMode(IMAGE_BLEND);
	ipSurf->drawToGL(pSrcRect, pDstRect);
	/*this->bindFBO();
     IosRect *pSrcRect, *pDstRect;
     fixRects(NULL, NULL, surf, this, &pSrcRect, &pDstRect);
     IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef *> (surf);
     ipSurf->drawToGL(pSrcRect, pDstRect, this->h);*/
}
void OpenGLDrawContext::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect) {
    m_backendUtil->ensureContextIsActive();
	IosRect *pSrcRect, *pDstRect;
	fixRects(srcRect, dstRect, surf, this, &pSrcRect, &pDstRect);
	IosGLSurfaceRef *ipSurf = static_cast<IosGLSurfaceRef *> (surf);
	ipSurf->applyBlendMode(IMAGE_BLEND);
	ipSurf->drawToGL(pSrcRect, pDstRect, IosGLSurfaceRef::X_FLIP);
}

void OpenGLDrawContext::setClipRect(IosRect *rect) {
    if (rect == NULL)
        m_clipRectPtr = NULL;
    else {
        m_clipRect = *rect;
        m_clipRectPtr = &m_clipRect;
    }
}

void OpenGLDrawContext::fillRect(const IosRect *rect, const RGBA &color) {
}

void OpenGLDrawContext::putString(IosFont *font, int x, int y, const char *text) {
    m_backendUtil->ensureContextIsActive();
	OpenGLIosFont *ifont = static_cast<OpenGLIosFont*> (font);
	ifont->print(x,y,text);
}
void OpenGLDrawContext::putStringWithShadow(IosFont *font, int x, int y, int shadow_x, int shadow_y, const char *text) {
    m_backendUtil->ensureContextIsActive();
	OpenGLIosFont *ifont = static_cast<OpenGLIosFont*> (font);
	ifont->printWithShadow(x,y,shadow_x,shadow_y,text);
}

void OpenGLDrawContext::putStringCenteredXY(IosFont *font, int x, int y, const char *text) {
	m_backendUtil->ensureContextIsActive();
	OpenGLIosFont *ifont = static_cast<OpenGLIosFont*> (font);
	ifont->printCentered(x,y-ifont->getHeight()/2,text);
}

void OpenGLDrawContext::setOffset(int offX, int offY)
{
    m_offsetX = offX;
    m_offsetY = offY;
}

void OpenGLDrawContext::startFrame()
{
    BENCH.start_frame();
}

void OpenGLDrawContext::endFrame()
{
    BENCH.end_frame();
}

ImageSpecialAbility OpenGLDrawContext::guessRequiredImageAbility(const ImageOperationList &list)
{
    /*shiftHue;
     bool shiftHSV;
     bool setValue;
     bool resizeAlpha;
     bool mirrorH;
     bool convertToGray;*/
    if ((list.shiftHue) || (list.shiftHSV))
        return IMAGE_READ;
    return IMAGE_NO_ABILITY;
}

