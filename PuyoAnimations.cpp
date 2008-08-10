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
  : finishedFlag(false), enabled(true),
    m_exclusive(true), m_tag(ANIMATION_NO_TAG)
{
}

bool Animation::isFinished() const
{
    return finishedFlag;
}

bool Animation::isEnabled() const
{
    return enabled;
}

float PuyoAnimation::getPuyoSoundPadding() const
{
    return ((float)attachedPuyo.getScreenCoordinateX() / 640.)*2. - 1.;
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
            // TODO jeko
            /*EventFX("neutral_bouncing", 
                attachedPuyo.getScreenCoordinateX() + TSIZE/2,
                attachedPuyo.getScreenCoordinateY() + TSIZE/2,
                attachedPuyo.getAttachedView()->getPlayerId());*/
            AudioManager::playSound(sound_bim[choosenSound], sound_bim_volume[choosenSound], getPuyoSoundPadding());
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
TurningAnimation::TurningAnimation(AnimatedPuyo &companionPuyo,
                                   bool counterclockwise) : PuyoAnimation(companionPuyo)
{
    enabled = false;
    m_exclusive = false;
    m_tag = ANIMATION_ROTATE;
    cpt = 0;
    angle = 3.14 / 2;
    step = (3.14 / 2) / 4 * (counterclockwise ? 1 : -1);
    cycle();
}

void TurningAnimation::cycle()
{
    static const char * sound_fff = "fff.wav";
    static const float  sound_fff_volume = .35;
    
    if (cpt == 0) {
        AudioManager::playSound(sound_fff, sound_fff_volume, getPuyoSoundPadding());
        EventFX("turning",
                attachedPuyo.getScreenCoordinateX() + TSIZE/2,
                attachedPuyo.getScreenCoordinateY() + TSIZE/2,
                attachedPuyo.getAttachedView()->getPlayerId());
    }
    cpt++;
    angle += step;
    if (cpt == 4) {
        finishedFlag = true;
        attachedPuyo.setRotation(0.);
    }
    else
        attachedPuyo.setRotation(angle);
}

/* Puyo moving from one place to another in the horizontal axis */
MovingHAnimation::MovingHAnimation(AnimatedPuyo &puyo, int hOffset, int step)
  : PuyoAnimation(puyo), m_cpt(0), m_hOffset(hOffset), m_step(step),
    m_hOffsetByStep((float)hOffset/(float)step)
{
    enabled = false;
    m_exclusive = false;
    m_tag = ANIMATION_H;
    AudioManager::playSound("tick.wav", .35, getPuyoSoundPadding());
    cycle();
}

void MovingHAnimation::cycle()
{
    m_cpt++;
    attachedPuyo.setOffsetX(m_hOffset - m_hOffsetByStep*m_cpt);
    if (m_cpt == m_step) {
        finishedFlag = true;
        attachedPuyo.setOffsetX(0);
    }
}

/* Puyo moving from one place to another in the vertical axis */
MovingVAnimation::MovingVAnimation(AnimatedPuyo &puyo, int vOffset, int step)
  : PuyoAnimation(puyo), m_cpt(0), m_vOffset(vOffset), m_step(step),
    m_vOffsetByStep((float)vOffset/(float)step)
{
    enabled = false;
    m_exclusive = false;
    m_tag = ANIMATION_V;
    cycle();
}

void MovingVAnimation::cycle()
{
    m_cpt++;
    attachedPuyo.setOffsetY(m_vOffset - m_vOffsetByStep*m_cpt);
    if (m_cpt == m_step) {
        finishedFlag = true;
        attachedPuyo.setOffsetY(0);
    }
}

/* Puyo falling and bouncing animation */

const int FallingAnimation::BOUNCING_OFFSET_NUM = 9;
const int FallingAnimation::BOUNCING_OFFSET[] = {-3, -4, -3, 0, 3, 6, 8, 6, 3};

FallingAnimation::FallingAnimation(AnimatedPuyo &puyo, int originY, int xOffset, int yOffset, int off) : PuyoAnimation(puyo), m_once(false)
{
    this->xOffset = xOffset;
    this->yOffset = yOffset;
    this->off     = off;
    this->step    = 0;
    this->X  = (attachedPuyo.getPuyoX()*TSIZE) + xOffset;
    this->Y  = (originY*TSIZE) + yOffset;
    bouncing = BOUNCING_OFFSET_NUM + off;
    attachedPuyo.getAttachedView()->disallowCycle();
    EventFX("start_falling", X+TSIZE/2, Y+TSIZE/2, puyo.getAttachedView()->getPlayerId());
}

void FallingAnimation::cycle()
{
    Y += step++;
    
    if (Y >= (attachedPuyo.getPuyoY()*TSIZE) + yOffset)
    {
        if (!m_once) {
            AudioManager::playSound("bam1.wav", .3, getPuyoSoundPadding());
            m_once = true;
        }
        Y = (attachedPuyo.getPuyoY()*TSIZE) + yOffset;
        if (bouncing == BOUNCING_OFFSET_NUM + off) attachedPuyo.getAttachedView()->allowCycle();

        bouncing--;

        if (bouncing < 0) {
            finishedFlag = true;
//<<<<<<< .mine
//            AudioManager::playSound("bam1.wav", .3);
//            EventFX("bouncing", X+TSIZE/2,Y+TSIZE/2, attachedPuyo.getAttachedView()->getPlayerId());
//            EventFX("end_falling", X+TSIZE/2,Y+TSIZE/2, attachedPuyo.getAttachedView()->getPlayerId());
//            attachedPuyo.setAnimatedState(AnimatedPuyo::PUYO_NORMAL);
//            attachedPuyo.getAttachedView()->allowCycle();
//=======
            //EventFX("bouncing", X+TSIZE/2,Y+TSIZE/2, attachedPuyo.getAttachedView()->getPlayerId());
            //EventFX("end_falling", X+TSIZE/2,Y+TSIZE/2, attachedPuyo.getAttachedView()->getPlayerId());
            attachedPuyo.setAnimatedState(0);
            //attachedPuyo.getAttachedView()->allowCycle();
//>>>>>>> .r1012
        }
        else if ((bouncing < BOUNCING_OFFSET_NUM) && (BOUNCING_OFFSET[bouncing] > 0)) {
            attachedPuyo.setAnimatedState(BOUNCING_OFFSET[bouncing]);
            //AudioManager::playSound("bam1.wav", .1);
            //EventFX("bouncing", X+TSIZE/2,Y+TSIZE/2, attachedPuyo.getAttachedView()->getPlayerId());
        }
        else
            attachedPuyo.setAnimatedState(0);
        
    }
}

void FallingAnimation::draw(int semiMove)
{
    SDL_Rect drect;
    drect.x = X;
    int coordY = Y;
    if (bouncing >=0)
    {
        for (int i = bouncing-off; i <= bouncing && i < BOUNCING_OFFSET_NUM ; i++)
            if (i>=0)
                coordY += BOUNCING_OFFSET[i];
    }
    attachedPuyo.renderShadowAt(X, coordY);
    attachedPuyo.renderAt(X, coordY);

    // TODO : Reactiver le EyeSwirl !
    //if (attachedPuyo.getPuyoState() != PUYO_NEUTRAL)
    //   painter.requestDraw(puyoEyesSwirl[(bouncing/2)%4], &drect);
}

/* Puyo exploding and vanishing animation */
VanishAnimation::VanishAnimation(AnimatedPuyo &puyo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer, int puyoNum, int groupSize, int groupNum, int phase) : PuyoAnimation(puyo), puyoNum(puyoNum), groupSize(groupSize), groupNum(groupNum), phase(phase)
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
        if (iter == 1) {
            EventFX("vanish",
                    attachedPuyo.getScreenCoordinateX() + TSIZE/2,
                    attachedPuyo.getScreenCoordinateY() + TSIZE/2,
                    attachedPuyo.getAttachedView()->getPlayerId());
        }
        if (iter == 20 + delay) {
            attachedPuyo.getAttachedView()->allowCycle();
            if ((groupNum == 0) && (puyoNum == 0))
                EventFX("vanish_phase", groupSize,phase, attachedPuyo.getAttachedView()->getPlayerId());
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
static float sound_splash_volume = 1.0;

void VanishSoundAnimation::cycle()
{
    if (once == false) {
        once = true;
        synchronizer->pop();
    }
    else if (synchronizer->isSynchronized()) {
        step++;
        if (step == 1) {
            AudioManager::playSound(sound_splash[phase-1>7?7:phase-1], sound_splash_volume);
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
            AudioManager::playSound("pop.wav", .25, getPuyoSoundPadding());
            EventFX("neutral_pop", 
                    attachedPuyo.getScreenCoordinateX() + TSIZE/2,
                    attachedPuyo.getScreenCoordinateY() + TSIZE/2,
                    attachedPuyo.getAttachedView()->getPlayerId());
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
    SDL_Rect drect;
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

SmoothBounceAnimation::SmoothBounceAnimation(AnimatedPuyo &puyo, AnimationSynchronizer *synchronizer, int depth) :
    PuyoAnimation(puyo), bounceMax(depth)
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
                if (bounceOffset == bounceMax)
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

