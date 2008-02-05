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
    virtual ~FramePicture() {}
    void render(SDL_Surface *surf, Vec3 bsize);
private:
    IIM_Surface *m_frameSurface;
    int m_leftW, m_middleW, m_rightW;
    int m_topH, m_middleH, m_bottomH;
};

class Frame : public VBox {
public:
    Frame(FramePicture frameSurface, GameLoop *loop = NULL);
    virtual ~Frame();
    virtual void draw(SDL_Surface *screen);
    void setBorderVisible(bool borderVisible) { m_borderVisible = borderVisible; }
    bool getBorderVisible() const { return m_borderVisible; }
private:
    FramePicture m_frameSurface;
    SDL_Surface *m_bgSurface;
    bool m_borderVisible;
};

}

#endif // FRAME_H

