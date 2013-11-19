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
    : Flobo(state), smallTicksCount(0), attachedTheme(themeSet != NULL ? &(themeSet->getFloboTheme(state)) : NULL),
      m_currentCompressedState(0), m_partner(NULL), m_offsetX(0), m_offsetY(0), m_angle(0), m_displayEyes(true)
{
    floboEyeState = random() % 8192;
    visibilityFlag = true;
    this->attachedView = attachedView;
}

AnimatedFlobo::~AnimatedFlobo()
{
    while (animationQueue.size() > 0)
           removeCurrentAnimation();
}

void AnimatedFlobo::addAnimation(FloboAnimation *animation)
{
    animationQueue.add(animation);
}

FloboAnimation * AnimatedFlobo::getCurrentAnimation() const
{
    if (animationQueue.size() == 0)
        return NULL;
    return animationQueue[0];
}

void AnimatedFlobo::removeCurrentAnimation()
{
    if (animationQueue.size() == 0)
        return;
    FloboAnimation *animationToRemove = animationQueue[0];
    animationQueue.removeKeepOrder(animationToRemove);
    delete animationToRemove;
}

void AnimatedFlobo::flushAnimations()
{
    while (getCurrentAnimation() != NULL)
        removeCurrentAnimation();
    m_angle = 0.;
    m_offsetX = 0;
    m_offsetY = 0;
}

void AnimatedFlobo::flushAnimations(int animationTag)
{
    for (int i = animationQueue.size() - 1 ; i >= 0 ; i--) {
        FloboAnimation *anim = animationQueue[i];
        if (anim->getTag() == animationTag) {
            animationQueue.removeKeepOrder(anim);
            delete anim;
        }
    }
}

