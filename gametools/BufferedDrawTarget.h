#ifndef _BUFFEREDDRAWTARGET_
#define _BUFFEREDDRAWTARGET_

#include <vector>
#include <list>
#include "drawcontext.h"

class BufferedDrawTarget : public DrawTarget
{
public:
    BufferedDrawTarget(DrawContext *dc);
    virtual ~BufferedDrawTarget();
    virtual void draw(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    //virtual void drawFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    //virtual void drawRotatedCentered(IosSurface *surf, int angle, int x, int y);
    //virtual void fillRect(const IosRect *rect, const RGBA &color);
public:
    void flush();
    IosSurface *getSurface();
private:
    struct DrawOperation {
        DrawOperation() {}
        DrawOperation(IosSurface *surf, IosRect &srcRect, IosRect &dstRect)
            : surf(surf), srcRect(srcRect), dstRect(dstRect) {}
        IosSurface *surf;
        IosRect     srcRect;
        IosRect     dstRect;
    };
    struct DrawTreeNode {
        DrawTreeNode() : innerNodes(NULL) {}
        DrawTreeNode(DrawOperation &op);
        ~DrawTreeNode();
        void appendDrawOperation(DrawOperation &drawOp);
        void draw(DrawTarget *dt, DrawTreeNode &backNode);
        IosRect nodeRect;
        DrawOperation drawOperation;
        typedef std::list<DrawTreeNode> DrawTreeNodeList;
        DrawTreeNodeList *innerNodes;
    };
    DrawTreeNode m_rootNodes[2];
    DrawTreeNode *m_frontRoot, *m_backRoot;
    DrawContext *m_dc;
    IosSurface *m_cache;
    bool m_dirty;
};

#endif // _BUFFEREDDRAWTARGET_

