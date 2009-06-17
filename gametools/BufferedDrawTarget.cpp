#include "BufferedDrawTarget.h"

BufferedDrawTarget::BufferedDrawTarget(DrawContext *dc)
    : m_frontList(&m_drawList[0]), m_backList(&m_drawList[1]), m_dc(dc),
      m_dirty(false)
{
    h = dc->h; w = dc->w;
    m_cache = m_dc->getIIMLibrary().create_DisplayFormat(dc->h, dc->w);
}

BufferedDrawTarget::~BufferedDrawTarget()
{
    delete m_cache;
}

#define IOSRECTPTR_TO_IOSRECT(iosrectptr, iosrect, H, W) \
    ((iosrectptr == NULL) ? \
    ( iosrect.h = (H), iosrect.w = (W), \
        iosrect.x = 0, iosrect.y = 0, iosrect) \
        : (*iosrectptr))

void BufferedDrawTarget::renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect)
{
    m_dirty = true;
    IosRect lsrcRect, ldstRect;
    m_frontList->push_back(DrawElt(surf,
                                   IOSRECTPTR_TO_IOSRECT(srcRect, lsrcRect, surf->h, surf->w),
                                   IOSRECTPTR_TO_IOSRECT(dstRect, ldstRect, h, w)));
}

void BufferedDrawTarget::flush()
{
    for (std::vector<DrawElt>::iterator iter = m_frontList->begin() ;
         iter != m_frontList->end() ; iter++) {
        m_dc->renderCopy(iter->surf, &(iter->srcRect), &(iter->dstRect));
    }
    m_backList->clear();
    std::vector<DrawElt> *backList = m_backList;
    m_backList = m_frontList;
    m_frontList = backList;
    m_dirty = false;
}

IosSurface *BufferedDrawTarget::getSurface()
{
    if (m_dirty)
        flush();
    return m_cache;
}

