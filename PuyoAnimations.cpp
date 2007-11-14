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

#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "PuyoView.h"
#include "IosImgProcess.h"
#include "SDL_Painter.h"
#include "audio.h"

/* Base class implementation */
Animation::Animation()
{
    finishedFlag = false;
    enabled = true;
}

bool Animation::isFinished() const
{
    return finishedFlag;
}

bool Animation::isEnabled() const
{
    return enabled;
}


/* Neutral falling animation */
NeutralAnimation::NeutralAnimation(AnimatedPuyo &puyo, int delay, AnimationSynchronizer *synchronizer) : PuyoAnimation(puyo)
{
    this->X = attachedPuyo.getScreenCoordinateX();
    this->Y = attachedPuyo.getScreenCoordinateY();
    this->currentY = attachedPuyo.getAttachedView()->getScreenCoordinateY(0);
    step = 0;
    this->delay = delay;
    attachedPuyo.getAttachedView()->disallowCycle();
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
}

NeutralAnimation::~NeutralAnimation()
{
    synchronizer->decrementUsage();
}

static const char *sound_bim[2] =        { "bim1.wav", "bim2.wav" };
static const float sound_bim_volume[2] = { .6, .6 };

void NeutralAnimation::cycle()
{
    if (delay >=0) {
        delay--;
    }
    else {
        currentY += (int)step;
        step += 0.5;
        if (currentY >= Y) {
            int choosenSound = random() % 2;
            AudioManager::playSound(sound_bim[choosenSound], sound_bim_volume[choosenSound]);
            finishedFlag = true;
            attachedPuyo.getAttachedView()->allowCycle();
            synchronizer->pop();
        }
    }
}

void NeutralAnimation::draw(int semiMove)
{
    attachedPuyo.renderAt(X, currentY);
}

/* Animation synchronization helper */
AnimationSynchronizer::AnimationSynchronizer()
{
    currentCounter = 0;
    currentUsage = 0;
}

void AnimationSynchronizer::push()
{
    currentCounter++;
}

void AnimationSynchronizer::pop()
{
    currentCounter--;
}

bool AnimationSynchronizer::isSynchronized()
{
    return (currentCounter <= 0);
}

void AnimationSynchronizer::incrementUsage()
{
    currentUsage++;
}

void AnimationSynchronizer::decrementUsage()
{
    currentUsage--;
    if (currentUsage == 0)
        delete this;
}

/* Companion turning around main puyo animation */
TurningAnimation::TurningAnimation(AnimatedPuyo &companionPuyo, int vector,
                                   bool counterclockwise) : PuyoAnimation(companionPuyo)
{
    this->counterclockwise = counterclockwise;
    companionVector = vector;
    cpt = 0;
    angle = 0;
    step = (3.14 / 2) / 4;
}

void TurningAnimation::cycle()
{
    static const char * sound_fff = "fff.wav";
    static const float  sound_fff_volume = .35;
    
    if (cpt == 0) {
        AudioManager::playSound(sound_fff, sound_fff_volume);
        EventFX("turning",
                attachedPuyo.getScreenCoordinateX() + TSIZE/2,
                attachedPuyo.getScreenCoordinateY() + TSIZE/2);
    }
    cpt++;
    angle += step;
    if (cpt == 4)
        finishedFlag = true;
}

void TurningAnimation::draw(int semiMove)
{
    int X = attachedPuyo.getScreenCoordinateX();
    int Y = attachedPuyo.getScreenCoordinateY();
    float offsetA = sin(angle) * TSIZE;
    float offsetB = cos(angle) * TSIZE * (counterclockwise ? -1 : 1);
    int ax, ay;

    switch (companionVector) {
        case 0:
            ax = (short)(X - offsetB);
            ay = (short)(Y + offsetA - TSIZE);
            break;
        case 1:
            ax = (short)(X - offsetA + TSIZE);
            ay = (short)(Y - offsetB);
            break;
        case 2:
            ax = (short)(X + offsetB);
            ay = (short)(Y - offsetA + TSIZE);
            break;
        case 3:
            ax = (short)(X + offsetA - TSIZE);
            ay = (short)(Y + offsetB);
            break;
            
        case -3:
            ax = (short)(X + offsetB);
            ay = (short)(Y + offsetA - TSIZE);
            break;
    }
    attachedPuyo.renderAt(ax, ay);
}

/* Puyo falling and bouncing animation */

const int FallingAnimation::BOUNCING_OFFSET_NUM = 12;
const int FallingAnimation::BOUNCING_OFFSET[] = { -1, -3, -5, -4, -2, 0, -6, -9, -11, -9, -6, 0 };

