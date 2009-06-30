#include "BufferedDrawTarget.h"

BufferedDrawTarget::BufferedDrawTarget(DrawContext *dc)
    : m_frontRoot(&m_rootNodes[0]), m_backRoot(&m_rootNodes[1]), m_dc(dc),
      m_dirty(false)
{
    h = dc->h; w = dc->w;
    m_cache = m_dc->getIIMLibrary().create_DisplayFormat(dc->w, dc->h);
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
    IosRect lsrcRect, ldstRect;
    DrawOperation newOp(surf,
                        IOSRECTPTR_TO_IOSRECT(srcRect, lsrcRect, surf->h, surf->w),
                        IOSRECTPTR_TO_IOSRECT(dstRect, ldstRect, h, w));
    if (!m_dirty) {
        // Set the first node of the tree
        *m_frontRoot = DrawTreeNode(newOp);
        m_dirty = true;
    }
    m_frontRoot->appendDrawOperation(newOp);
}

BufferedDrawTarget::DrawTreeNode::DrawTreeNode(DrawOperation &op)
    : nodeRect(op.dstRect), drawOperation(op), innerNodes(NULL)
{}

BufferedDrawTarget::DrawTreeNode::~DrawTreeNode()
{
    if (innerNodes != NULL)
        delete innerNodes;
}

void BufferedDrawTarget::DrawTreeNode::appendDrawOperation(DrawOperation &drawOp)
{
    // Search for the matching intersecting node
    if (innerNodes != NULL) {
        for (DrawTreeNodeList::iterator iter = innerNodes->begin() ;
             iter != innerNodes->end() ; iter++) {
            if (iter->nodeRect.hasIntersection(drawOp.dstRect)) {
                iter->appendDrawOperation(drawOp);
                return;
            }
        }
    }
    else
        innerNodes = new DrawTreeNodeList();
    // No intersecting node has been found, append to current node
    innerNodes->push_back(DrawTreeNode(drawOp));
}

void BufferedDrawTarget::DrawTreeNode::draw(DrawTarget *dt, DrawTreeNode &backNode)
{
    dt->renderCopy(drawOperation.surf, &drawOperation.srcRect, &drawOperation.dstRect);
    if (innerNodes != NULL) {
        for (DrawTreeNodeList::iterator iter = innerNodes->begin() ;
             iter != innerNodes->end() ; iter++) {
            iter->draw(dt, backNode);
        }
    }
}

void BufferedDrawTarget::flush()
{
    m_cache->setClipRect(NULL);
    // Clear the damaged areas
    // Draw
    m_frontRoot->draw(m_cache, *m_backRoot);
    // Switch the front tree and the back tree
    DrawTreeNode *backRoot = m_backRoot;
    m_backRoot = m_frontRoot;
    m_frontRoot = backRoot;
#ifdef DISABLED
    for (std::vector<DrawElt>::iterator iter = m_frontList->begin() ;
         iter != m_frontList->end() ; iter++) {
        m_cache->renderCopy(iter->surf, &(iter->srcRect), &(iter->dstRect));
    }
    m_backList->clear();
    std::vector<DrawElt> *backList = m_backList;
    m_backList = m_frontList;
    m_frontList = backList;
#endif
    m_dirty = false;
}

IosSurface *BufferedDrawTarget::getSurface()
{
    if (m_dirty)
        flush();
    return m_cache;
}

