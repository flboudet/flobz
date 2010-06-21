#include "CompositeDrawContext.h"

/**
 * CompositeSurface implementation
 */

CompositeSurface::CompositeSurface(IosSurface *baseSurface)
    : m_baseSurface(baseSurface), m_isCropped(false)
{
    w = baseSurface->w;
    h = baseSurface->h;
}

CompositeSurface::CompositeSurface(IosSurface *baseSurface,
                                   const IosRect &cropRect)
    : m_baseSurface(baseSurface), m_isCropped(true), m_cropRect(cropRect)
{
    w = cropRect.w;
    h = cropRect.h;
}

CompositeSurface::~CompositeSurface()
{
    delete m_baseSurface;
}

// IosSurface methods
bool CompositeSurface::isOpaque() const
{
    return m_baseSurface->isOpaque();
}

bool CompositeSurface::haveAbility(int ability) const
{
    return m_baseSurface->haveAbility(ability);
}

void CompositeSurface::dropAbility(int ability)
{
    if (!m_isCropped)
        m_baseSurface->dropAbility(ability);
}

RGBA CompositeSurface::readRGBA(int x, int y)
{
    if (!m_isCropped)
        return m_baseSurface->readRGBA(x, y);
    return m_baseSurface->readRGBA(x+m_cropRect.x, y+m_cropRect.y);
}

IosSurface *CompositeSurface::shiftHue(float hue_offset, IosSurface *mask)
{
    if (mask == NULL)
        return new CompositeSurface(m_baseSurface->shiftHue(hue_offset, NULL));
    CompositeSurface *m = static_cast<CompositeSurface *>(mask);
    return new CompositeSurface(m_baseSurface->shiftHue(hue_offset, m->m_baseSurface));
}

IosSurface *CompositeSurface::shiftHSV(float h, float s, float v)
{
    return new CompositeSurface(m_baseSurface->shiftHSV(h, s, v));
}

IosSurface *CompositeSurface::setValue(float value)
{
    return new CompositeSurface(m_baseSurface->setValue(value));
}

IosSurface * CompositeSurface::resizeAlpha(int width, int height)
{
    return new CompositeSurface(m_baseSurface->resizeAlpha(width, height));
}

IosSurface * CompositeSurface::mirrorH()
{
    return new CompositeSurface(m_baseSurface->mirrorH());
}

void CompositeSurface::convertToGray()
{
    m_baseSurface->convertToGray();
}

// DrawTarget implementation
void CompositeSurface::setClipRect(IosRect *rect)
{
    m_baseSurface->setClipRect(rect);
}

void CompositeSurface::setBlendMode(ImageBlendMode mode)
{
    //if (!m_isComposite)
    m_baseSurface->setBlendMode(mode);
}

void CompositeSurface::draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    if (!s->m_isCropped)
        m_baseSurface->draw(s->m_baseSurface, srcRect, dstRect);
    if (srcRect == NULL) {
        m_baseSurface->draw(s->m_baseSurface, &(s->m_cropRect), dstRect);
        return;
    }
}

void CompositeSurface::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseSurface->drawHFlipped(s->m_baseSurface, srcRect, dstRect);
}

void CompositeSurface::drawRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseSurface->drawRotatedCentered(s->m_baseSurface, angle, x, y);
}

void CompositeSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    m_baseSurface->fillRect(rect, color);
}

void CompositeSurface::putString(IosFont *font, int x, int y, const char *text)
{
    m_baseSurface->putString(font, x, y, text);
}

/**
 * CompositeImageLibrary implementation
 */

CompositeImageLibrary::CompositeImageLibrary(CompositeDrawContext &owner)
    : m_owner(owner), m_baseDrawContext(owner.getBaseDrawContext()),
      m_baseImageLibrary(m_baseDrawContext.getImageLibrary())
{
}

IosSurface * CompositeImageLibrary::createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility)
{
    return new CompositeSurface(m_baseImageLibrary.createImage(type, w, h, specialAbility));
}

IosSurface * CompositeImageLibrary::loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    CompositeSurfaceDefinition *def = m_owner.getCompositeSurfaceDefinition(path);
    if (def == NULL)
        return new CompositeSurface(m_baseImageLibrary.loadImage(type, path, specialAbility));
    IosSurface *baseSurface = m_baseImageLibrary.loadImage(type, def->getPath(), specialAbility);
    return new CompositeSurface(baseSurface, def->getCropRect());
}

IosFont * CompositeImageLibrary::createFont(const char *path, int size, IosFontFx fx)
{
    return m_baseImageLibrary.createFont(path, size, fx);
}

/**
 * CompositeDrawContext implementation
 */

CompositeDrawContext::CompositeDrawContext(DrawContext *baseDrawContext)
    : m_baseDrawContext(baseDrawContext), m_imageLibrary(*this)
{
    w = baseDrawContext->w;
    h = baseDrawContext->h;
}

// DrawTarget implementation

void CompositeDrawContext::setClipRect(IosRect *rect)
{
    m_baseDrawContext->setClipRect(rect);
}

void CompositeDrawContext::setBlendMode(ImageBlendMode mode)
{
    m_baseDrawContext->setBlendMode(mode);
}

void CompositeDrawContext::draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    if (! s->m_isCropped) {
        m_baseDrawContext->draw(s->m_baseSurface, srcRect, dstRect);
        return;
    }
    if (srcRect == NULL) {
        m_baseDrawContext->draw(s->m_baseSurface, &(s->m_cropRect), dstRect);
        return;
    }
}

void CompositeDrawContext::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseDrawContext->drawHFlipped(s->m_baseSurface, srcRect, dstRect);
}

void CompositeDrawContext::drawRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseDrawContext->drawRotatedCentered(s->m_baseSurface, angle, x, y);
}

void CompositeDrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    m_baseDrawContext->fillRect(rect, color);
}

void CompositeDrawContext::putString(IosFont *font, int x, int y, const char *text)
{
    m_baseDrawContext->putString(font, x, y, text);
}

// Specific methods

void CompositeDrawContext::declareCompositeSurface(const char *key,
                                                   const char *path,
                                                   IosRect &cropRect)
{
    m_compositeSurfaceDefs[key] = CompositeSurfaceDefinition(path, cropRect);
}

void CompositeDrawContext::declareCompositeSurface(const char *key,
                                                   const char *path,
                                                   int x, int y, int w, int h)
{
    IosRect cropRect = {x, y, w, h};
    declareCompositeSurface(key, path, cropRect);
}

CompositeSurfaceDefinition *CompositeDrawContext::getCompositeSurfaceDefinition(const char *key)
{
    std::map<std::string, CompositeSurfaceDefinition>::iterator iter;
    iter = m_compositeSurfaceDefs.find(key);
    if (iter == m_compositeSurfaceDefs.end())
        return NULL;
    return &(iter->second);
}

