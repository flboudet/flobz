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

#include "AnimatedFlobo.h"
#include "GameView.h"
#include "Theme.h"

AnimatedFlobo::AnimatedFlobo(FloboState state, FloboSetTheme *themeSet, GameView *attachedView)
    : Flobo(state), _smallTicksCount(0), _attachedTheme(themeSet != NULL ? &(themeSet->getFloboTheme(state)) : NULL),
      _currentCompressedState(0), _partner(NULL), _offsetX(0), _offsetY(0), _angle(0), _displayEyes(true)
{
    _floboEyeState = random() % 8192;
    _visibilityFlag = true;
    this->_attachedView = attachedView;
}

AnimatedFlobo::~AnimatedFlobo()
{
    while (_animationQueue.size() > 0)
           removeCurrentAnimation();
}

void AnimatedFlobo::addAnimation(FloboAnimation *animation)
{
    _animationQueue.add(animation);
}

FloboAnimation * AnimatedFlobo::getCurrentAnimation() const
{
    if (_animationQueue.size() == 0)
        return NULL;
    return _animationQueue[0];
}

void AnimatedFlobo::removeCurrentAnimation()
{
    if (_animationQueue.size() == 0)
        return;
    FloboAnimation *animationToRemove = _animationQueue[0];
    _animationQueue.removeKeepOrder(animationToRemove);
    delete animationToRemove;
}

void AnimatedFlobo::flushAnimations()
{
    while (getCurrentAnimation() != NULL)
        removeCurrentAnimation();
    _angle = 0.;
    _offsetX = 0;
    _offsetY = 0;
}

void AnimatedFlobo::flushAnimations(int animationTag)
{
    for (int i = _animationQueue.size() - 1 ; i >= 0 ; i--) {
        FloboAnimation *anim = _animationQueue[i];
        if (anim->getTag() == animationTag) {
            _animationQueue.removeKeepOrder(anim);
            delete anim;
        }
    }
}

void AnimatedFlobo::cycleAnimation()
{
    bool exclusive = false;
    _smallTicksCount+=2;
    for (int i = 0 ; (i < _animationQueue.size()) && (!exclusive) ; i++) {
        FloboAnimation *animation = _animationQueue[i];
        exclusive = animation->getExclusive();
        if ((!exclusive) || (i == 0)) {
            animation->cycle();
            if (animation->isFinished()) {
                _animationQueue.removeKeepOrder(animation);
                delete animation;
                i--;
                exclusive = false;
            }
        }
    }
}

bool AnimatedFlobo::isRenderingAnimation() const
{
    FloboAnimation *animation = getCurrentAnimation();
    if (animation == NULL)
        return false;
    return animation->isEnabled();
}

void AnimatedFlobo::render(DrawTarget *dt)
{
    FloboGame *attachedGame = _attachedView->getAttachedGame();
    FloboAnimation *animation = getCurrentAnimation();
    if (!isRenderingAnimation()) {
        renderAt(getScreenCoordinateX(), getScreenCoordinateY(), dt);
    }
    else {
        if (!animation->isFinished()) {
            animation->draw(attachedGame->getSemiMove(), dt);
        }
    }
}

void AnimatedFlobo::renderAt(int X, int Y, DrawTarget *dt)
{
    if (_attachedView == NULL)
        return;
    if (!_visibilityFlag)
        return;
    FloboGame *attachedGame = _attachedView->getAttachedGame();

    IosRect drect;

    IosSurface *currentSurface;

    currentSurface = _attachedTheme->getFloboSurfaceForValence(_attachedView->getValenceForFlobo(this), _currentCompressedState);
    if (currentSurface != NULL) {
        drect.x = X;
        drect.y = Y;
        drect.w = currentSurface->w;
        drect.h = currentSurface->h;

        dt->draw(currentSurface, NULL, &drect);

        /* Main flobo show */
        /* TODO: Investigate why, during network game, the falling flobo starts by being neutral */
        if ((this == attachedGame->getFallingFlobo().get())
            && (getFloboState() != FLOBO_NEUTRAL)
            && (_currentCompressedState == 0))
            dt->draw(_attachedTheme->getCircleSurfaceForIndex((_smallTicksCount >> 2) & 0x1F), NULL, &drect);

        /* Eye management */
        if ((getFloboState() != FLOBO_NEUTRAL) && (_displayEyes)) {
            int eyePhase = fmod((_floboEyeState + ios_fc::getTimeMs()), 8192.);
            IosSurface *s = NULL;
            if (eyePhase < 100)
                s = _attachedTheme->getEyeSurfaceForIndex(1, _currentCompressedState);
            else if (eyePhase < 200)
                s = _attachedTheme->getEyeSurfaceForIndex(2, _currentCompressedState);
            else if (eyePhase < 300)
                s = _attachedTheme->getEyeSurfaceForIndex(1, _currentCompressedState);
            else
                s = _attachedTheme->getEyeSurfaceForIndex(0, _currentCompressedState);
            if (s != NULL) {
                drect.x += _attachedTheme->getEyeSurfaceOffsetX();
                drect.y += _attachedTheme->getEyeSurfaceOffsetY();
                drect.w = s->w;
                drect.h = s->h;
                dt->draw(s, NULL, &drect);
            }
        }
    }
}

