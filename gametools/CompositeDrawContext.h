#ifndef _COMPOSITEDRAWCONTEXT_H_
#define _COMPOSITEDRAWCONTEXT_H_

#include <string>
#include <map>
#include "drawcontext.h"
#include "ios_ptr.h"

class CompositeDrawContext;
class CompositeImageLibrary;

#ifdef DISABLED
template <typename T>
class SharedReference
{
public:
    SharedReference(T *p)
        : m_p(p), m_refCount(1) {}
    T *m_p;
    volatile int m_refCount;
};

template <typename T>
class SharedPtr
{
public:
    SharedPtr()
        : m_ref(NULL) {}
    SharedPtr(T *p)
        : m_ref(p == NULL?NULL:new SharedReference<T>(p)) {}
    SharedPtr(const SharedPtr &s)
        : m_ref(s.m_ref) {
        if (m_ref != NULL)
            ++(m_ref->m_refCount);
    }
    ~SharedPtr() {
        if ((m_ref != NULL)
            && (--(m_ref->m_refCount) == 0)) {
            delete m_ref->m_p;
            delete m_ref;
        }
    }
    void reset(T *p) {
        if ((m_ref != NULL)
            && (--(m_ref->m_refCount) == 0)) {
            delete m_ref->m_p;
            delete m_ref;
        }
        m_ref = (p == NULL?NULL:new SharedReference<T>(p));
    }
    /*operator T *() const {
        if (m_ref == NULL)
            return NULL;
        return m_ref->m_p;
    }*/
    T * operator ->()  const {
        //if (m_ref == NULL)
        //    return NULL;
        return m_ref->m_p;
    }
    T *get() const {
        if (m_ref == NULL)
            return NULL;
        return m_ref->m_p;
    }
    bool empty() const { return m_ref == NULL; }
private:
    SharedReference<T> *m_ref;
};
#endif


class CompositeSurfaceDefinition
{
public:
    CompositeSurfaceDefinition() {}
    CompositeSurfaceDefinition(const char *path, const IosRect &cropRect)
        : m_path(path), m_cropRect(cropRect) {}
    const char *getPath() const { return m_path.data(); }
    const IosRect &getCropRect() const { return m_cropRect; }
private:
    std::string m_path;
    IosRect m_cropRect;
};

class CompositeSurface : public IosSurface
{
public:
    CompositeSurface(CompositeImageLibrary &ownerImageLibrary,
                     ios_fc::SharedPtr<IosSurface> baseSurface);
    CompositeSurface(CompositeImageLibrary &ownerImageLibrary,
                     ios_fc::SharedPtr<IosSurface> baseSurface, const IosRect &cropRect);
    virtual ~CompositeSurface();
    // IosSurface methods
    virtual bool isOpaque() const;
	virtual bool haveAbility(int ability) const;
	virtual void dropAbility(int ability);
    virtual RGBA readRGBA(int x, int y);
    virtual IosSurface *shiftHue(float hue_offset, IosSurface *mask = NULL);
    virtual IosSurface *shiftHSV(float h, float s, float v);
    virtual IosSurface *setValue(float value);
    virtual IosSurface * resizeAlpha(int width, int height);
    virtual IosSurface * mirrorH();
    virtual void         convertToGray();
    // DrawTarget methods
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
	virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
private:
    CompositeImageLibrary &m_ownerImageLibrary;
    ios_fc::SharedPtr<IosSurface> m_baseSurface;
    bool m_isCropped;
    IosRect m_cropRect;
    ImageBlendMode m_blendMode;
    friend class CompositeDrawContext;
};

class CompositeImageLibrary : public ImageLibrary
{
public:
    CompositeImageLibrary(CompositeDrawContext &owner);
    virtual IosSurface * createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility = 0);
    virtual IosSurface * loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    virtual IosFont    * createFont(const char *path, int size, IosFontFx fx = Font_STD);
public:
    DrawContext &getBaseDrawContext() const;
    void unregisterImage(IosSurface *image);
private:
    CompositeDrawContext &m_owner;
    DrawContext  &m_baseDrawContext;
    ImageLibrary &m_baseImageLibrary;
    typedef std::map<std::string, ios_fc::SharedPtr<IosSurface> > BaseSurfaceMap;
    BaseSurfaceMap m_baseSurfaceMap;
};

class CompositeDrawContext : public DrawContext
{
public:
    CompositeDrawContext(DrawContext *baseDrawContext);
    virtual void flip() {
        m_baseDrawContext->flip();
    }
    virtual int getHeight() const {
        return m_baseDrawContext->getHeight();
    }
    virtual int getWidth() const {
        return m_baseDrawContext->getWidth();
    }
    virtual ImageLibrary &getImageLibrary() {
        return m_imageLibrary;
    }
    // Query for drawcontext abilities
    virtual bool hasScaleAbility() const;
    virtual ImageSpecialAbility guessRequiredImageAbility(const ImageOperationList &list);
    // DrawTarget implementation
    virtual void setClipRect(IosRect *rect);
    virtual void setBlendMode(ImageBlendMode mode);
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
	virtual void fillRect(const IosRect *rect, const RGBA &color);
    virtual void putString(IosFont *font, int x, int y, const char *text);
    // Specific methods
    DrawContext &getBaseDrawContext() const { return *m_baseDrawContext; }
    void declareCompositeSurface(const char *key,
                                 const char *path,
                                 IosRect &cropRect);
    void declareCompositeSurface(const char *key,
                                 const char *path,
                                 int x, int y, int w, int h);
    CompositeSurfaceDefinition *getCompositeSurfaceDefinition(const char *key);
private:
    DrawContext *m_baseDrawContext;
    CompositeImageLibrary m_imageLibrary;
    std::map<std::string, CompositeSurfaceDefinition> m_compositeSurfaceDefs;
};

#endif // _COMPOSITEDRAWCONTEXT_H_