FallingAnimation::FallingAnimation(AnimatedPuyo &puyo, int originY, int xOffset, int yOffset, int step) : PuyoAnimation(puyo)
{
    this->xOffset = xOffset;
    this->yOffset = yOffset;
    this->step    = 0/*step*/;
    this->X  = (attachedPuyo.getPuyoX()*TSIZE) + xOffset;
    this->Y  = (originY*TSIZE) + yOffset;
    bouncing = BOUNCING_OFFSET_NUM - 1;
    if (originY == attachedPuyo.getPuyoY()) {
        bouncing = -1;
    }
    attachedPuyo.getAttachedView()->disallowCycle();
    EventFX("start_falling", X+TSIZE/2,Y+TSIZE/2);
}

void FallingAnimation::cycle()
{
    Y += step++;
    if (Y >= (attachedPuyo.getPuyoY()*TSIZE) + yOffset)
    {
        Y = (attachedPuyo.getPuyoY()*TSIZE) + yOffset;
        bouncing--;

        if (bouncing < 0) {
            finishedFlag = true;
            AudioManager::playSound("bam1.wav", .1);
            EventFX("bouncing", X+TSIZE/2,Y+TSIZE/2);
            EventFX("end_falling", X+TSIZE/2,Y+TSIZE/2);
            attachedPuyo.getAttachedView()->allowCycle();
        }
        else if (BOUNCING_OFFSET[bouncing] == 0) {
            AudioManager::playSound("bam1.wav", .1);

            EventFX("bouncing", X+TSIZE/2,Y+TSIZE/2);
        }
    }
}

void FallingAnimation::draw(int semiMove)
{
    SDL_Rect drect;
    drect.x = X;
    int coordY = Y + (bouncing>=0?BOUNCING_OFFSET[bouncing]:0);
    attachedPuyo.renderAt(X, coordY);

    // TODO : Reactiver le EyeSwirl !
    //if (attachedPuyo.getPuyoState() != PUYO_NEUTRAL)
    //   painter.requestDraw(puyoEyesSwirl[(bouncing/2)%4], &drect);
}

/* Puyo exploding and vanishing animation */
VanishAnimation::VanishAnimation(AnimatedPuyo &puyo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer) : PuyoAnimation(puyo)
{
    this->xOffset = xOffset;
    this->yOffset = yOffset;
    this->X = (attachedPuyo.getPuyoX()*TSIZE) + xOffset;
    this->Y = (attachedPuyo.getPuyoY()*TSIZE) + yOffset;
    this->color = attachedPuyo.getPuyoState();
    if (color > PUYO_EMPTY)
        color -= PUYO_BLUE;
    iter = 0;
    once = false;
    enabled = false;
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
    this->delay = delay;
    attachedPuyo.getAttachedView()->disallowCycle();
}

VanishAnimation::~VanishAnimation()
{
    synchronizer->decrementUsage();
}

void VanishAnimation::cycle()
{
    if (once == false) {
        once = true;
        synchronizer->pop();
    }
    else if (synchronizer->isSynchronized()) {
        enabled = true;
        iter ++;
        if (iter == 20 + delay) {
            attachedPuyo.getAttachedView()->allowCycle();
        }
        else if (iter == 50 + delay) {
            finishedFlag = true;
            attachedPuyo.setVisible(false);
        }
    }
}

void VanishAnimation::draw(int semiMove)
{
    if (iter < (10 + delay)) {
        if (iter % 2 == 0) {
            attachedPuyo.renderAt(X, Y);
        }
    }
    else {
		AnimatedPuyoTheme *theme = attachedPuyo.getAttachedTheme();
        SDL_Painter &painter = attachedPuyo.getAttachedView()->getPainter();
        
        SDL_Rect drect, xrect;
        int iter2 = iter - 10 - delay;
        int shrinkingImage = (iter - 10 - delay) / 4;
        if (shrinkingImage < 4) {
            IIM_Surface *shrinkingSurface, *explodingSurface;
            shrinkingSurface = theme->getShrinkingSurfaceForIndex(shrinkingImage);
            explodingSurface = theme->getExplodingSurfaceForIndex(shrinkingImage);
            
            drect.x = X;
            drect.y = Y;
            drect.w = shrinkingSurface->w;
            drect.h = shrinkingSurface->h;
        
            painter.requestDraw(shrinkingSurface, &drect);
            int xrectY = Y + (int)(2.5 * pow(iter - 16 - delay, 2) - 108);
            xrect.w = explodingSurface->w;
            xrect.h = explodingSurface->h;
            xrect.x = X - iter2 * iter2;
            xrect.y = xrectY;
            painter.requestDraw(explodingSurface, &xrect);
            xrect.x = X - iter2;
            xrect.y = xrectY + iter2;
            painter.requestDraw(explodingSurface, &xrect);
            xrect.x = X + iter2;
            xrect.y = xrectY + iter2;
            painter.requestDraw(explodingSurface, &xrect);
            xrect.x = X + iter2 * iter2;
            xrect.y = xrectY;
            painter.requestDraw(explodingSurface, &xrect);
        }
    }
}

