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

/* not clean, but basta */
extern SDL_Painter painter;
extern IIM_Surface *puyoEyes;
extern IIM_Surface *puyoEye[3];
extern IIM_Surface *puyoEyesSwirl[4];
extern IIM_Surface *shrinkingPuyo[5][5];
extern IIM_Surface *explodingPuyo[5][5];

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
IIM_Surface *NeutralAnimation::neutral = NULL;
NeutralAnimation::NeutralAnimation(AnimatedPuyo &puyo, int delay) : PuyoAnimation(puyo)
{
    if (neutral == NULL)
        neutral = PuyoView::getSurfaceForState(PUYO_NEUTRAL);
	this->X = attachedPuyo.getScreenCoordinateX();
	this->Y = attachedPuyo.getScreenCoordinateY();
	this->currentY = attachedPuyo.getAttachedView()->getScreenCoordinateY(0);
    step = 0;
    this->delay = delay;
    attachedPuyo.getAttachedView()->disallowCycle();
}

void NeutralAnimation::cycle()
{
    if (delay >=0) {
        delay--;
    }
    else {
        currentY += (int)step;
        step += 0.5;
        if (currentY >= Y) {
            audio_sound_play(sound_bim[random() % 2]);
            finishedFlag = true;
            attachedPuyo.getAttachedView()->allowCycle();
        }
    }
}

void NeutralAnimation::draw(int semiMove)
{
	SDL_Rect drect;
	drect.x = X;
	drect.y = currentY;
	drect.w = neutral->w;
	drect.h = neutral->h;
	painter.requestDraw(neutral, &drect);
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
    targetSurface = attachedPuyo.getAttachedView()->getSurfaceForPuyo(&attachedPuyo);
    cpt = 0;
    angle = 0;
    step = (3.14 / 2) / 4;
}

void TurningAnimation::cycle()
{
    if (cpt == 0) {
        audio_sound_play(sound_fff);
    }
    cpt++;
    angle += step;
    if (cpt == 4)
        finishedFlag = true;
}

void TurningAnimation::draw(int semiMove)
{
    if (targetSurface == NULL)
        return;
    X = attachedPuyo.getScreenCoordinateX();
    Y = attachedPuyo.getScreenCoordinateY();
    
    float offsetA = sin(angle) * TSIZE;
    float offsetB = cos(angle) * TSIZE * (counterclockwise ? -1 : 1);
    SDL_Rect drect, drect2;
    drect.w = targetSurface->w;
    drect.h = targetSurface->h;
    drect.y = -semiMove * TSIZE / 2;
    switch (companionVector) {
        case 0:
            drect.x = (short)(X - offsetB);
            drect.y += (short)(Y + offsetA - TSIZE);
            break;
        case 1:
            drect.x = (short)(X - offsetA + TSIZE);
            drect.y += (short)(Y - offsetB);
            break;
        case 2:
            drect.x = (short)(X + offsetB);
            drect.y += (short)(Y - offsetA + TSIZE);
            break;
        case 3:
            drect.x = (short)(X + offsetA - TSIZE);
            drect.y += (short)(Y + offsetB);
            break;
            
        case -3:
            drect.x = (short)(X + offsetB);
            drect.y += (short)(Y + offsetA - TSIZE);
            break;
    }
    drect2 = drect;
    painter.requestDraw(targetSurface, &drect);
    painter.requestDraw(puyoEyes, &drect2);
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
    puyoFace = PuyoView::getSurfaceForState(attachedPuyo.getPuyoState());
    bouncing = BOUNCING_OFFSET_NUM - 1;
    if (originY == attachedPuyo.getPuyoY()) {
        bouncing = -1;
    }
    attachedPuyo.getAttachedView()->disallowCycle();
}

void FallingAnimation::cycle()
{
    Y += step++;
    if (Y >= (attachedPuyo.getPuyoY()*TSIZE) + yOffset)
    {
        bouncing--;
        if (bouncing < 0) {
            finishedFlag = true;
            audio_sound_play(sound_bam1);
            attachedPuyo.getAttachedView()->allowCycle();
        }
        else {
            if (BOUNCING_OFFSET[bouncing] == 0)
                audio_sound_play(sound_bam1);
        }
        Y = (attachedPuyo.getPuyoY()*TSIZE) + yOffset;
    }
}

void FallingAnimation::draw(int semiMove)
{
    if (puyoFace) {
        SDL_Rect drect;
        drect.x = X;
        drect.y = Y + (bouncing>=0?BOUNCING_OFFSET[bouncing]:0);
        // drect.y = -semiMove() * TSIZE / 2;
        drect.w = puyoFace->w;
        drect.h = puyoFace->h;
        painter.requestDraw(puyoFace, &drect);
        if (attachedPuyo.getPuyoState() != PUYO_NEUTRAL)
            painter.requestDraw(puyoEyesSwirl[(bouncing/2)%4], &drect);
    }
}

/* Puyo exploding and vanishing animation */
VanishAnimation::VanishAnimation(AnimatedPuyo &puyo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer) : PuyoAnimation(puyo)
{
    puyoFace = PuyoView::getSurfaceForState(attachedPuyo.getPuyoState());
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
        if (puyoFace && (iter % 2 == 0)) {
            SDL_Rect drect;
            drect.x = X;
            drect.y = Y;
            drect.w = puyoFace->w;
            drect.h = puyoFace->h;
            painter.requestDraw(puyoFace, &drect);
            if (color != PUYO_NEUTRAL)
                painter.requestDraw(puyoEyes, &drect);
        }
    }
    else {
        if (puyoFace) {
            SDL_Rect drect, xrect;
            drect.x = X;
            drect.y = Y;// + (2.5 * pow(iter - 16, 2) - 108);
                drect.w = puyoFace->w;
                drect.h = puyoFace->h;
                int iter2 = iter - 10 - delay;
                int shrinkingImage = (iter - 10 - delay) / 4;
                if (shrinkingImage < 4) {
                    painter.requestDraw(shrinkingPuyo[shrinkingImage][color], &drect);
                    int xrectY = Y + (int)(2.5 * pow(iter - 16 - delay, 2) - 108);
                    xrect.w = explodingPuyo[shrinkingImage][color]->w;
                    xrect.h = explodingPuyo[shrinkingImage][color]->h;
                    xrect.x = X - iter2 * iter2;
                    xrect.y = xrectY;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X - iter2;
                    xrect.y = xrectY + iter2;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X + iter2;
                    xrect.y = xrectY + iter2;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                    xrect.x = X + iter2 * iter2;
                    xrect.y = xrectY;
                    painter.requestDraw(explodingPuyo[shrinkingImage][color], &xrect);
                }
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

void VanishSoundAnimation::cycle()
{
    if (once == false) {
        once = true;
        synchronizer->pop();
    }
    else if (synchronizer->isSynchronized()) {
        step++;
        if (step == 1) {
            audio_sound_play(sound_splash[phase>7?7:phase]);
            finishedFlag = true;
        }
    }
}

void VanishSoundAnimation::draw(int semiMove)
{
    // do nothing
}
