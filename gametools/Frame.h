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

class Frame : public VBox {
public:
    Frame(IIM_Surface *frameSurface, GameLoop *loop = NULL);
    virtual ~Frame();
    virtual void draw(SDL_Surface *screen);
    void setBorderVisible(bool borderVisible) { m_borderVisible = borderVisible; }
    bool getBorderVisible() const { return m_borderVisible; }
private:
    static const int leftW, middleW, rightW;
    static const int topH, middleH, bottomH;
    IIM_Surface *m_frameSurface;
    SDL_Surface *m_bgSurface;
    bool m_borderVisible;
};

}

#endif // FRAME_H

