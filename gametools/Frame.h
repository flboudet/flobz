/*
 *  Frame.h
 *  flobopuyo
 *
 *  Created by Florent Boudet on 23/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef FRAME_H
#define FRAME_H

#include "gameui.h"

namespace gameui {

class FramePicture {
public:
    FramePicture(IIM_Surface *frameSurface, int leftW, int middleW, int rightW, int topH, int middleH, int bottomH);
    FramePicture(int leftW, int middleW, int rightW, int topH, int middleH, int bottomH);
    virtual ~FramePicture() {}
    void setFrameSurface(IIM_Surface *frameSurface);
    void setContentColor(RGBA contentColor) { m_contentColor = contentColor; }
    void render(SDL_Surface *surf) const;
    int getMaxMargin() const;
private:
    IIM_Surface *m_frameSurface;
    int m_leftW, m_middleW, m_rightW;
    int m_topH, m_middleH, m_bottomH;
    RGBA m_contentColor;
};

class Frame : public VBox {
public:
    Frame(const FramePicture *frameSurface, GameLoop *loop = NULL);
    virtual ~Frame();
    virtual void draw(SDL_Surface *screen);
    void setBorderVisible(bool borderVisible) { m_borderVisible = borderVisible; }
    bool getBorderVisible() const { return m_borderVisible; }
    void setFocusedPicture(const FramePicture *focusedSurface) { m_focusedSurface = focusedSurface; }
private:
    void cacheSurface(IIM_Surface * &cachedSurface, const FramePicture *framePicture);
    const FramePicture *m_frameSurface;
    const FramePicture *m_focusedSurface;
    IIM_Surface *m_bgSurface, *m_bgFocus;
    bool m_borderVisible;
};

}

#endif // FRAME_H

