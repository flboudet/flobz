/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#ifndef _ANIMATEDPUYO
#define _ANIMATEDPUYO

#include "SDL_Painter.h"
#include "PuyoGame.h"
#include "PuyoAnimations.h"
#include "IosVector.h"

class PuyoView;

class AnimatedPuyo : public PuyoPuyo {
public:
    AnimatedPuyo(PuyoState state, PuyoView *attachedView);
    virtual ~AnimatedPuyo();
    void addAnimation(PuyoAnimation *animation);
    PuyoAnimation * getCurrentAnimation() const;
    void removeCurrentAnimation();
    void cycleAnimation();
    void render();
    bool isRenderingAnimation() const;
    void setVisible(bool flag) { visibilityFlag = flag; }
    bool getVisible() const { return visibilityFlag; }
    PuyoView *getAttachedView() const { return attachedView; }
    int getScreenCoordinateX() const;
    int getScreenCoordinateY() const;
private:
    IosVector animationQueue;
    int puyoEyeState;
    unsigned int smallTicksCount;
    bool visibilityFlag;
    PuyoView *attachedView;
};


class AnimatedPuyoFactory : public PuyoFactory {
public:
    AnimatedPuyoFactory(PuyoView *attachedView);
    virtual ~AnimatedPuyoFactory();
    virtual PuyoPuyo *createPuyo(PuyoState state);
    virtual void deletePuyo(PuyoPuyo *target);
    void renderWalhalla();
    void cycleWalhalla();
private:
    IosVector puyoWalhalla;
    PuyoView *attachedView;
};

#endif // _ANIMATEDPUYO

