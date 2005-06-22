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

#ifndef _PUYOANIMATIONS
#define _PUYOANIMATIONS

#include <stdlib.h>
#include <math.h>
#include "glSDL.h"
#include "IosImgProcess.h"
#include "PuyoGame.h"

class AnimatedPuyo;

/* Abstract Animation class */
class Animation {
public:
    Animation();
    bool isFinished() const;
    bool isEnabled() const;
    virtual void cycle() = 0;
    virtual void draw(int semiMove) = 0;
protected:
    bool finishedFlag;
    bool enabled;
};

/* Abstract animation class for puyos */
class PuyoAnimation : public Animation{
public:
    PuyoAnimation(AnimatedPuyo &puyo):attachedPuyo(puyo) {}
protected:
    AnimatedPuyo &attachedPuyo;
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
class NeutralAnimation : public PuyoAnimation {
  public:
    NeutralAnimation(AnimatedPuyo &puyo, int delay, AnimationSynchronizer *synchronizer);
    NeutralAnimation::~NeutralAnimation();
    void cycle();
    void draw(int semiMove);
  private:
    int X, Y, currentY;
    float step;
    int delay;
    AnimationSynchronizer *synchronizer;
};

/* Companion turning around main puyo animation */
class TurningAnimation : public PuyoAnimation {
public:
    TurningAnimation(AnimatedPuyo &companionPuyo,
                     int vector, bool counterclockwise);
    void cycle();
    void draw(int semiMove);
private:
    int companionVector, cpt;
    float angle;
    float step;
    bool counterclockwise;
};

/* Puyo falling and bouncing animation */
class FallingAnimation : public PuyoAnimation {
public:
    FallingAnimation(AnimatedPuyo &puyo,
                     int originY, int xOffset, int yOffset, int step);
    void cycle();
    void draw(int semiMove);
private:
    int xOffset, yOffset, step;
    int X, Y;
    int bouncing;
    static const int BOUNCING_OFFSET_NUM;
    static const int BOUNCING_OFFSET[];
};

/* Puyo exploding and vanishing animation */
class VanishAnimation : public PuyoAnimation {
public:
    VanishAnimation(AnimatedPuyo &puyo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer);
    virtual ~VanishAnimation();
    void cycle();
    void draw(int semiMove);
private:
    int xOffset, yOffset;
    int X, Y, iter, color;
    AnimationSynchronizer *synchronizer;
    bool once;
    int delay;
};

class VanishSoundAnimation : public Animation {
public:
    VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer);
    virtual ~VanishSoundAnimation();
    void cycle();
    void draw(int semiMove);
private:
    int phase;
    int step;
    bool once;
    AnimationSynchronizer *synchronizer;
};

class NeutralPopAnimation : public PuyoAnimation {
public:
    NeutralPopAnimation(AnimatedPuyo &puyo, int delay, AnimationSynchronizer *synchronizer);
    virtual ~NeutralPopAnimation();
    void cycle();
    void draw(int semiMove);
    static void initResources();
private:
    AnimationSynchronizer *synchronizer;
    int iter, delay;
    bool once;
    int X, Y;
    static IIM_Surface *neutralPop[3];
};

class SmoothBounceAnimation : public PuyoAnimation {
public:
    SmoothBounceAnimation(AnimatedPuyo &puyo, AnimationSynchronizer *synchronizer);
    virtual ~SmoothBounceAnimation();
    void cycle();
    void draw(int semiMove);
private:
    int bounceOffset, bouncePhase;
    int origX, origY;
    AnimationSynchronizer *synchronizer;
};

class GameOverFallAnimation : public PuyoAnimation {
public:
    GameOverFallAnimation(AnimatedPuyo &puyo, int delay);
    virtual ~GameOverFallAnimation();
    void cycle();
    void draw(int semiMove);
private:
    int delay;
    int Y;
    int yAccel;
};

#endif // _PUYOANIMATIONS

