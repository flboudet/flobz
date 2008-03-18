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
    : m_leftW(leftW), m_middleW(middleW), m_rightW(rightW),
      m_topH(topH), m_middleH(middleH), m_bottomH(bottomH)
{
  setFrameSurface(frameSurface);
}

FramePicture::FramePicture(int leftW, int middleW, int rightW, int topH, int middleH, int bottomH)
    : m_leftW(leftW), m_middleW(middleW), m_rightW(rightW),
      m_topH(topH), m_middleH(middleH), m_bottomH(bottomH)
{
}

void FramePicture::setFrameSurface(IIM_Surface *frameSurface)
{
  if (frameSurface && frameSurface->surf)
  {
    m_frameSurface = frameSurface;
    m_contentColor = iim_surface_get_rgba(frameSurface->surf,m_leftW,m_topH);
  }
}

int FramePicture::getMaxMargin() const
{
  int a = m_leftW;
  int b = m_rightW;
  int c = m_topH;
  int d = m_bottomH;

  int e = (a>b)?a:b;
  int f = (c>d)?c:d;
  return (e>f)?e:f;
}

void FramePicture::render(SDL_Surface *surf) const
{
  if (m_frameSurface && m_frameSurface->surf) {
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
    SDL_FillRect(surf, &src_rect,
		 (surf->format->Rmask & ((m_contentColor.red)      |
					 (m_contentColor.red<<8)   |
					 (m_contentColor.red<<16)  |
					 (m_contentColor.red<<24))) |
                 (surf->format->Gmask & ((m_contentColor.green)    |
					 (m_contentColor.green<<8) |
					 (m_contentColor.green<<16)|
					 (m_contentColor.green<<24)))|
                 (surf->format->Bmask & ((m_contentColor.blue)    |
					 (m_contentColor.blue<<8) |
					 (m_contentColor.blue<<16)|
					 (m_contentColor.blue<<24)))|
                 (surf->format->Amask & ((m_contentColor.alpha)    |
					 (m_contentColor.alpha<<8) |
					 (m_contentColor.alpha<<16)|
					 (m_contentColor.alpha<<24))));
  }
}

Frame::Frame(const FramePicture *frameSurface, GameLoop *loop)
  : VBox(loop), m_frameSurface(frameSurface), m_focusedSurface(NULL),
    m_bgSurface(NULL), m_bgFocus(NULL), m_borderVisible(true)
{
    setPolicy(USE_MIN_SIZE);
    setInnerMargin(9);
}

Frame::~Frame()
{
  if (m_bgSurface)
    IIM_Free(m_bgSurface);
  if (m_bgFocus)
    IIM_Free(m_bgFocus);
}

void Frame::draw(SDL_Surface *screen)
{
    Vec3 bsize = getSize();
    SDL_Rect srcrect, dstrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.h = (int)(bsize.y);
    srcrect.w = (int)(bsize.x);
    
    dstrect.x = (int)(getPosition().x);
    dstrect.y = (int)(getPosition().y);
    dstrect.h = (int)(bsize.y);
    dstrect.w = (int)(bsize.x);
    
    if (haveFocus() && (m_focusedSurface != NULL)) {
      cacheSurface(m_bgFocus, m_focusedSurface);
      // Drawing the background
      if (m_borderVisible)
        IIM_BlitSurface(m_bgFocus, &srcrect, screen, &dstrect);
    }
    else {
      cacheSurface(m_bgSurface, m_frameSurface);
      // Drawing the background
      if (m_borderVisible)
        IIM_BlitSurface(m_bgSurface, &srcrect, screen, &dstrect);
    }
    VBox::draw(screen);
}

void Frame::cacheSurface(IIM_Surface * &cachedSurface, const FramePicture *framePicture)
{
  Vec3 bsize = getSize();
  // If the background of the frame has not been created or has changed size, recreate it
  if ((cachedSurface == NULL) || ((int)(bsize.x) != cachedSurface->w) || ((int)(bsize.y) != cachedSurface->h)) {
    if (cachedSurface != NULL)
      IIM_Free(cachedSurface);
    cachedSurface = iim_surface_create_rgba((int)(bsize.x), (int)(bsize.y));
    framePicture->render(cachedSurface->surf);
  }
}

}

