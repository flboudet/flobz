/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gborios@ios-software.com>
 *
 * iOS Software <http://ios.free.fr>
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
    AnimatedPuyo(PuyoState state);
    virtual ~AnimatedPuyo();
    void addAnimation(PuyoAnimation *animation);
    PuyoAnimation * getCurrentAnimation() const;
    void removeCurrentAnimation();
    void cycleAnimation();
    void render(SDL_Painter &painter, PuyoView *attachedView);
    bool isRenderingAnimation() const;
private:
    IosVector animationQueue;
    int puyoEyeState;
    unsigned int smallTicksCount;
};


class AnimatedPuyoFactory : public PuyoFactory {
public:
    AnimatedPuyoFactory();
    virtual ~AnimatedPuyoFactory();
    virtual PuyoPuyo *createPuyo(PuyoState state);
    virtual void deletePuyo(PuyoPuyo *target);
    void renderWalhalla(SDL_Painter &painter, PuyoView *attachedView);
    void cycleWalhalla();
private:
    IosVector puyoWalhalla;
};

#endif // _ANIMATEDPUYO

