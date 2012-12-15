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

#include "GTLog.h"
#include "FloboAnimations.h"
#include "AnimatedFlobo.h"
#include "GameView.h"
#include "audio.h"
#include "FPCommander.h"

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

float FloboAnimation::getSoundPadding() const
{
    return ((float)attachedFlobo.getScreenCoordinateX() / 640.)*2. - 1.;
}

/* Neutral falling animation */
NeutralAnimation::NeutralAnimation(AnimatedFlobo &flobo, int delay, AnimationSynchronizer *synchronizer) : FloboAnimation(flobo)
{
    this->X = attachedFlobo.getScreenCoordinateX();
    this->Y = attachedFlobo.getScreenCoordinateY();
    this->currentY = attachedFlobo.getAttachedView()->getScreenCoordinateY(0);
    step = 0;
    this->delay = delay;
    attachedFlobo.getAttachedView()->disallowCycle();
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
        step += 0.8;
        if (currentY >= Y) {
            int choosenSound = random() % 2;
            // TODO jeko
            /*EventFX("neutral_bouncing",
                attachedFlobo.getScreenCoordinateX() + TSIZE/2,
                attachedFlobo.getScreenCoordinateY() + TSIZE/2,
                attachedFlobo.getAttachedView()->getPlayerId());*/
            theCommander->playSound(sound_bim[choosenSound], sound_bim_volume[choosenSound], getSoundPadding());
            finishedFlag = true;
            attachedFlobo.getAttachedView()->allowCycle();
            synchronizer->pop();
        }
    }
}

void NeutralAnimation::draw(int semiMove, DrawTarget *dt)
{
    attachedFlobo.renderAt(X, currentY, dt);
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

/* Companion turning around main flobo animation */
TurningAnimation::TurningAnimation(AnimatedFlobo &companionFlobo,
                                   bool counterclockwise) : FloboAnimation(companionFlobo), NUMSTEPS(6)
{
    enabled = false;
    m_exclusive = false;
    m_tag = ANIMATION_ROTATE;
    cpt = 0;
    angle = (3.14 / 2) * (counterclockwise ? -1 : 1);
    step = (3.14 / 2) / NUMSTEPS * (counterclockwise ? 1 : -1);
    cycle();
}

void TurningAnimation::cycle()
{
    static const char * sound_fff = "fff.wav";
    static const float  sound_fff_volume = .35;

    if (cpt == 0) {
        theCommander->playSound(sound_fff, sound_fff_volume, getSoundPadding());
        EventFX("turning",
                attachedFlobo.getScreenCoordinateX() + TSIZE/2,
                attachedFlobo.getScreenCoordinateY() + TSIZE/2,
                attachedFlobo.getAttachedView()->getPlayerId());
    }
    cpt++;
    angle += step;
    if (cpt == NUMSTEPS) {
        finishedFlag = true;
        attachedFlobo.setRotation(0.);
    }
    else
        attachedFlobo.setRotation(angle);
}

/* Flobo moving from one place to another in the horizontal axis */
MovingHAnimation::MovingHAnimation(AnimatedFlobo &flobo, int hOffset, int step)
  : FloboAnimation(flobo), m_cpt(0), m_hOffset(hOffset), m_step(step),
    m_hOffsetByStep((float)hOffset/(float)step)
{
    enabled = false;
    m_exclusive = false;
    m_tag = ANIMATION_H;
    theCommander->playSound("tick.wav", .35, getSoundPadding());
    cycle();
}

void MovingHAnimation::cycle()
{
    m_cpt++;
    attachedFlobo.setOffsetX(m_hOffset - m_hOffsetByStep*m_cpt);
    if (m_cpt == m_step) {
        finishedFlag = true;
        attachedFlobo.setOffsetX(0);
    }
}

/* Flobo moving from one place to another in the vertical axis */
MovingVAnimation::MovingVAnimation(AnimatedFlobo &flobo, int vOffset, int step)
  : FloboAnimation(flobo), m_cpt(0), m_vOffset(vOffset), m_step(step),
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
    attachedFlobo.setOffsetY(m_vOffset - m_vOffsetByStep*m_cpt);
    if (m_cpt == m_step) {
        finishedFlag = true;
        attachedFlobo.setOffsetY(0);
    }
}

/* Flobo falling and bouncing animation */

const int FallingAnimation::BOUNCING_OFFSET_NUM = 9;
const int FallingAnimation::BOUNCING_OFFSET[] = {-3, -4, -3, 0, 3, 6, 8, 6, 3};

FallingAnimation::FallingAnimation(AnimatedFlobo &flobo, int originY, int xOffset, int yOffset, int off) : FloboAnimation(flobo), m_once(false)
{
    this->xOffset = xOffset;
    this->yOffset = yOffset;
    this->off     = off;
    this->step    = 0;
    this->X  = (attachedFlobo.getFloboX()*TSIZE) + xOffset;
    this->Y  = (originY*TSIZE) + yOffset;
    bouncing = BOUNCING_OFFSET_NUM + off;
    attachedFlobo.getAttachedView()->disallowCycle();
    EventFX("start_falling", X+TSIZE/2, Y+TSIZE/2, flobo.getAttachedView()->getPlayerId());
}

