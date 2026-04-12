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
#include <memory>
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
    int getTag() const { return _tag; }
    bool getExclusive() const { return _exclusive; }
protected:
    bool _finishedFlag;
    bool _enabled;
    bool _exclusive;
    int _tag;
};

/* Abstract animation class for flobos */
class FloboAnimation : public Animation{
public:
    FloboAnimation(const std::shared_ptr<AnimatedFlobo> &flobo):_attachedFlobo(flobo) {}
    float getSoundPadding() const;
protected:
    std::shared_ptr<AnimatedFlobo> _attachedFlobo;
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
    int _currentCounter;
    int _currentUsage;
};

/* Neutral falling animation */
class NeutralAnimation : public FloboAnimation {
  public:
    NeutralAnimation(const std::shared_ptr<AnimatedFlobo> &flobo, int delay, AnimationSynchronizer *synchronizer);
    virtual ~NeutralAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
  private:
    int _X, _Y, _currentY;
    float _step;
    int _delay;
    AnimationSynchronizer *_synchronizer;
};

/* Companion turning around main flobo animation */
class TurningAnimation : public FloboAnimation {
public:
    TurningAnimation(const std::shared_ptr<AnimatedFlobo> &companionFlobo, bool counterclockwise);
    void cycle();
private:
    int _cpt;
    float _angle;
    float _step;
    const int _NUMSTEPS;
};

/* Flobo moving from one place to another, horizontal axis */
class MovingHAnimation : public FloboAnimation {
public:
    MovingHAnimation(const std::shared_ptr<AnimatedFlobo> &flobo, int hOffset, int step);
    void cycle();
private:
    int _cpt, _hOffset, _step;
    float _hOffsetByStep;
};

/* Flobo moving from one place to another, vertical axis */
class MovingVAnimation : public FloboAnimation {
public:
    MovingVAnimation(const std::shared_ptr<AnimatedFlobo> &flobo, int vOffset, int step);
    void cycle();
private:
    int _cpt, _vOffset, _step;
    float _vOffsetByStep;
};

/* Flobo falling and bouncing animation */
class FallingAnimation : public FloboAnimation {
public:
    FallingAnimation(const std::shared_ptr<AnimatedFlobo> &flobo,
                     int originY, int xOffset, int yOffset, int step);
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int _xOffset, _yOffset, _step, _off;
    int _X, _Y;
    int _bouncing;
    static const int _BOUNCING_OFFSET_NUM;
    static const int _BOUNCING_OFFSET[];
    bool _once;
};

/* Flobo exploding and vanishing animation */
class VanishAnimation : public FloboAnimation {
public:
    VanishAnimation(const std::shared_ptr<AnimatedFlobo> &flobo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer, int floboNum, int groupSize, int groupNum, int phase);
    virtual ~VanishAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int _xOffset, _yOffset;
    int _X, _Y, _iter, _color;
    AnimationSynchronizer *_synchronizer;
    bool _once;
    int _delay;

    int _floboNum;
    int _groupSize;
    int _groupNum;
    int _phase;
};

class VanishSoundAnimation : public Animation {
public:
    VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer, float soundPadding);
    virtual ~VanishSoundAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int _phase;
    int _step;
    bool _once;
    AnimationSynchronizer *_synchronizer;
    float _soundPadding;
};

class NeutralPopAnimation : public FloboAnimation {
public:
    NeutralPopAnimation(std::shared_ptr<AnimatedFlobo> flobo, int delay, AnimationSynchronizer *synchronizer);
    virtual ~NeutralPopAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    AnimationSynchronizer *_synchronizer;
    int _iter, _delay;
    bool _once;
    int _X, _Y;
    IosSurface *_neutralPop[3];
};

class SmoothBounceAnimation : public FloboAnimation {
public:
    SmoothBounceAnimation(std::shared_ptr<AnimatedFlobo> flobo, AnimationSynchronizer *synchronizer, int depth = 10);
    virtual ~SmoothBounceAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int _bounceOffset, _bouncePhase, _bounceMax;
    int _origX, _origY;
    AnimationSynchronizer *_synchronizer;
};

class GameOverFallAnimation : public FloboAnimation {
public:
    GameOverFallAnimation(std::shared_ptr<AnimatedFlobo> flobo, int delay);
    virtual ~GameOverFallAnimation();
    void cycle();
    void draw(int semiMove, DrawTarget *dt);
private:
    int _delay;
    int _Y;
    int _yAccel;
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
    int _iter;
    int _duration, _shakeCount;
    float _amplX, _amplY;
    float _smoothFactor;
    AnimationSynchronizer *_synchronizer;
    float _sineStep;
    float _sine;
    DrawContext *_dc;
};

#endif // _FLOBOANIMATIONS_H_

