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

FramePicture::FramePicture(IIM_Surface *frameSurface, int leftW, int middleW, int rightW, int topH, int middleH, int bottomH)
    : m_frameSurface(frameSurface), m_leftW(leftW), m_middleW(middleW), m_rightW(rightW),
      m_topH(topH), m_middleH(middleH), m_bottomH(bottomH)
{
}

void FramePicture::render(SDL_Surface *surf)
{
    int surfW = surf->w;
    int surfH = surf->h;
    SDL_SetAlpha(m_frameSurface->surf, 0, SDL_ALPHA_OPAQUE);
    // Draw the corners first
    // Top left corner
    IIM_Rect src_rect = {0, 0, m_leftW, m_topH};
    SDL_Rect dst_rect = {0, 0, m_leftW, m_topH};
    IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    // Top right corner
    src_rect.x = m_leftW + m_middleW; src_rect.y = 0; src_rect.w = m_rightW; src_rect.h = m_topH;
    dst_rect.x = surfW - m_rightW; dst_rect.y = 0; dst_rect.w = m_rightW; dst_rect.h = m_topH;
    IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    // Bottom left corner
    src_rect.x = 0; src_rect.y = m_topH + m_middleH; src_rect.w = m_leftW; src_rect.h = m_bottomH;
    dst_rect.x = 0; dst_rect.y = surfH - m_bottomH; dst_rect.w = m_leftW; dst_rect.h = m_bottomH;
    IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    // Bottom right corner
    src_rect.x = m_leftW + m_middleW; src_rect.y = m_topH + m_middleH; src_rect.w = m_rightW; src_rect.h = m_bottomH;
    dst_rect.x = surfW - m_rightW; dst_rect.y = surfH - m_bottomH; dst_rect.w = m_rightW; dst_rect.h = m_bottomH;
    IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    // Top edge
    src_rect.x = m_leftW; src_rect.y = 0; src_rect.w = m_middleW; src_rect.h = m_topH;
    dst_rect.y = 0; dst_rect.w = m_middleW; dst_rect.h = m_topH;
    for (dst_rect.x = m_leftW ; dst_rect.x < surfW - m_rightW ; dst_rect.x += m_middleW) {
        if (dst_rect.x + m_middleW > surfW - m_rightW)
            dst_rect.w = src_rect.w = surfW - m_rightW - dst_rect.x;
        IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    }
    // Bottom edge
    src_rect.x = m_leftW; src_rect.y = m_topH + m_middleH; src_rect.w = m_middleW; src_rect.h = m_bottomH;
    dst_rect.y = surfH - m_bottomH; dst_rect.w = m_middleW; dst_rect.h = m_bottomH;
    for (dst_rect.x = m_leftW ; dst_rect.x < surfW - m_rightW ; dst_rect.x += m_middleW) {
        if (dst_rect.x + m_middleW > surfW - m_rightW)
            dst_rect.w = src_rect.w = surfW - m_rightW - dst_rect.x;
        IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    }
    // Left edge
    src_rect.x = 0; src_rect.y = m_topH; src_rect.w = m_leftW; src_rect.h = m_middleH;
    dst_rect.x = 0; dst_rect.w = m_leftW; dst_rect.h = m_middleH;
    for (dst_rect.y = m_topH ; dst_rect.y < surfH - m_bottomH ; dst_rect.y += m_middleH) {
        if (dst_rect.y + m_middleH > surfH - m_bottomH)
            dst_rect.h = src_rect.h = surfH - m_bottomH - dst_rect.y;
        IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    }
    // Right edge
    src_rect.x = m_leftW + m_middleW; src_rect.y = m_topH; src_rect.w = m_rightW; src_rect.h = m_middleH;
    dst_rect.x = surfW - m_rightW; dst_rect.w = m_rightW; dst_rect.h = m_middleH;
    for (dst_rect.y = m_topH ; dst_rect.y < surfH - m_bottomH ; dst_rect.y += m_middleH) {
        if (dst_rect.y + m_middleH > surfH - m_bottomH)
            dst_rect.h = src_rect.h = surfH - m_bottomH - dst_rect.y;
        IIM_BlitSurface(m_frameSurface, &src_rect, surf, &dst_rect);
    }
    // Content rect
    src_rect.x = m_leftW; src_rect.y = m_topH; src_rect.w = surfW - m_leftW - m_rightW; src_rect.h = surfH - m_topH - m_bottomH;
    SDL_FillRect(surf, &src_rect, (surf->format->Rmask & 0x00000000) |
                 (surf->format->Gmask & 0x00000000) |
                 (surf->format->Bmask & 0x00000000) |
                 (surf->format->Amask & 0x80808080));
}

Frame::Frame(FramePicture frameSurface, GameLoop *loop)
  : VBox(loop), m_frameSurface(frameSurface), m_bgSurface(NULL), m_borderVisible(true)
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
        m_frameSurface.render(m_bgSurface);
    }
    // Drawing the background
    if (m_borderVisible)
        SDL_BlitSurface(m_bgSurface, &srcrect, screen, &dstrect);
    VBox::draw(screen);
}

void Frame::add (Widget *child)
{
  Vec3 childSize = child->getPreferedSize();
  if (! childSize.is_zero())
    childSize += 18;
  setPreferedSize(childSize);
  VBox::add(child);
}

}