void FallingAnimation::cycle()
{
    Y += step++;

    if (Y >= (attachedFlobo.getFloboY()*TSIZE) + yOffset)
    {
        if (!m_once) {
            theCommander->playSound("bam1.wav", .3, getSoundPadding());
            m_once = true;
        }
        Y = (attachedFlobo.getFloboY()*TSIZE) + yOffset;
        if (bouncing == BOUNCING_OFFSET_NUM + off) attachedFlobo.getAttachedView()->allowCycle();

        bouncing--;

        if (bouncing < 0) {
            finishedFlag = true;
            attachedFlobo.setAnimatedState(0);
        }
        else if ((bouncing < BOUNCING_OFFSET_NUM) && (BOUNCING_OFFSET[bouncing] > 0)) {
            attachedFlobo.setAnimatedState(BOUNCING_OFFSET[bouncing]);
        }
        else
            attachedFlobo.setAnimatedState(0);

    }
}

void FallingAnimation::draw(int semiMove, DrawTarget *dt)
{
    IosRect drect;
    drect.x = X;
    int coordY = Y;
    if (bouncing >=0)
    {
        for (int i = bouncing-off; i <= bouncing && i < BOUNCING_OFFSET_NUM ; i++)
            if (i>=0)
                coordY += BOUNCING_OFFSET[i];
    }
    attachedFlobo.renderShadowAt(X, coordY, dt);
    attachedFlobo.renderAt(X, coordY, dt);
}

/* Flobo exploding and vanishing animation */
VanishAnimation::VanishAnimation(AnimatedFlobo &flobo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer, int floboNum, int groupSize, int groupNum, int phase) : FloboAnimation(flobo), floboNum(floboNum), groupSize(groupSize), groupNum(groupNum), phase(phase)
{
    this->xOffset = xOffset;
    this->yOffset = yOffset;
    this->X = (attachedFlobo.getFloboX()*TSIZE) + xOffset;
    this->Y = (attachedFlobo.getFloboY()*TSIZE) + yOffset;
    this->color = attachedFlobo.getFloboState();
    if (color > FLOBO_EMPTY)
        color -= FLOBO_BLUE;
    iter = 0;
    once = false;
    enabled = false;
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
    this->delay = delay;
    attachedFlobo.getAttachedView()->disallowCycle();
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
                    attachedFlobo.getScreenCoordinateX() + TSIZE/2,
                    attachedFlobo.getScreenCoordinateY() + TSIZE/2,
                    attachedFlobo.getAttachedView()->getPlayerId());
        }
        if (iter == 20 + delay) {
            attachedFlobo.getAttachedView()->allowCycle();
            if ((groupNum == 0) && (floboNum == 0))
                EventFX("vanish_phase", groupSize,phase, attachedFlobo.getAttachedView()->getPlayerId());
        }
        else if (iter == 50 + delay) {
            finishedFlag = true;
            attachedFlobo.setVisible(false);
        }
    }
}

void VanishAnimation::draw(int semiMove, DrawTarget *dt)
{
    if (iter < (10 + delay)) {
        if (iter % 2 == 0) {
            attachedFlobo.renderAt(X, Y, dt);
        }
    }
    else {
		const FloboTheme *theme = attachedFlobo.getAttachedTheme();

        IosRect drect, xrect;
        int iter2 = iter - 10 - delay;
        int shrinkingImage = (iter - 10 - delay) / 4;
        if (shrinkingImage < 4) {
            IosSurface *shrinkingSurface, *explodingSurface;
            shrinkingSurface = theme->getShrinkingSurfaceForIndex(shrinkingImage);
            explodingSurface = theme->getExplodingSurfaceForIndex(shrinkingImage);

            drect.x = X;
            drect.y = Y;
            drect.w = shrinkingSurface->w;
            drect.h = shrinkingSurface->h;

            dt->draw(shrinkingSurface, NULL, &drect);
            int xrectY = Y + (int)(2.5 * pow(iter - 16 - delay, 2) - 108);
            xrect.w = explodingSurface->w;
            xrect.h = explodingSurface->h;
            xrect.x = X - iter2 * iter2;
            xrect.y = xrectY;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = X - iter2;
            xrect.y = xrectY + iter2;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = X + iter2;
            xrect.y = xrectY + iter2;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = X + iter2 * iter2;
            xrect.y = xrectY;
            dt->draw(explodingSurface, NULL, &xrect);
        }
    }
}

VanishSoundAnimation::VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer, float soundPadding)
  : phase(phase), step(0), once(false), synchronizer(synchronizer), soundPadding(soundPadding)
{
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
            theCommander->playSound(sound_splash[phase-1>7?7:phase-1], sound_splash_volume, soundPadding);
            finishedFlag = true;
        }
    }
}