void AnimatedFlobo::cycleAnimation()
{
    bool exclusive = false;
    smallTicksCount+=2;
    for (int i = 0 ; (i < animationQueue.size()) && (!exclusive) ; i++) {
        FloboAnimation *animation = animationQueue[i];
        exclusive = animation->getExclusive();
        if ((!exclusive) || (i == 0)) {
            animation->cycle();
            if (animation->isFinished()) {
                animationQueue.removeKeepOrder(animation);
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
    FloboGame *attachedGame = attachedView->getAttachedGame();
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
    if (attachedView == NULL)
        return;
    if (!visibilityFlag)
        return;
    FloboGame *attachedGame = attachedView->getAttachedGame();

    IosRect drect;

    IosSurface *currentSurface;

    currentSurface = attachedTheme->getFloboSurfaceForValence(attachedView->getValenceForFlobo(this), m_currentCompressedState);
    if (currentSurface != NULL) {
        drect.x = X;
        drect.y = Y;
        drect.w = currentSurface->w;
        drect.h = currentSurface->h;

        dt->draw(currentSurface, NULL, &drect);

        /* Main flobo show */
        /* TODO: Investigate why, during network game, the falling flobo starts by being neutral */
        if ((this == attachedGame->getFallingFlobo())
            && (getFloboState() != FLOBO_NEUTRAL)
            && (m_currentCompressedState == 0))
            dt->draw(attachedTheme->getCircleSurfaceForIndex((smallTicksCount >> 2) & 0x1F), NULL, &drect);

        /* Eye management */
        if ((getFloboState() != FLOBO_NEUTRAL) && (m_displayEyes)) {
            int eyePhase = fmod((floboEyeState + ios_fc::getTimeMs()), 8192.);
            IosSurface *s = NULL;
            if (eyePhase < 100)
                s = attachedTheme->getEyeSurfaceForIndex(1, m_currentCompressedState);
            else if (eyePhase < 200)
                s = attachedTheme->getEyeSurfaceForIndex(2, m_currentCompressedState);
            else if (eyePhase < 300)
                s = attachedTheme->getEyeSurfaceForIndex(1, m_currentCompressedState);
            else
                s = attachedTheme->getEyeSurfaceForIndex(0, m_currentCompressedState);
            if (s != NULL) {
                drect.x += attachedTheme->getEyeSurfaceOffsetX();
                drect.y += attachedTheme->getEyeSurfaceOffsetY();
                drect.w = s->w;
                drect.h = s->h;
                dt->draw(s, NULL, &drect);
            }
        }
    }
}

void AnimatedFlobo::renderShadow(DrawTarget *dt)
{
    if (!visibilityFlag)
        return;
    if (!isRenderingAnimation()) {
        renderShadowAt(getScreenCoordinateX(), getScreenCoordinateY(), dt);
    }
}

void AnimatedFlobo::renderShadowAt(int X, int Y, DrawTarget *dt)
{
    if (getFloboState() != FLOBO_NEUTRAL) {
        IosSurface *currentSurface;
        currentSurface = attachedTheme->getShadowSurface(m_currentCompressedState);
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
    if ((m_angle == 0.) || (m_partner == NULL))
        return attachedView->getScreenCoordinateX(getFloboX()) + m_offsetX;
    if (m_partner->getFloboX() == getFloboX()) {
        if (m_partner->getFloboY() < getFloboY())
            return (m_partner->getScreenCoordinateX()) - sin(m_angle) * TSIZE;
        else
            return (m_partner->getScreenCoordinateX()) + sin(m_angle) * TSIZE;
    }
    else if (m_partner->getFloboX() < getFloboX())
        return (m_partner->getScreenCoordinateX()) + cos(m_angle) * TSIZE;
    else
        return (m_partner->getScreenCoordinateX()) - cos(m_angle) * TSIZE;
}

int AnimatedFlobo::getScreenCoordinateY() const
{
    if ((m_angle == 0.) || (m_partner == NULL)) {
        if (getFloboState() < FLOBO_EMPTY)
            if (attachedView->getAttachedGame()->getSemiMove())
                return (attachedView->getScreenCoordinateY(getFloboY()) -  TSIZE / 2) + m_offsetY;
        return attachedView->getScreenCoordinateY(getFloboY()) + m_offsetY;
    }
    if (m_partner->getFloboY() == getFloboY()) {
        if (m_partner->getFloboX() < getFloboX())
            return (m_partner->getScreenCoordinateY()) + sin(m_angle) * TSIZE;
        else
            return (m_partner->getScreenCoordinateY()) - sin(m_angle) * TSIZE;
    }
    else if (m_partner->getFloboY() < getFloboY())
        return (m_partner->getScreenCoordinateY()) + cos(m_angle) * TSIZE;
    else
        return (m_partner->getScreenCoordinateY()) - cos(m_angle) * TSIZE;
}

AnimatedFloboFactory::AnimatedFloboFactory(GameView *attachedView)
  : attachedView(attachedView), m_showEyes(true)
{
    this->attachedThemeSet = attachedView->getFloboSetTheme();
}

AnimatedFloboFactory::~AnimatedFloboFactory()
{
    while (floboWalhalla.size() > 0) {
        Flobo *currentFlobo = floboWalhalla[0];
        floboWalhalla.removeAt(0);
        delete currentFlobo;
    }
}

Flobo *AnimatedFloboFactory::createFlobo(FloboState state)
{
    AnimatedFlobo *result = new AnimatedFlobo(state, attachedThemeSet, attachedView);
    result->setShowEyes(m_showEyes);
    return result;
}

void AnimatedFloboFactory::deleteFlobo(Flobo *target)
{
    floboWalhalla.add(target);
}


void AnimatedFloboFactory::renderWalhalla(DrawTarget *dt)
{
    for (int i = floboWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(floboWalhalla[i]);
        currentFlobo->render(dt);
    }
}

void AnimatedFloboFactory::cycleWalhalla()
{
    for (int i = floboWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(floboWalhalla[i]);
        if (currentFlobo->getCurrentAnimation() != NULL) {
            currentFlobo->cycleAnimation();
        } else {
            floboWalhalla.removeAt(i);
            delete currentFlobo;
        }
    }
}

