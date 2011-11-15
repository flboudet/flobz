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

#ifndef _FLOBOANIMATIONS_H_
#define _FLOBOANIMATIONS_H_

void EventFX(const char *name, float x, float y, int player);

#include <stdlib.h>
#include <math.h>
#include "drawcontext.h"
#include "FloboGame.h"

class AnimatedFlobo;

enum AnimationTag {
    ANIMATION_NO_TAG,
    ANIMATION_H,
    ANIMATION_V,
    ANIMATION_ROTATE
};

/* Abstract Animation class */
class Animation {
public:
    Animation();
    virtual ~Animation() {}
    bool isFinished() const;
    bool isEnabled() const;
    virtual void cycle() = 0;
    virtual void draw(int semiMove, DrawTarget *dt) {}
    int getTag() const { return m_tag; }
    bool getExclusive() const { return m_exclusive; }
protected:
    bool finishedFlag;
    bool enabled;
    bool m_exclusive;
    int m_tag;
};

/* Abstract animation class for flobos */
class FloboAnimation : public Animation{
public:
    FloboAnimation(AnimatedFlobo &flobo):attachedFlobo(flobo) {}
    float getSoundPadding() const;
protected:
    AnimatedFlobo &attachedFlobo;
};

/* Animation synchronization helper */
class AnimationSynchronizer {
public:
    AnimationSynchronizer();
    void push();
    void pop();
    bool isSynchronized();
    void incrementUsage();
    void decrementUsage();
private:
    int currentCounter;
    int currentUsage;
};

/* Neutral falling animation */
class NeutralAnimation : public FloboAnimation {
  public:
    NeutralAnimation(AnimatedFlobo &flobo, int delay, AnimationSynchronizer *synchronizer);
    virtual ~NeutralAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
  private:
    int X, Y, currentY;
    float step;
    int delay;
    AnimationSynchronizer *synchronizer;
};

/* Companion turning around main flobo animation */
class TurningAnimation : public FloboAnimation {
public:
    TurningAnimation(AnimatedFlobo &companionFlobo, bool counterclockwise);
    void cycle();
private:
    int cpt;
    float angle;
    float step;
    const int NUMSTEPS;
};

/* Flobo moving from one place to another, horizontal axis */
class MovingHAnimation : public FloboAnimation {
public:
    MovingHAnimation(AnimatedFlobo &flobo, int hOffset, int step);
    void cycle();
private:
    int m_cpt, m_hOffset, m_step;
    float m_hOffsetByStep;
};

/* Flobo moving from one place to another, vertical axis */
class MovingVAnimation : public FloboAnimation {
public:
    MovingVAnimation(AnimatedFlobo &flobo, int vOffset, int step);
    void cycle();
private:
    int m_cpt, m_vOffset, m_step;
    float m_vOffsetByStep;
};

/* Flobo falling and bouncing animation */
class FallingAnimation : public FloboAnimation {
public:
    FallingAnimation(AnimatedFlobo &flobo,
                     int originY, int xOffset, int yOffset, int step);
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int xOffset, yOffset, step, off;
    int X, Y;
    int bouncing;
    static const int BOUNCING_OFFSET_NUM;
    static const int BOUNCING_OFFSET[];
    bool m_once;
};

/* Flobo exploding and vanishing animation */
class VanishAnimation : public FloboAnimation {
public:
    VanishAnimation(AnimatedFlobo &flobo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer, int floboNum, int groupSize, int groupNum, int phase);
    virtual ~VanishAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int xOffset, yOffset;
    int X, Y, iter, color;
    AnimationSynchronizer *synchronizer;
    bool once;
    int delay;

    int floboNum;
    int groupSize;
    int groupNum;
    int phase;
};

class VanishSoundAnimation : public Animation {
public:
    VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer, float soundPadding);
    virtual ~VanishSoundAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int phase;
    int step;
    bool once;
    AnimationSynchronizer *synchronizer;
    float soundPadding;
};

class NeutralPopAnimation : public FloboAnimation {
public:
    NeutralPopAnimation(AnimatedFlobo &flobo, int delay, AnimationSynchronizer *synchronizer);
    virtual ~NeutralPopAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    AnimationSynchronizer *synchronizer;
    int iter, delay;
    bool once;
    int X, Y;
    IosSurface *neutralPop[3];
};

class SmoothBounceAnimation : public FloboAnimation {
public:
    SmoothBounceAnimation(AnimatedFlobo &flobo, AnimationSynchronizer *synchronizer, int depth = 10);
    virtual ~SmoothBounceAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int bounceOffset, bouncePhase, bounceMax;
    int origX, origY;
    AnimationSynchronizer *synchronizer;
};

class GameOverFallAnimation : public FloboAnimation {
public:
    GameOverFallAnimation(AnimatedFlobo &flobo, int delay);
    virtual ~GameOverFallAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int delay;
    int Y;
    int yAccel;
};

class ScreenShakingAnimation : public Animation {
public:
    ScreenShakingAnimation(int duration, int shakeCount,
                           float amplX, float amplY,
                           float smoothFactor,
                           AnimationSynchronizer *synchronizer = NULL);
    virtual ~ScreenShakingAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int m_iter;
    int m_duration, m_shakeCount;
    float m_amplX, m_amplY;
    float m_smoothFactor;
    AnimationSynchronizer *m_synchronizer;
    float m_sineStep;
    float m_sine;
    DrawContext *m_dc;
};

#endif // _FLOBOANIMATIONS_H_

