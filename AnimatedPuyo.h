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

#include "ios_memory.h"
#include "PuyoGame.h"
#include "PuyoAnimations.h"

class PuyoView;
class PuyoTheme;
class PuyoSetTheme;

class AnimatedPuyo : public PuyoPuyo {
public:
    enum AnimatedPuyoState {
        PUYO_NORMAL,
        PUYO_CRUNSHED
    };
    AnimatedPuyo(PuyoState state, PuyoSetTheme *themeSet, PuyoView *attachedView);
    virtual ~AnimatedPuyo();
    void addAnimation(PuyoAnimation *animation);
    PuyoAnimation * getCurrentAnimation() const;
    void removeCurrentAnimation();
    void flushAnimations();
    void flushAnimations(int animationTag);
    void cycleAnimation();
    void render(DrawTarget *dt);
    void renderAt(int X, int Y, DrawTarget *dt);
    void renderShadow(DrawTarget *dt);
    void renderShadowAt(int X, int Y, DrawTarget *dt);
    bool isRenderingAnimation() const;
    void setVisible(bool flag) { visibilityFlag = flag; }
    bool getVisible() const { return visibilityFlag; }
    PuyoView *getAttachedView() const { return attachedView; }
    int getScreenCoordinateX() const;
    int getScreenCoordinateY() const;
    const PuyoTheme *getAttachedTheme() const { return attachedTheme; }
    void setAnimatedState(int animatedState) { m_currentCompressedState = animatedState; }
    void setPartner(AnimatedPuyo *partner) { m_partner = partner; }
    void setOffsetX(int offsetX) { m_offsetX = offsetX; }
    void setOffsetY(int offsetY) { m_offsetY = offsetY; }
    void setRotation(float angle) { m_angle = angle; }
    void setShowEyes(bool show) { m_displayEyes = show; }
private:
    AdvancedBuffer<PuyoAnimation *> animationQueue;
    int puyoEyeState;
    unsigned int smallTicksCount;
    bool visibilityFlag;
    PuyoView *attachedView;
	const PuyoTheme *attachedTheme;
    int m_currentCompressedState;
    AnimatedPuyo *m_partner;
    int m_offsetX, m_offsetY;
    float m_angle;
    bool m_displayEyes;
};

class AnimatedPuyoFactory : public PuyoFactory {
public:
    AnimatedPuyoFactory(PuyoView *attachedView);
    virtual ~AnimatedPuyoFactory();
    virtual PuyoPuyo *createPuyo(PuyoState state);
    virtual void deletePuyo(PuyoPuyo *target);
    void renderWalhalla(DrawTarget *dt);
    void cycleWalhalla();
    void setShowEyes(bool show) { m_showEyes = show; }
private:
    AdvancedBuffer<PuyoPuyo *> puyoWalhalla;
    PuyoView *attachedView;
    PuyoSetTheme *attachedThemeSet;
    bool m_showEyes;
};


#endif // _ANIMATEDPUYO

