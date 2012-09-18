#include <iostream>
#include <memory>
#include "CompositeDrawContext.h"

using namespace std;
using namespace ios_fc;

/**
 * CompositeSurface implementation
 */

CompositeSurface::CompositeSurface(CompositeImageLibrary &ownerImageLibrary,
                                   SharedPtr<IosSurface> baseSurface)
    : m_ownerImageLibrary(ownerImageLibrary),
      m_baseSurface(baseSurface), m_isCropped(false), m_blendMode(IMAGE_COPY)
{
    w = baseSurface->w;
    h = baseSurface->h;
}

CompositeSurface::CompositeSurface(CompositeImageLibrary &ownerImageLibrary,
                                   SharedPtr<IosSurface> baseSurface,
                                   const IosRect &cropRect)
    : m_ownerImageLibrary(ownerImageLibrary),
      m_baseSurface(baseSurface), m_isCropped(true), m_cropRect(cropRect), m_blendMode(IMAGE_COPY)
{
    w = cropRect.w;
    h = cropRect.h;
}

CompositeSurface::~CompositeSurface()
{
    // Me and the ImageLibrary
    if (m_baseSurface.refcount() == 2) {
        m_ownerImageLibrary.unregisterImage(m_baseSurface.get());
    }
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
    IosSurface *srcSurface = m_baseSurface.get();
    auto_ptr<IosSurface> tempSurface;
    if (m_isCropped) {
        DrawContext &baseDC = m_ownerImageLibrary.getBaseDrawContext();
        tempSurface.reset(baseDC.getImageLibrary().createImage(IMAGE_RGBA, m_cropRect.w, m_cropRect.h, IMAGE_READ));
        tempSurface->setBlendMode(IMAGE_COPY);
        tempSurface->draw(m_baseSurface.get(), &m_cropRect, NULL);
        srcSurface = tempSurface.get();
    }
    if (mask == NULL)
        return new CompositeSurface(m_ownerImageLibrary,
                                    srcSurface->shiftHue(hue_offset, NULL));
    CompositeSurface *m = static_cast<CompositeSurface *>(mask);
    return new CompositeSurface(m_ownerImageLibrary,
                                srcSurface->shiftHue(hue_offset, m->m_baseSurface.get()));
}

IosSurface *CompositeSurface::shiftHSV(float h, float s, float v)
{
    return new CompositeSurface(m_ownerImageLibrary,
                                m_baseSurface->shiftHSV(h, s, v));
}

IosSurface *CompositeSurface::setValue(float value)
{
    IosSurface *srcSurface = m_baseSurface.get();
    auto_ptr<IosSurface> tempSurface;
    if (m_isCropped) {
        DrawContext &baseDC = m_ownerImageLibrary.getBaseDrawContext();
        tempSurface.reset(baseDC.getImageLibrary().createImage(IMAGE_RGBA, m_cropRect.w, m_cropRect.h, IMAGE_READ));
        tempSurface->setBlendMode(IMAGE_COPY);
        tempSurface->draw(m_baseSurface.get(), &m_cropRect, NULL);
        srcSurface = tempSurface.get();
    }
    return new CompositeSurface(m_ownerImageLibrary,
                                srcSurface->setValue(value));
}

IosSurface *CompositeSurface::setAlpha(float alpha)
{
    IosSurface *srcSurface = m_baseSurface.get();
    auto_ptr<IosSurface> tempSurface;
    if (m_isCropped) {
        DrawContext &baseDC = m_ownerImageLibrary.getBaseDrawContext();
        tempSurface.reset(baseDC.getImageLibrary().createImage(IMAGE_RGBA, m_cropRect.w, m_cropRect.h, IMAGE_READ));
        tempSurface->setBlendMode(IMAGE_COPY);
        tempSurface->draw(m_baseSurface.get(), &m_cropRect, NULL);
        srcSurface = tempSurface.get();
    }
    return new CompositeSurface(m_ownerImageLibrary,
                                srcSurface->setAlpha(alpha));
}

IosSurface * CompositeSurface::resizeAlpha(int width, int height)
{
    // If the DC doesnt'have the ability to scale graphics when drawing,
    // create a new surface with the rescaled graphics
    if (! m_ownerImageLibrary.getBaseDrawContext().hasScaleAbility()) {
        IosSurface *srcSurface = m_baseSurface.get();
        auto_ptr<IosSurface> tempSurface;
        if (m_isCropped) {
            DrawContext &baseDC = m_ownerImageLibrary.getBaseDrawContext();
            tempSurface.reset(baseDC.getImageLibrary().createImage(IMAGE_RGBA, m_cropRect.w, m_cropRect.h, IMAGE_READ));
            tempSurface->setBlendMode(IMAGE_COPY);
            tempSurface->draw(m_baseSurface.get(), &m_cropRect, NULL);
            srcSurface = tempSurface.get();
        }
        return new CompositeSurface(m_ownerImageLibrary,
                                    srcSurface->resizeAlpha(width, height));
    }
    // Otherwise, just create a new CompositeSurface with the same croprect
    // and base surface, and change its dimensions
    CompositeSurface *result = new CompositeSurface(m_ownerImageLibrary, m_baseSurface, m_cropRect);
    result->w = width;
    result->h = height;
    result->m_isCropped = m_isCropped;
    return result;
}

