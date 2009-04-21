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

#include "AnimatedPuyo.h"
#include "PuyoView.h"
#include "AnimatedPuyoTheme.h"

AnimatedPuyo::AnimatedPuyo(PuyoState state, AnimatedPuyoSetTheme *themeSet, PuyoView *attachedView)
    : PuyoPuyo(state), smallTicksCount(0), attachedTheme(themeSet->getAnimatedPuyoTheme(state)),
      m_currentCompressedState(0), m_partner(NULL), m_offsetX(0), m_offsetY(0), m_angle(0)
{
    puyoEyeState = random() % 8192;
    visibilityFlag = true;
    this->attachedView = attachedView;
}

AnimatedPuyo::~AnimatedPuyo()
{
    while (animationQueue.size() > 0)
           removeCurrentAnimation();
}

void AnimatedPuyo::addAnimation(PuyoAnimation *animation)
{
    animationQueue.add(animation);
}

PuyoAnimation * AnimatedPuyo::getCurrentAnimation() const
{
    if (animationQueue.size() == 0)
        return NULL;
    return animationQueue[0];
}

void AnimatedPuyo::removeCurrentAnimation()
{
    if (animationQueue.size() == 0)
        return;
    PuyoAnimation *animationToRemove = animationQueue[0];
    animationQueue.removeKeepOrder(animationToRemove);
    delete animationToRemove;
}

void AnimatedPuyo::flushAnimations()
{
    while (getCurrentAnimation() != NULL)
        removeCurrentAnimation();
    m_angle = 0.;
    m_offsetX = 0;
    m_offsetY = 0;
}

void AnimatedPuyo::flushAnimations(int animationTag)
{
    for (int i = animationQueue.size() - 1 ; i >= 0 ; i--) {
        PuyoAnimation *anim = animationQueue[i];
        if (anim->getTag() == animationTag)
            animationQueue.removeKeepOrder(anim);
    }
}

void AnimatedPuyo::cycleAnimation()
{
    bool exclusive = false;
    smallTicksCount+=2;
    for (int i = 0 ; (i < animationQueue.size()) && (!exclusive) ; i++) {
        PuyoAnimation *animation = animationQueue[i];
        exclusive = animation->getExclusive();
        if ((!exclusive) || (i == 0)) {
            animation->cycle();
            if (animation->isFinished()) {
                animationQueue.removeKeepOrder(animation);
                i--;
                exclusive = false;
            }
        }
    }
}

bool AnimatedPuyo::isRenderingAnimation() const
{
    PuyoAnimation *animation = getCurrentAnimation();
    if (animation == NULL)
        return false;
    return animation->isEnabled();
}

void AnimatedPuyo::render()
{
    PuyoGame *attachedGame = attachedView->getAttachedGame();
    PuyoAnimation *animation = getCurrentAnimation();
    if (!isRenderingAnimation()) {
        renderAt(getScreenCoordinateX(), getScreenCoordinateY());
    }
    else {
        if (!animation->isFinished()) {
            animation->draw(attachedGame->getSemiMove());
        }
    }
}

void AnimatedPuyo::renderAt(int X, int Y)
{
    SDL_Painter &painter = attachedView->getPainter();

    if (attachedView == NULL)
        return;
    if (!visibilityFlag)
        return;
    PuyoGame *attachedGame = attachedView->getAttachedGame();

    IosRect drect;

    IosSurface *currentSurface;

    currentSurface = attachedTheme->getPuyoSurfaceForValence(attachedView->getValenceForPuyo(this), m_currentCompressedState);
    if (currentSurface != NULL) {
        drect.x = X;
        drect.y = Y;
        drect.w = currentSurface->w;
        drect.h = currentSurface->h;

        painter.requestDraw(currentSurface, &drect);

        /* Main puyo show */
        /* TODO: Investigate why, during network game, the falling puyo starts by being neutral */
        if ((this == attachedGame->getFallingPuyo()) && (getPuyoState() != PUYO_NEUTRAL))
            painter.requestDraw(attachedTheme->getCircleSurfaceForIndex((smallTicksCount >> 2) & 0x1F, m_currentCompressedState), &drect);

        /* Eye management */
        if (getPuyoState() != PUYO_NEUTRAL) {
            int eyePhase = (puyoEyeState + SDL_GetTicks()) % 8192;
            if (eyePhase < 100)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(1, m_currentCompressedState), &drect);
            else if (eyePhase < 200)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(2, m_currentCompressedState), &drect);
            else if (eyePhase < 300)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(1, m_currentCompressedState), &drect);
            else
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(0, m_currentCompressedState), &drect);
        }
    }
}