VanishSoundAnimation::VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer)
{
    once = false;
    step = 0;
    this->phase = phase;
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
}

VanishSoundAnimation::~VanishSoundAnimation()
{
    synchronizer->decrementUsage();
}

static const char *sound_splash[8] = {
    "splash1.wav", "splash2.wav", "splash3.wav", "splash4.wav",
    "splash5.wav", "splash6.wav", "splash7.wav", "splash8.wav" };
static float sound_splash_volume = .6;

void VanishSoundAnimation::cycle()
{
    if (once == false) {
        once = true;
        synchronizer->pop();
    }
    else if (synchronizer->isSynchronized()) {
        step++;
        if (step == 1) {
            AudioManager::playSound(sound_splash[phase>7?7:phase], sound_splash_volume);
            finishedFlag = true;
        }
    }
}

void VanishSoundAnimation::draw(int semiMove)
{
    // do nothing
}

NeutralPopAnimation::NeutralPopAnimation(AnimatedPuyo &puyo, int delay, AnimationSynchronizer *synchronizer)
    : PuyoAnimation(puyo), synchronizer(synchronizer), iter(0), delay(delay), once(false),
      X(attachedPuyo.getScreenCoordinateX()), Y(attachedPuyo.getScreenCoordinateY())
{
    synchronizer->push();
    synchronizer->incrementUsage();
    AnimatedPuyoTheme *attachedTheme = attachedPuyo.getAttachedTheme();
    neutralPop[0] = attachedTheme->getExplodingSurfaceForIndex(0);
    neutralPop[1] = attachedTheme->getExplodingSurfaceForIndex(1);
    neutralPop[2] = attachedTheme->getExplodingSurfaceForIndex(2);
    enabled = false;
}

NeutralPopAnimation::~NeutralPopAnimation()
{
    synchronizer->decrementUsage();
}

void NeutralPopAnimation::cycle()
{
    if (once == false) {
        once = true;
        synchronizer->pop();
    }
    else if (synchronizer->isSynchronized()) {
        iter ++;
        if (iter == 17 + delay) {
            AudioManager::playSound("pop.wav", .25);
            enabled = true;
        }
        else if (iter == 30 + delay) {
            attachedPuyo.setVisible(false);
            finishedFlag = true;
        }
    }
}

void NeutralPopAnimation::draw(int semiMove)
{
    SDL_Painter &painter = attachedPuyo.getAttachedView()->getPainter();
    SDL_Rect drect, xrect;
    drect.x = X;
    drect.y = Y;
    drect.w = neutralPop[0]->w;
    drect.h = neutralPop[0]->h;
    if (iter - delay < 20) {
        painter.requestDraw(neutralPop[0], &drect);
    }
    else if (iter - delay < 23) {
        painter.requestDraw(neutralPop[1], &drect);
    }
    else if (iter - delay < 26) {
        painter.requestDraw(neutralPop[2], &drect);
    }
}

SmoothBounceAnimation::SmoothBounceAnimation(AnimatedPuyo &puyo, AnimationSynchronizer *synchronizer) : PuyoAnimation(puyo)
{
    bounceOffset = 0;
    bouncePhase = 0;
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    origX = attachedPuyo.getScreenCoordinateX();
    origY = attachedPuyo.getScreenCoordinateY();
    enabled = false;
}

SmoothBounceAnimation::~SmoothBounceAnimation()
{
    synchronizer->decrementUsage();
}

void SmoothBounceAnimation::cycle()
{
    if (synchronizer->isSynchronized()) {
        enabled = true;
        switch (bouncePhase) {
            case 0:
                bounceOffset++;
                if (bounceOffset == 10)
                    bouncePhase++;
                    break;
            case 1:
                bounceOffset--;
                if (bounceOffset == 0)
                    bouncePhase++;
                    break;
            case 2:
                finishedFlag = true;
                break;
        }
    }
}

void SmoothBounceAnimation::draw(int semiMove)
{
    attachedPuyo.renderAt(origX, origY + bounceOffset);
}

GameOverFallAnimation::GameOverFallAnimation(AnimatedPuyo &puyo, int delay)
    : PuyoAnimation(puyo), delay(delay), Y(attachedPuyo.getScreenCoordinateY()), yAccel(10)
{
}

GameOverFallAnimation::~GameOverFallAnimation()
{
}

void GameOverFallAnimation::cycle()
{
    if (delay < 0) {
        if (yAccel > -20)
            yAccel -= 1;
        Y -= yAccel;
        if (Y > 480) {
            attachedPuyo.setVisible(false);
            finishedFlag = true;
        }
    }
    else delay--;
}

void GameOverFallAnimation::draw(int semiMove)
{
    attachedPuyo.renderAt(attachedPuyo.getScreenCoordinateX(), Y);
}

