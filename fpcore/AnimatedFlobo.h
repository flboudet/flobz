/* FloboPop
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

#include <memory>
#include <vector>
#include "ios_memory.h"
#include "FloboGame.h"
#include "FloboAnimations.h"

class GameView;
class FloboTheme;
class FloboSetTheme;

class AnimatedFlobo : public Flobo {
public:
    enum AnimatedFloboState {
        FLOBO_NORMAL,
        FLOBO_CRUNSHED
    };
    AnimatedFlobo(FloboState state, FloboSetTheme *themeSet, GameView *attachedView);
    virtual ~AnimatedFlobo();
    void addAnimation(const std::shared_ptr<FloboAnimation> &animation);
    std::shared_ptr<FloboAnimation> getCurrentAnimation() const;
    void removeCurrentAnimation();
    void flushAnimations();
    void flushAnimations(int animationTag);
    void cycleAnimation();
    void render(DrawTarget *dt);
    void renderAt(int X, int Y, DrawTarget *dt);
    void renderShadow(DrawTarget *dt);
    void renderShadowAt(int X, int Y, DrawTarget *dt);
    bool isRenderingAnimation() const;
    void setVisible(bool flag) { _visibilityFlag = flag; }
    bool getVisible() const { return _visibilityFlag; }
    GameView *getAttachedView() const { return _attachedView; }
    int getScreenCoordinateX() const;
    int getScreenCoordinateY() const;
    const FloboTheme *getAttachedTheme() const { return _attachedTheme; }
    void setAnimatedState(int animatedState) { _currentCompressedState = animatedState; }
    void setPartner(AnimatedFlobo *partner) { _partner = partner; }
    void setOffsetX(int offsetX) { _offsetX = offsetX; }
    void setOffsetY(int offsetY) { _offsetY = offsetY; }
    void setRotation(float angle) { _angle = angle; }
    void setShowEyes(bool show) { _displayEyes = show; }
private:
    std::vector<std::shared_ptr<FloboAnimation>> _animationQueue;
    int _floboEyeState;
    unsigned int _smallTicksCount;
    bool _visibilityFlag;
    GameView *_attachedView;
	const FloboTheme *_attachedTheme;
    int _currentCompressedState;
    AnimatedFlobo *_partner;
    int _offsetX, _offsetY;
    float _angle;
    bool _displayEyes;
};

class AnimatedFloboFactory : public FloboFactory {
public:
    AnimatedFloboFactory(GameView *attachedView);
    virtual ~AnimatedFloboFactory();
    virtual std::shared_ptr<Flobo> createFlobo(FloboState state);
    void setShowEyes(bool show) { _showEyes = show; }
private:
    GameView *_attachedView;
    FloboSetTheme *_attachedThemeSet;
    bool _showEyes;
};


#endif // _ANIMATEDPUYO

