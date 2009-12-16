/*
 *  Frame.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 23/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include <assert.h>
#include "Frame.h"

namespace gameui {

FramePicture::FramePicture(IosSurface *frameSurface, int leftW, int middleW, int rightW, int topH, int middleH, int bottomH)
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

void FramePicture::setFrameSurface(IosSurface *frameSurface)
{
  if (frameSurface)
  {
    m_frameSurface = frameSurface;
	if (frameSurface->haveAbility(IMAGE_READ)) {
		m_contentColor = frameSurface->readRGBA(m_leftW, m_topH);
		frameSurface->dropAbility(IMAGE_READ);
	}
	else {
		fprintf(stderr, "WARNING: Cannot read pixel color of a FramePicture: %s\n", frameSurface->name.c_str());
		m_contentColor.red = m_contentColor.green = m_contentColor.blue = m_contentColor.alpha = 0;
	}
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

void FramePicture::render(DrawTarget *surf) const
{
  if (m_frameSurface != NULL) {
    int surfW = surf->w;
    int surfH = surf->h;
    if(m_frameSurface->isOpaque() != true) {
		//->setAlpha(IOS_ALPHA_OPAQUE);
		fprintf(stderr, "WARNING: FramePicture not opaque: %s\n", m_frameSurface->name.c_str());
		assert(m_frameSurface->isOpaque() == true);
	}
    // Draw the corners first
    // Top left corner
    IosRect src_rect = {0, 0, m_leftW, m_topH};
    IosRect dst_rect = {0, 0, m_leftW, m_topH};
    surf->draw(m_frameSurface, &src_rect, &dst_rect);
    // Top right corner
    src_rect.x = m_leftW + m_middleW; src_rect.y = 0; src_rect.w = m_rightW; src_rect.h = m_topH;
    dst_rect.x = surfW - m_rightW; dst_rect.y = 0; dst_rect.w = m_rightW; dst_rect.h = m_topH;
    surf->draw(m_frameSurface, &src_rect, &dst_rect);
    // Bottom left corner
    src_rect.x = 0; src_rect.y = m_topH + m_middleH; src_rect.w = m_leftW; src_rect.h = m_bottomH;
    dst_rect.x = 0; dst_rect.y = surfH - m_bottomH; dst_rect.w = m_leftW; dst_rect.h = m_bottomH;
    surf->draw(m_frameSurface, &src_rect, &dst_rect);
    // Bottom right corner
    src_rect.x = m_leftW + m_middleW; src_rect.y = m_topH + m_middleH; src_rect.w = m_rightW; src_rect.h = m_bottomH;
    dst_rect.x = surfW - m_rightW; dst_rect.y = surfH - m_bottomH; dst_rect.w = m_rightW; dst_rect.h = m_bottomH;
    surf->draw(m_frameSurface, &src_rect, &dst_rect);
    // Top edge
    src_rect.x = m_leftW; src_rect.y = 0; src_rect.w = m_middleW; src_rect.h = m_topH;
    dst_rect.y = 0; dst_rect.w = m_middleW; dst_rect.h = m_topH;
    for (dst_rect.x = m_leftW ; dst_rect.x < surfW - m_rightW ; dst_rect.x += m_middleW) {
        if (dst_rect.x + m_middleW > surfW - m_rightW)
            dst_rect.w = src_rect.w = surfW - m_rightW - dst_rect.x;
        surf->draw(m_frameSurface, &src_rect, &dst_rect);
    }
    // Bottom edge
    src_rect.x = m_leftW; src_rect.y = m_topH + m_middleH; src_rect.w = m_middleW; src_rect.h = m_bottomH;
    dst_rect.y = surfH - m_bottomH; dst_rect.w = m_middleW; dst_rect.h = m_bottomH;
    for (dst_rect.x = m_leftW ; dst_rect.x < surfW - m_rightW ; dst_rect.x += m_middleW) {
        if (dst_rect.x + m_middleW > surfW - m_rightW)
            dst_rect.w = src_rect.w = surfW - m_rightW - dst_rect.x;
        surf->draw(m_frameSurface, &src_rect, &dst_rect);
    }
    // Left edge
    src_rect.x = 0; src_rect.y = m_topH; src_rect.w = m_leftW; src_rect.h = m_middleH;
    dst_rect.x = 0; dst_rect.w = m_leftW; dst_rect.h = m_middleH;
    for (dst_rect.y = m_topH ; dst_rect.y < surfH - m_bottomH ; dst_rect.y += m_middleH) {
        if (dst_rect.y + m_middleH > surfH - m_bottomH)
            dst_rect.h = src_rect.h = surfH - m_bottomH - dst_rect.y;
        surf->draw(m_frameSurface, &src_rect, &dst_rect);
    }
    // Right edge
    src_rect.x = m_leftW + m_middleW; src_rect.y = m_topH; src_rect.w = m_rightW; src_rect.h = m_middleH;
    dst_rect.x = surfW - m_rightW; dst_rect.w = m_rightW; dst_rect.h = m_middleH;
    for (dst_rect.y = m_topH ; dst_rect.y < surfH - m_bottomH ; dst_rect.y += m_middleH) {
        if (dst_rect.y + m_middleH > surfH - m_bottomH)
            dst_rect.h = src_rect.h = surfH - m_bottomH - dst_rect.y;
        surf->draw(m_frameSurface, &src_rect, &dst_rect);
    }
    // Content rect
    src_rect.x = m_leftW; src_rect.y = m_topH; src_rect.w = surfW - m_leftW - m_rightW; src_rect.h = surfH - m_topH - m_bottomH;
    surf->fillRect(&src_rect, m_contentColor);
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
    delete m_bgSurface;
  if (m_bgFocus)
    delete m_bgFocus;
}

void Frame::draw(DrawTarget *dt)
{
    Vec3 bsize = getSize();
    IosRect srcrect, dstrect;
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
        dt->draw(m_bgFocus, &srcrect, &dstrect);
    }
    else if (m_frameSurface) {
      cacheSurface(m_bgSurface, m_frameSurface);
      // Drawing the background
      if (m_borderVisible)
        dt->draw(m_bgSurface, &srcrect, &dstrect);
    }
    VBox::draw(dt);
}

void Frame::cacheSurface(IosSurface * &cachedSurface, const FramePicture *framePicture)
{
  Vec3 bsize = getSize();
  // If the background of the frame has not been created or has changed size, recreate it
  if ((cachedSurface == NULL) || ((int)(bsize.x) != cachedSurface->w) || ((int)(bsize.y) != cachedSurface->h)) {
    if (cachedSurface != NULL)
      delete cachedSurface;
    ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
    cachedSurface = iimLib.createImage(IMAGE_RGBA, (int)(bsize.x), (int)(bsize.y));
    framePicture->render(cachedSurface);
  }
}

}