void VanishSoundAnimation::draw(int semiMove, DrawTarget *dt)
{
    // do nothing
}

NeutralPopAnimation::NeutralPopAnimation(AnimatedFlobo &flobo, int delay, AnimationSynchronizer *synchronizer)
    : FloboAnimation(flobo), synchronizer(synchronizer), iter(0), delay(delay), once(false),
      X(attachedFlobo.getScreenCoordinateX()), Y(attachedFlobo.getScreenCoordinateY())
{
    synchronizer->push();
    synchronizer->incrementUsage();
    const FloboTheme *attachedTheme = attachedFlobo.getAttachedTheme();
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
            theCommander->playSound("pop.wav", .25, getSoundPadding());
            EventFX("neutral_pop",
                    attachedFlobo.getScreenCoordinateX() + TSIZE/2,
                    attachedFlobo.getScreenCoordinateY() + TSIZE/2,
                    attachedFlobo.getAttachedView()->getPlayerId());
            enabled = true;
        }
        else if (iter == 30 + delay) {
            attachedFlobo.setVisible(false);
            finishedFlag = true;
        }
    }
}

void NeutralPopAnimation::draw(int semiMove, DrawTarget *dt)
{
    IosRect drect;
    drect.x = X;
    drect.y = Y;
    drect.w = neutralPop[0]->w;
    drect.h = neutralPop[0]->h;
    if (iter - delay < 20) {
        dt->draw(neutralPop[0], NULL, &drect);
    }
    else if (iter - delay < 23) {
        dt->draw(neutralPop[1], NULL, &drect);
    }
    else if (iter - delay < 26) {
        dt->draw(neutralPop[2], NULL, &drect);
    }
}

SmoothBounceAnimation::SmoothBounceAnimation(AnimatedFlobo &flobo, AnimationSynchronizer *synchronizer, int depth) :
    FloboAnimation(flobo), bounceMax(depth)
{
    bounceOffset = 0;
    bouncePhase = 0;
    this->synchronizer = synchronizer;
    synchronizer->incrementUsage();
    origX = attachedFlobo.getScreenCoordinateX();
    origY = attachedFlobo.getScreenCoordinateY();
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

void SmoothBounceAnimation::draw(int semiMove, DrawTarget *dt)
{
    attachedFlobo.renderAt(origX, origY + bounceOffset, dt);
}

GameOverFallAnimation::GameOverFallAnimation(AnimatedFlobo &flobo, int delay)
    : FloboAnimation(flobo), delay(delay), Y(attachedFlobo.getScreenCoordinateY()), yAccel(10)
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
            attachedFlobo.setVisible(false);
            finishedFlag = true;
        }
    }
    else delay--;
}

void GameOverFallAnimation::draw(int semiMove, DrawTarget *dt)
{
    attachedFlobo.renderAt(attachedFlobo.getScreenCoordinateX(), Y, dt);
}


ScreenShakingAnimation::ScreenShakingAnimation(int duration, int shakeCount,
                                               float amplX, float amplY,
                                               float smoothFactor,
                                               AnimationSynchronizer *synchronizer)
    : m_iter(0),
      m_duration(duration), m_shakeCount(shakeCount),
      m_amplX(amplX), m_amplY(amplY),
      m_smoothFactor(smoothFactor),
      m_synchronizer(synchronizer),
      m_sine(0),
      m_dc(NULL)
{
    m_sineStep = (M_PI * m_shakeCount * 2) / m_duration;
    if (m_synchronizer != NULL)
        m_synchronizer->incrementUsage();
    m_exclusive = false;
}

ScreenShakingAnimation::~ScreenShakingAnimation()
{
    if (m_dc != NULL) {
        m_dc->setOffset(0, 0);
    }
    if (m_synchronizer != NULL)
        m_synchronizer->decrementUsage();
}

void ScreenShakingAnimation::cycle()
{
    if (m_synchronizer != NULL)
        if (! m_synchronizer->isSynchronized())
            return;
    ++m_iter;
    if (m_iter > m_duration) {
        m_dc->setOffset(0, 0);
        finishedFlag = true;
    }
    m_sine += m_sineStep;
}

void ScreenShakingAnimation::draw(int semiMove, DrawTarget *dt)
{
    if (finishedFlag)
        return;
    if (m_synchronizer != NULL)
        if (! m_synchronizer->isSynchronized())
            return;
    DrawContext *dc = dynamic_cast<DrawContext *>(dt);
    if (dc != NULL) {
        m_dc = dc;
        float sine = sin(m_sine);
        float smooth = (float)(m_duration - m_iter) / (float)m_duration;// * m_smoothFactor;
        m_dc->setOffset((sine * m_amplX) * smooth,  (sine * m_amplY) * smooth);
    }
}