void AnimatedPuyo::renderShadow()
{
    if (!visibilityFlag)
        return;
    if (!isRenderingAnimation()) {
        renderShadowAt(getScreenCoordinateX(), getScreenCoordinateY());
    }
}

void AnimatedPuyo::renderShadowAt(int X, int Y)
{
    if (getPuyoState() != PUYO_NEUTRAL) {
        IosSurface *currentSurface;
        currentSurface = attachedTheme->getShadowSurface(m_currentCompressedState);
        if (currentSurface != NULL) {
            IosRect drect;
            SDL_Painter &painter = attachedView->getPainter();
            drect.x = X;
            drect.y = Y;

            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            painter.requestDraw(currentSurface, &drect);
        }
    }
}

int AnimatedPuyo::getScreenCoordinateX() const
{
    if ((m_angle == 0.) || (m_partner == NULL))
        return attachedView->getScreenCoordinateX(getPuyoX()) + m_offsetX;
    if (m_partner->getPuyoX() == getPuyoX()) {
        if (m_partner->getPuyoY() < getPuyoY())
            return (m_partner->getScreenCoordinateX()) - sin(m_angle) * TSIZE;
        else
            return (m_partner->getScreenCoordinateX()) + sin(m_angle) * TSIZE;
    }
    else if (m_partner->getPuyoX() < getPuyoX())
        return (m_partner->getScreenCoordinateX()) + cos(m_angle) * TSIZE;
    else
        return (m_partner->getScreenCoordinateX()) - cos(m_angle) * TSIZE;
}

int AnimatedPuyo::getScreenCoordinateY() const
{
    if ((m_angle == 0.) || (m_partner == NULL)) {
        if (getPuyoState() < PUYO_EMPTY)
            if (attachedView->getAttachedGame()->getSemiMove())
                return (attachedView->getScreenCoordinateY(getPuyoY()) -  TSIZE / 2) + m_offsetY;
        return attachedView->getScreenCoordinateY(getPuyoY()) + m_offsetY;
    }
    if (m_partner->getPuyoY() == getPuyoY()) {
        if (m_partner->getPuyoX() < getPuyoX())
            return (m_partner->getScreenCoordinateY()) + sin(m_angle) * TSIZE;
        else
            return (m_partner->getScreenCoordinateY()) - sin(m_angle) * TSIZE;
    }
    else if (m_partner->getPuyoY() < getPuyoY())
        return (m_partner->getScreenCoordinateY()) + cos(m_angle) * TSIZE;
    else
        return (m_partner->getScreenCoordinateY()) - cos(m_angle) * TSIZE;
}

AnimatedPuyoFactory::AnimatedPuyoFactory(PuyoView *attachedView)
  : attachedView(attachedView)
{
    this->attachedThemeSet = attachedView->getPuyoThemeSet();
}

AnimatedPuyoFactory::~AnimatedPuyoFactory()
{
    while (puyoWalhalla.size() > 0) {
        PuyoPuyo *currentPuyo = puyoWalhalla[0];
        puyoWalhalla.removeAt(0);
        delete currentPuyo;
    }
}

PuyoPuyo *AnimatedPuyoFactory::createPuyo(PuyoState state)
{
    return new AnimatedPuyo(state, attachedThemeSet, attachedView);
}

void AnimatedPuyoFactory::deletePuyo(PuyoPuyo *target)
{
    puyoWalhalla.add(target);
}


void AnimatedPuyoFactory::renderWalhalla()
{
    for (int i = puyoWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(puyoWalhalla[i]);
        currentPuyo->render();
    }
}

void AnimatedPuyoFactory::cycleWalhalla()
{
    for (int i = puyoWalhalla.size() - 1 ; i >= 0 ; i--) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(puyoWalhalla[i]);
        if (currentPuyo->getCurrentAnimation() != NULL) {
            currentPuyo->cycleAnimation();
        } else {
            puyoWalhalla.removeAt(i);
            delete currentPuyo;
        }
    }
}