IosSurface * CompositeSurface::mirrorH()
{
    return new CompositeSurface(m_ownerImageLibrary,
                                m_baseSurface->mirrorH());
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
    if (!m_isCropped) {
        m_baseSurface->setBlendMode(mode);
        return;
    }
    m_blendMode = mode;
}

void CompositeSurface::draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    if (!s->m_isCropped) {
        m_baseSurface->draw(s->m_baseSurface.get(), srcRect, dstRect);
        return;
    }
    s->m_baseSurface->setBlendMode(m_blendMode);
    if (srcRect == NULL) {
        m_baseSurface->draw(s->m_baseSurface.get(), &(s->m_cropRect), dstRect);
        return;
    }
    // TODO: srcRect != NULL
}

void CompositeSurface::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseSurface->drawHFlipped(s->m_baseSurface.get(), srcRect, dstRect);
}

void CompositeSurface::drawRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseSurface->drawRotatedCentered(s->m_baseSurface.get(), angle, x, y);
}

void CompositeSurface::fillRect(const IosRect *rect, const RGBA &color)
{
    m_baseSurface->fillRect(rect, color);
}

void CompositeSurface::putString(IosFont *font, int x, int y, const char *text, const RGBA &color)
{
    m_baseSurface->putString(font, x, y, text, color);
}

/**
 * CompositeImageLibrary implementation
 */

CompositeImageLibrary::CompositeImageLibrary(CompositeDrawContext &owner)
    : m_owner(owner), m_baseDrawContext(owner.getBaseDrawContext()),
      m_baseImageLibrary(m_baseDrawContext.getImageLibrary())
{
}

DrawContext & CompositeImageLibrary::getBaseDrawContext() const
{
    return m_owner.getBaseDrawContext();
}

IosSurface * CompositeImageLibrary::createImage(ImageType type, int w, int h, ImageSpecialAbility specialAbility)
{
    return new CompositeSurface(*this, m_baseImageLibrary.createImage(type, w, h, specialAbility));
}

IosSurface * CompositeImageLibrary::loadImage(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    CompositeSurfaceDefinition *def = m_owner.getCompositeSurfaceDefinition(path);
    if (def == NULL)
        return new CompositeSurface(*this, m_baseImageLibrary.loadImage(type, path, specialAbility));
    SharedPtr<IosSurface> baseSurface;
    BaseSurfaceMap::iterator existingBaseSurface = m_baseSurfaceMap.find(def->getPath());
    if (existingBaseSurface == m_baseSurfaceMap.end()) {
        baseSurface = SharedPtr<IosSurface>(m_baseImageLibrary.loadImage(type, def->getPath(), specialAbility));
        m_baseSurfaceMap[def->getPath()] = baseSurface;
    }
    else {
        baseSurface = existingBaseSurface->second;
    }
    CompositeSurface *result = new CompositeSurface(*this, baseSurface, def->getCropRect());
    return result;
}

IosFont * CompositeImageLibrary::createFont(const char *path, int size)
{
    return m_baseImageLibrary.createFont(path, size);
}

void CompositeImageLibrary::unregisterImage(IosSurface *image)
{
    for (BaseSurfaceMap::iterator iter = m_baseSurfaceMap.begin();
         iter != m_baseSurfaceMap.end() ; ++iter) {
        if (iter->second.get() == image) {
            m_baseSurfaceMap.erase(iter);
            return;
        }
    }
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

void CompositeDrawContext::setOffset(int offX, int offY)
{
    m_baseDrawContext->setOffset(offX, offY);
}

bool CompositeDrawContext::hasScaleAbility() const
{
    return m_baseDrawContext->hasScaleAbility();
}

ImageSpecialAbility CompositeDrawContext::guessRequiredImageAbility(const ImageOperationList &list)
{
    return m_baseDrawContext->guessRequiredImageAbility(list);
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
        m_baseDrawContext->draw(s->m_baseSurface.get(), srcRect, dstRect);
        return;
    }
    if (srcRect == NULL) {
        m_baseDrawContext->draw(s->m_baseSurface.get(), &(s->m_cropRect), dstRect);
        return;
    }
}

void CompositeDrawContext::drawHFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    if (! s->m_isCropped) {
        m_baseDrawContext->drawHFlipped(s->m_baseSurface.get(), srcRect, dstRect);
        return;
    }
    if (srcRect == NULL) {
        m_baseDrawContext->drawHFlipped(s->m_baseSurface.get(), &(s->m_cropRect), dstRect);
        return;
    }
}

void CompositeDrawContext::drawRotatedCentered(IosSurface *surf, int angle, int x, int y)
{
    CompositeSurface *s = static_cast<CompositeSurface *>(surf);
    m_baseDrawContext->drawRotatedCentered(s->m_baseSurface.get(), angle, x, y);
}

void CompositeDrawContext::fillRect(const IosRect *rect, const RGBA &color)
{
    m_baseDrawContext->fillRect(rect, color);
}

void CompositeDrawContext::putString(IosFont *font, int x, int y, const char *text, const RGBA &color)
{
    m_baseDrawContext->putString(font, x, y, text, color);
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

