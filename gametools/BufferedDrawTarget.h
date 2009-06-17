#ifndef _BUFFEREDDRAWTARGET_
#define _BUFFEREDDRAWTARGET_

#include <vector>
#include "drawcontext.h"

class BufferedDrawTarget : public DrawTarget
{
public:
    BufferedDrawTarget(DrawContext *dc);
    virtual ~BufferedDrawTarget();
    virtual void renderCopy(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    //virtual void renderCopyFlipped(IosSurface *surf, IosRect *srcRect, IosRect *dstRect);
    //virtual void renderRotatedCentered(IosSurface *surf, int angle, int x, int y);
    //virtual void fillRect(const IosRect *rect, const RGBA &color);
public:
    void flush();
    IosSurface *getSurface();
private:
    struct DrawElt {
        DrawElt(IosSurface *surf, IosRect &srcRect, IosRect &dstRect)
            : surf(surf), srcRect(srcRect), dstRect(dstRect) {}
        IosSurface *surf;
        IosRect     srcRect;
        IosRect     dstRect;
    };
    std::vector<DrawElt> m_drawList[2];
    std::vector<DrawElt> *m_frontList, *m_backList;
    DrawContext *m_dc;
    IosSurface *m_cache;
    bool m_dirty;
};

#endif // _BUFFEREDDRAWTARGET_

