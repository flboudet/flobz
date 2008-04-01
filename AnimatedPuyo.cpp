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
      m_currentAnimatedState(PUYO_NORMAL)
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

void AnimatedPuyo::cycleAnimation()
{
    smallTicksCount+=2;
    PuyoAnimation * animation = getCurrentAnimation();
    if ((animation != NULL)) {
        animation->cycle();
        if (animation->isFinished()) {
            removeCurrentAnimation();
        }
    }
}

bool AnimatedPuyo::isRenderingAnimation() const
{
    PuyoAnimation *animation = getCurrentAnimation();
    if (animation == NULL)
        return false;
    if (animation->isFinished())
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
    
    SDL_Rect drect;
    IIM_Surface *currentSurface;
    currentSurface = attachedTheme->getPuyoSurfaceForValence(attachedView->getValenceForPuyo(this));
    if (currentSurface != NULL) {
        drect.x = X;
        drect.y = Y;
        
        drect.w = currentSurface->w;
        drect.h = currentSurface->h;
        if (m_currentAnimatedState == PUYO_NORMAL)
            painter.requestDraw(currentSurface, &drect);
        else if (m_currentAnimatedState == PUYO_CRUNSHED) {
            SDL_Rect srcRect;
            int shrinkSurface = 12;//currentSurface->h / 3;
            drect.y += (currentSurface->h - 2*shrinkSurface)/2;
            srcRect.x = 0; srcRect.y = 0; srcRect.w = currentSurface->w; srcRect.h = shrinkSurface;
            drect.h = shrinkSurface;
            painter.requestDraw(currentSurface, &srcRect, &drect);
            drect.y += shrinkSurface;
            srcRect.y = currentSurface->h - shrinkSurface;
            painter.requestDraw(currentSurface, &srcRect, &drect);
            drect.y = Y;
            drect.h = currentSurface->h;
        }
        
        /* Main puyo show */
        /* TODO: Investigate why, during network game, the falling puyo starts by being neutral */
        if ((this == attachedGame->getFallingPuyo()) && (getPuyoState() != PUYO_NEUTRAL) && (m_currentAnimatedState != PUYO_CRUNSHED))
            painter.requestDraw(attachedTheme->getCircleSurfaceForIndex((smallTicksCount >> 2) & 0x1F), &drect);
        
        /* Eye management */
        if (getPuyoState() != PUYO_NEUTRAL) {
            int eyePhase = (puyoEyeState + SDL_GetTicks()) % 8192;
            if (eyePhase < 100)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(1), &drect);
            else if (eyePhase < 200)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(2), &drect);
            else if (eyePhase < 300)
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(1), &drect);
            else
                painter.requestDraw(attachedTheme->getEyeSurfaceForIndex(0), &drect);
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
        IIM_Surface *currentSurface;
        currentSurface = attachedTheme->getShadowSurface();
        if (currentSurface != NULL) {
            SDL_Rect drect;
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
    return attachedView->getScreenCoordinateX(getPuyoX());
}

int AnimatedPuyo::getScreenCoordinateY() const
{
    if (getPuyoState() < PUYO_EMPTY)
        if (attachedView->getAttachedGame()->getSemiMove())
            return attachedView->getScreenCoordinateY(getPuyoY()) -  TSIZE / 2;
    return attachedView->getScreenCoordinateY(getPuyoY());
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

