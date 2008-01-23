/*
 *  Frame.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 23/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Frame.h"

namespace gameui {

const int Frame::leftW = 25, Frame::middleW = 28, Frame::rightW = 25;
const int Frame::topH = 19, Frame::middleH = 26, Frame::bottomH = 23;

Frame::Frame(IIM_Surface *frameSurface, GameLoop *loop) : VBox(loop), m_frameSurface(frameSurface), m_bgSurface(NULL)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(9);
}

Frame::~Frame()
{}

void Frame::draw(SDL_Surface *screen)
{
    Vec3 bsize = getSize();
    SDL_Rect srcrect, dstrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.h = bsize.y;
    srcrect.w = bsize.x;
    
    dstrect.x = getPosition().x;
    dstrect.y = getPosition().y;
    dstrect.h = bsize.y;
    dstrect.w = bsize.x;
    
    // If the background of the frame has not been created or has changed size, recreate it
    if ((m_bgSurface == NULL) || (bsize.x != m_bgSurface->w) || (bsize.y != m_bgSurface->h)) {
        if (m_bgSurface != NULL)
            SDL_FreeSurface(m_bgSurface);
        Uint32 rmask, gmask, bmask, amask;
        /* SDL interprets each pixel as a 32-bit number, so our masks must depend
           on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
        m_bgSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, bsize.x, bsize.y, 32, 
                                      rmask, gmask, bmask, amask);
        SDL_SetAlpha(m_frameSurface->surf, 0, SDL_ALPHA_OPAQUE);
        // Draw the corners first
        // Top left corner
        IIM_Rect src_rect = {0, 0, leftW, topH};
        SDL_Rect dst_rect = {0, 0, leftW, topH};
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Top right corner
        src_rect.x = leftW + middleW; src_rect.y = 0; src_rect.w = rightW; src_rect.h = topH;
        dst_rect.x = bsize.x - rightW; dst_rect.y = 0; dst_rect.w = rightW; dst_rect.h = topH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Bottom left corner
        src_rect.x = 0; src_rect.y = topH + middleH; src_rect.w = leftW; src_rect.h = bottomH;
        dst_rect.x = 0; dst_rect.y = bsize.y - bottomH; dst_rect.w = leftW; dst_rect.h = bottomH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Bottom right corner
        src_rect.x = leftW + middleW; src_rect.y = topH + middleH; src_rect.w = rightW; src_rect.h = bottomH;
        dst_rect.x = bsize.x - rightW; dst_rect.y = bsize.y - bottomH; dst_rect.w = rightW; dst_rect.h = bottomH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Top edge
        src_rect.x = leftW; src_rect.y = 0; src_rect.w = middleW; src_rect.h = topH;
        dst_rect.y = 0; dst_rect.w = middleW; dst_rect.h = topH;
        for (dst_rect.x = leftW ; dst_rect.x < bsize.x - rightW ; dst_rect.x += middleW) {
            if (dst_rect.x + middleW > bsize.x - rightW)
                dst_rect.w = src_rect.w = bsize.x - rightW - dst_rect.x;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Bottom edge
        src_rect.x = leftW; src_rect.y = topH + middleH; src_rect.w = middleW; src_rect.h = bottomH;
        dst_rect.y = bsize.y - bottomH; dst_rect.w = middleW; dst_rect.h = bottomH;
        for (dst_rect.x = leftW ; dst_rect.x < bsize.x - rightW ; dst_rect.x += middleW) {
            if (dst_rect.x + middleW > bsize.x - rightW)
                dst_rect.w = src_rect.w = bsize.x - rightW - dst_rect.x;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Left edge
        src_rect.x = 0; src_rect.y = topH; src_rect.w = leftW; src_rect.h = middleH;
        dst_rect.x = 0; dst_rect.w = leftW; dst_rect.h = middleH;
        for (dst_rect.y = topH ; dst_rect.y < bsize.y - bottomH ; dst_rect.y += middleH) {
            if (dst_rect.y + middleH > bsize.y - bottomH)
                dst_rect.h = src_rect.h = bsize.y - bottomH - dst_rect.y;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Right edge
        src_rect.x = leftW + middleW; src_rect.y = topH; src_rect.w = rightW; src_rect.h = middleH;
        dst_rect.x = bsize.x - rightW; dst_rect.w = rightW; dst_rect.h = middleH;
        for (dst_rect.y = topH ; dst_rect.y < bsize.y - bottomH ; dst_rect.y += middleH) {
            if (dst_rect.y + middleH > bsize.y - bottomH)
                dst_rect.h = src_rect.h = bsize.y - bottomH - dst_rect.y;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Content rect
        src_rect.x = leftW; src_rect.y = topH; src_rect.w = bsize.x - leftW - rightW; src_rect.h = bsize.y - topH - bottomH;
        SDL_FillRect(m_bgSurface, &src_rect, (m_bgSurface->format->Rmask & 0x00000000) |
                                             (m_bgSurface->format->Gmask & 0x00000000) |
                                             (m_bgSurface->format->Bmask & 0x00000000) |
                                             (m_bgSurface->format->Amask & 0x80808080));
    }
    // Drawing the background
    SDL_BlitSurface(m_bgSurface, &srcrect, screen, &dstrect);
    VBox::draw(screen);
}

}