void AnimatedFlobo::renderShadow(DrawTarget *dt)
{
    if (!_visibilityFlag)
        return;
    if (!isRenderingAnimation()) {
        renderShadowAt(getScreenCoordinateX(), getScreenCoordinateY(), dt);
    }
}

void AnimatedFlobo::renderShadowAt(int X, int Y, DrawTarget *dt)
{
    if (getFloboState() != FLOBO_NEUTRAL) {
        IosSurface *currentSurface;
        currentSurface = _attachedTheme->getShadowSurface(_currentCompressedState);
        if (currentSurface != NULL) {
            IosRect drect;
            drect.x = X + (TSIZE >> 2) - (currentSurface->w >> 2) + 3;
            drect.y = Y + (TSIZE >> 2) - (currentSurface->h >> 2) + 3;

            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
        }
    }
}

int AnimatedFlobo::getScreenCoordinateX() const
{
    if ((_angle == 0.) || (_partner == NULL))
        return _attachedView->getScreenCoordinateX(getFloboX()) + _offsetX;
    if (_partner->getFloboX() == getFloboX()) {
        if (_partner->getFloboY() < getFloboY())
            return (_partner->getScreenCoordinateX()) - sin(_angle) * TSIZE;
        else
            return (_partner->getScreenCoordinateX()) + sin(_angle) * TSIZE;
    }
    else if (_partner->getFloboX() < getFloboX())
        return (_partner->getScreenCoordinateX()) + cos(_angle) * TSIZE;
    else
        return (_partner->getScreenCoordinateX()) - cos(_angle) * TSIZE;
}

int AnimatedFlobo::getScreenCoordinateY() const
{
    if ((_angle == 0.) || (_partner == NULL)) {
        if (getFloboState() < FLOBO_EMPTY)
            if (_attachedView->getAttachedGame()->getSemiMove())
                return (_attachedView->getScreenCoordinateY(getFloboY()) -  TSIZE / 2) + _offsetY;
        return _attachedView->getScreenCoordinateY(getFloboY()) + _offsetY;
    }
    if (_partner->getFloboY() == getFloboY()) {
        if (_partner->getFloboX() < getFloboX())
            return (_partner->getScreenCoordinateY()) + sin(_angle) * TSIZE;
        else
            return (_partner->getScreenCoordinateY()) - sin(_angle) * TSIZE;
    }
    else if (_partner->getFloboY() < getFloboY())
        return (_partner->getScreenCoordinateY()) + cos(_angle) * TSIZE;
    else
        return (_partner->getScreenCoordinateY()) - cos(_angle) * TSIZE;
}

AnimatedFloboFactory::AnimatedFloboFactory(GameView *attachedView)
  : _attachedView(attachedView), _showEyes(true)
{
    this->_attachedThemeSet = attachedView->getFloboSetTheme();
}

AnimatedFloboFactory::~AnimatedFloboFactory()
{
    while (_floboWalhalla.size() > 0) {
        Flobo *currentFlobo = _floboWalhalla[0];
        _floboWalhalla.removeAt(0);
        delete currentFlobo;
    }
}

std::shared_ptr<Flobo> AnimatedFloboFactory::createFlobo(FloboState state)
{
    AnimatedFlobo *result = new AnimatedFlobo(state, _attachedThemeSet, _attachedView);
    result->setShowEyes(_showEyes);
    return std::shared_ptr<Flobo>(result, [this](Flobo *target) { deleteFlobo(target); });
}

void AnimatedFloboFactory::deleteFlobo(Flobo *target)
{
    _floboWalhalla.add(target);
}


void AnimatedFloboFactory::renderWalhalla(DrawTarget *dt)
{
    for (int i = _floboWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(_floboWalhalla[i]);
        currentFlobo->render(dt);
    }
}

void AnimatedFloboFactory::cycleWalhalla()
{
    for (int i = _floboWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(_floboWalhalla[i]);
        if (currentFlobo->getCurrentAnimation() != NULL) {
            currentFlobo->cycleAnimation();
        } else {
            _floboWalhalla.removeAt(i);
            delete currentFlobo;
        }
    }
}

