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
  : _finishedFlag(false), _enabled(true),
    _exclusive(true), _tag(ANIMATION_NO_TAG)
{
}

bool Animation::isFinished() const
{
    return _finishedFlag;
}

bool Animation::isEnabled() const
{
    return _enabled;
}

float FloboAnimation::getSoundPadding() const
{
    return ((float)_attachedFlobo.lock()->getScreenCoordinateX() / 640.)*2. - 1.;
}

/* Neutral falling animation */
NeutralAnimation::NeutralAnimation(const std::weak_ptr<AnimatedFlobo> &flobo, int delay, AnimationSynchronizer *synchronizer) : FloboAnimation(flobo)
{
    auto attachedFlobo = _attachedFlobo.lock();
    this->_X = attachedFlobo->getScreenCoordinateX();
    this->_Y = attachedFlobo->getScreenCoordinateY();
    this->_currentY = attachedFlobo->getAttachedView()->getScreenCoordinateY(0);
    _step = 0;
    this->_delay = delay;
    attachedFlobo->getAttachedView()->disallowCycle();
    this->_synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
}

NeutralAnimation::~NeutralAnimation()
{
    _synchronizer->decrementUsage();
}

static const char *sound_bim[2] =        { "bim1.wav", "bim2.wav" };
static const float sound_bim_volume[2] = { .6, .6 };

void NeutralAnimation::cycle()
{
    if (_delay >=0) {
        _delay--;
    }
    else {
        _currentY += (int)_step;
        _step += 0.8;
        if (_currentY >= _Y) {
            int choosenSound = random() % 2;
            // TODO jeko
            /*EventFX("neutral_bouncing",
                attachedFlobo->getScreenCoordinateX() + TSIZE/2,
                attachedFlobo->getScreenCoordinateY() + TSIZE/2,
                attachedFlobo->getAttachedView()->getPlayerId());*/
            theCommander->playSound(sound_bim[choosenSound], sound_bim_volume[choosenSound], getSoundPadding());
            _finishedFlag = true;
            _attachedFlobo.lock()->getAttachedView()->allowCycle();
            _synchronizer->pop();
        }
    }
}

void NeutralAnimation::draw(int semiMove, DrawTarget *dt)
{
    _attachedFlobo.lock()->renderAt(_X, _currentY, dt);
}

/* Animation synchronization helper */
AnimationSynchronizer::AnimationSynchronizer()
{
    _currentCounter = 0;
    _currentUsage = 0;
}

void AnimationSynchronizer::push()
{
    _currentCounter++;
}

void AnimationSynchronizer::pop()
{
    _currentCounter--;
}

bool AnimationSynchronizer::isSynchronized()
{
    return (_currentCounter <= 0);
}

void AnimationSynchronizer::incrementUsage()
{
    _currentUsage++;
}

void AnimationSynchronizer::decrementUsage()
{
    _currentUsage--;
    if (_currentUsage == 0)
        delete this;
}

/* Companion turning around main flobo animation */
TurningAnimation::TurningAnimation(const std::weak_ptr<AnimatedFlobo> &companionFlobo,
                                   bool counterclockwise) : FloboAnimation(companionFlobo), _NUMSTEPS(6)
{
    _enabled = false;
    _exclusive = false;
    _tag = ANIMATION_ROTATE;
    _cpt = 0;
    _angle = (3.14 / 2) * (counterclockwise ? -1 : 1);
    _step = (3.14 / 2) / _NUMSTEPS * (counterclockwise ? 1 : -1);
    cycle();
}

void TurningAnimation::cycle()
{
    static const char * sound_fff = "fff.wav";
    static const float  sound_fff_volume = .35;
    auto attachedFlobo = _attachedFlobo.lock();
    
    if (_cpt == 0) {
        theCommander->playSound(sound_fff, sound_fff_volume, getSoundPadding());
        EventFX("turning",
                attachedFlobo->getScreenCoordinateX() + TSIZE/2,
                attachedFlobo->getScreenCoordinateY() + TSIZE/2,
                attachedFlobo->getAttachedView()->getPlayerId());
    }
    _cpt++;
    _angle += _step;
    if (_cpt == _NUMSTEPS) {
        _finishedFlag = true;
        attachedFlobo->setRotation(0.);
    }
    else
        attachedFlobo->setRotation(_angle);
}

/* Flobo moving from one place to another in the horizontal axis */
MovingHAnimation::MovingHAnimation(const std::weak_ptr<AnimatedFlobo> &flobo, int hOffset, int step)
  : FloboAnimation(flobo), _cpt(0), _hOffset(hOffset), _step(step),
    _hOffsetByStep((float)hOffset/(float)step)
{
    _enabled = false;
    _exclusive = false;
    _tag = ANIMATION_H;
    theCommander->playSound("tick.wav", .35, getSoundPadding());
    cycle();
}

void MovingHAnimation::cycle()
{
    _cpt++;
    auto attachedFlobo = _attachedFlobo.lock();
    attachedFlobo->setOffsetX(_hOffset - _hOffsetByStep*_cpt);
    if (_cpt == _step) {
        _finishedFlag = true;
        attachedFlobo->setOffsetX(0);
    }
}

/* Flobo moving from one place to another in the vertical axis */
MovingVAnimation::MovingVAnimation(const std::weak_ptr<AnimatedFlobo> &flobo, int vOffset, int step)
  : FloboAnimation(flobo), _cpt(0), _vOffset(vOffset), _step(step),
    _vOffsetByStep((float)vOffset/(float)step)
{
    _enabled = false;
    _exclusive = false;
    _tag = ANIMATION_V;
    cycle();
}

void MovingVAnimation::cycle()
{
    _cpt++;
    auto attachedFlobo = _attachedFlobo.lock();
    attachedFlobo->setOffsetY(_vOffset - _vOffsetByStep*_cpt);
    if (_cpt == _step) {
        _finishedFlag = true;
        attachedFlobo->setOffsetY(0);
    }
}

/* Flobo falling and bouncing animation */

const int FallingAnimation::_BOUNCING_OFFSET_NUM = 9;
const int FallingAnimation::_BOUNCING_OFFSET[] = {-3, -4, -3, 0, 3, 6, 8, 6, 3};

FallingAnimation::FallingAnimation(const std::weak_ptr<AnimatedFlobo> &flobo, int originY, int xOffset, int yOffset, int off) : FloboAnimation(flobo), _once(false)
{
    auto attachedFlobo = _attachedFlobo.lock();
    this->_xOffset = xOffset;
    this->_yOffset = yOffset;
    this->_off     = off;
    this->_step    = 0;
    this->_X  = (attachedFlobo->getFloboX()*TSIZE) + xOffset;
    this->_Y  = (originY*TSIZE) + yOffset;
    _bouncing = _BOUNCING_OFFSET_NUM + off;
    attachedFlobo->getAttachedView()->disallowCycle();
    EventFX("start_falling", _X+TSIZE/2, _Y+TSIZE/2, attachedFlobo->getAttachedView()->getPlayerId());
}

void FallingAnimation::cycle()
{
    auto attachedFlobo = _attachedFlobo.lock();
    _Y += _step++;

    if (_Y >= (attachedFlobo->getFloboY()*TSIZE) + _yOffset)
    {
        if (!_once) {
            theCommander->playSound("bam1.wav", .3, getSoundPadding());
            _once = true;
        }
        _Y = (attachedFlobo->getFloboY()*TSIZE) + _yOffset;
        if (_bouncing == _BOUNCING_OFFSET_NUM + _off) attachedFlobo->getAttachedView()->allowCycle();

        _bouncing--;

        if (_bouncing < 0) {
            _finishedFlag = true;
            attachedFlobo->setAnimatedState(0);
        }
        else if ((_bouncing < _BOUNCING_OFFSET_NUM) && (_BOUNCING_OFFSET[_bouncing] > 0)) {
            attachedFlobo->setAnimatedState(_BOUNCING_OFFSET[_bouncing]);
        }
        else
            attachedFlobo->setAnimatedState(0);

    }
}

void FallingAnimation::draw(int semiMove, DrawTarget *dt)
{
    auto attachedFlobo = _attachedFlobo.lock();
    IosRect drect;
    drect.x = _X;
    int coordY = _Y;
    if (_bouncing >=0)
    {
        for (int i = _bouncing-_off; i <= _bouncing && i < _BOUNCING_OFFSET_NUM ; i++)
            if (i>=0)
                coordY += _BOUNCING_OFFSET[i];
    }
    attachedFlobo->renderShadowAt(_X, coordY, dt);
    attachedFlobo->renderAt(_X, coordY, dt);
}

/* Flobo exploding and vanishing animation */
VanishAnimation::VanishAnimation(const std::weak_ptr<AnimatedFlobo> &flobo, int delay, int xOffset, int yOffset, AnimationSynchronizer *synchronizer, int floboNum, int groupSize, int groupNum, int phase) : FloboAnimation(flobo), _floboNum(floboNum), _groupSize(groupSize), _groupNum(groupNum), _phase(phase)
{
    auto attachedFlobo = _attachedFlobo.lock();
    this->_xOffset = xOffset;
    this->_yOffset = yOffset;
    this->_X = (attachedFlobo->getFloboX()*TSIZE) + xOffset;
    this->_Y = (attachedFlobo->getFloboY()*TSIZE) + yOffset;
    this->_color = attachedFlobo->getFloboState();
    if (_color > FLOBO_EMPTY)
        _color -= FLOBO_BLUE;
    _iter = 0;
    _once = false;
    _enabled = false;
    this->_synchronizer = synchronizer;
    synchronizer->incrementUsage();
    synchronizer->push();
    this->_delay = delay;
    attachedFlobo->getAttachedView()->disallowCycle();
}

VanishAnimation::~VanishAnimation()
{
    _synchronizer->decrementUsage();
}

void VanishAnimation::cycle()
{
    auto attachedFlobo = _attachedFlobo.lock();
    if (_once == false) {
        _once = true;
        _synchronizer->pop();
    }
    else if (_synchronizer->isSynchronized()) {
        _enabled = true;
        _iter ++;
        if (_iter == 1) {
            EventFX("vanish",
                    attachedFlobo->getScreenCoordinateX() + TSIZE/2,
                    attachedFlobo->getScreenCoordinateY() + TSIZE/2,
                    attachedFlobo->getAttachedView()->getPlayerId());
        }
        if (_iter == 20 + _delay) {
            attachedFlobo->getAttachedView()->allowCycle();
            if ((_groupNum == 0) && (_floboNum == 0))
                EventFX("vanish_phase", _groupSize,_phase, attachedFlobo->getAttachedView()->getPlayerId());
        }
        else if (_iter == 50 + _delay) {
            _finishedFlag = true;
            attachedFlobo->setVisible(false);
        }
    }
}

void VanishAnimation::draw(int semiMove, DrawTarget *dt)
{
    auto attachedFlobo = _attachedFlobo.lock();
    if (_iter < (10 + _delay)) {
        if (_iter % 2 == 0) {
            attachedFlobo->renderAt(_X, _Y, dt);
        }
    }
    else {
		const FloboTheme *theme = attachedFlobo->getAttachedTheme();

        IosRect drect, xrect;
        int iter2 = _iter - 10 - _delay;
        int shrinkingImage = (_iter - 10 - _delay) / 4;
        if (shrinkingImage < 4) {
            IosSurface *shrinkingSurface, *explodingSurface;
            shrinkingSurface = theme->getShrinkingSurfaceForIndex(shrinkingImage);
            explodingSurface = theme->getExplodingSurfaceForIndex(shrinkingImage);

            drect.x = _X + (TSIZE >> 2) - (shrinkingSurface->w >> 2);
            drect.y = _Y + (TSIZE >> 2) - (shrinkingSurface->h >> 2);
            drect.w = shrinkingSurface->w;
            drect.h = shrinkingSurface->h;

            dt->draw(shrinkingSurface, NULL, &drect);
            int xrectY = _Y  + (TSIZE >> 2) - (explodingSurface->h >> 2) + (int)(2.5 * pow(_iter - 16 - _delay, 2) - 108);
            xrect.w = explodingSurface->w;
            xrect.h = explodingSurface->h;
            xrect.x = _X  + (TSIZE >> 2) - (explodingSurface->w >> 2) - iter2 * iter2;
            xrect.y = xrectY;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = _X  + (TSIZE >> 2) - (explodingSurface->w >> 2) - iter2;
            xrect.y = xrectY + iter2;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = _X  + (TSIZE >> 2) - (explodingSurface->w >> 2) + iter2;
            xrect.y = xrectY + iter2;
            dt->draw(explodingSurface, NULL, &xrect);
            xrect.x = _X  + (TSIZE >> 2) - (explodingSurface->w >> 2) + iter2 * iter2;
            xrect.y = xrectY;
            dt->draw(explodingSurface, NULL, &xrect);
        }
    }
}

VanishSoundAnimation::VanishSoundAnimation(int phase, AnimationSynchronizer *synchronizer, float soundPadding)
  : _phase(phase), _step(0), _once(false), _synchronizer(synchronizer), _soundPadding(soundPadding)
{
    synchronizer->incrementUsage();
    synchronizer->push();
}

VanishSoundAnimation::~VanishSoundAnimation()
{
    _synchronizer->decrementUsage();
}

static const char *sound_splash[8] = {
    "splash1.wav", "splash2.wav", "splash3.wav", "splash4.wav",
    "splash5.wav", "splash6.wav", "splash7.wav", "splash8.wav" };
static float sound_splash_volume = 1.0;

void VanishSoundAnimation::cycle()
{
    if (_once == false) {
        _once = true;
        _synchronizer->pop();
    }
    else if (_synchronizer->isSynchronized()) {
        _step++;
        if (_step == 1) {
            theCommander->playSound(sound_splash[_phase-1>7?7:_phase-1], sound_splash_volume, _soundPadding);
            _finishedFlag = true;
        }
    }
}

void VanishSoundAnimation::draw(int semiMove, DrawTarget *dt)
{
    // do nothing
}

NeutralPopAnimation::NeutralPopAnimation(std::weak_ptr<AnimatedFlobo> flobo, int delay, AnimationSynchronizer *synchronizer)
    : FloboAnimation(flobo), _synchronizer(synchronizer), _iter(0), _delay(delay), _once(false)
{
    auto attachedFlobo = _attachedFlobo.lock();
    _X = attachedFlobo->getScreenCoordinateX();
    _Y = attachedFlobo->getScreenCoordinateY();

    synchronizer->push();
    synchronizer->incrementUsage();
    const FloboTheme *attachedTheme = attachedFlobo->getAttachedTheme();
    _neutralPop[0] = attachedTheme->getExplodingSurfaceForIndex(0);
    _neutralPop[1] = attachedTheme->getExplodingSurfaceForIndex(1);
    _neutralPop[2] = attachedTheme->getExplodingSurfaceForIndex(2);
    _enabled = false;
}

NeutralPopAnimation::~NeutralPopAnimation()
{
    _synchronizer->decrementUsage();
}

void NeutralPopAnimation::cycle()
{
    auto attachedFlobo = _attachedFlobo.lock();
    if (_once == false) {
        _once = true;
        _synchronizer->pop();
    }
    else if (_synchronizer->isSynchronized()) {
        _iter ++;
        if (_iter == 17 + _delay) {
            theCommander->playSound("pop.wav", .25, getSoundPadding());
            EventFX("neutral_pop",
                    attachedFlobo->getScreenCoordinateX() + TSIZE/2,
                    attachedFlobo->getScreenCoordinateY() + TSIZE/2,
                    attachedFlobo->getAttachedView()->getPlayerId());
            _enabled = true;
        }
        else if (_iter == 30 + _delay) {
            attachedFlobo->setVisible(false);
            _finishedFlag = true;
        }
    }
}

void NeutralPopAnimation::draw(int semiMove, DrawTarget *dt)
{
    IosRect drect;
    drect.x = _X;
    drect.y = _Y;
    drect.w = _neutralPop[0]->w;
    drect.h = _neutralPop[0]->h;
    if (_iter - _delay < 20) {
        dt->draw(_neutralPop[0], NULL, &drect);
    }
    else if (_iter - _delay < 23) {
        dt->draw(_neutralPop[1], NULL, &drect);
    }
    else if (_iter - _delay < 26) {
        dt->draw(_neutralPop[2], NULL, &drect);
    }
}

SmoothBounceAnimation::SmoothBounceAnimation(std::weak_ptr<AnimatedFlobo> flobo, AnimationSynchronizer *synchronizer, int depth) :
    FloboAnimation(flobo), _bounceMax(depth)
{
    auto attachedFlobo = _attachedFlobo.lock();

    _bounceOffset = 0;
    _bouncePhase = 0;
    this->_synchronizer = synchronizer;
    synchronizer->incrementUsage();
    _origX = attachedFlobo->getScreenCoordinateX();
    _origY = attachedFlobo->getScreenCoordinateY();
    _enabled = false;
}

SmoothBounceAnimation::~SmoothBounceAnimation()
{
    _synchronizer->decrementUsage();
}

void SmoothBounceAnimation::cycle()
{
    if (_synchronizer->isSynchronized()) {
        _enabled = true;
        switch (_bouncePhase) {
            case 0:
                _bounceOffset++;
                if (_bounceOffset == _bounceMax)
                    _bouncePhase++;
                    break;
            case 1:
                _bounceOffset--;
                if (_bounceOffset == 0)
                    _bouncePhase++;
                    break;
            case 2:
                _finishedFlag = true;
                break;
        }
    }
}

void SmoothBounceAnimation::draw(int semiMove, DrawTarget *dt)
{
    _attachedFlobo.lock()->renderAt(_origX, _origY + _bounceOffset, dt);
}

GameOverFallAnimation::GameOverFallAnimation(std::weak_ptr<AnimatedFlobo> flobo, int delay)
    : FloboAnimation(flobo), _delay(delay), _Y(_attachedFlobo.lock()->getScreenCoordinateY()), _yAccel(10)
{
}

GameOverFallAnimation::~GameOverFallAnimation()
{
}

void GameOverFallAnimation::cycle()
{
    if (_delay < 0) {
        if (_yAccel > -20)
            _yAccel -= 1;
        _Y -= _yAccel;
        if (_Y > 480) {
            _attachedFlobo.lock()->setVisible(false);
            _finishedFlag = true;
        }
    }
    else _delay--;
}

void GameOverFallAnimation::draw(int semiMove, DrawTarget *dt)
{
    auto attachedFlobo = _attachedFlobo.lock();
    attachedFlobo->renderAt(attachedFlobo->getScreenCoordinateX(), _Y, dt);
}


ScreenShakingAnimation::ScreenShakingAnimation(int duration, int shakeCount,
                                               float amplX, float amplY,
                                               float smoothFactor,
                                               AnimationSynchronizer *synchronizer)
    : _iter(0),
      _duration(duration), _shakeCount(shakeCount),
      _amplX(amplX), _amplY(amplY),
      _smoothFactor(smoothFactor),
      _synchronizer(synchronizer),
      _sine(0),
      _dc(NULL)
{
    _sineStep = (M_PI * _shakeCount * 2) / _duration;
    if (_synchronizer != NULL)
        _synchronizer->incrementUsage();
    _exclusive = false;
}

ScreenShakingAnimation::~ScreenShakingAnimation()
{
    if (_dc != NULL) {
        _dc->setOffset(0, 0);
    }
    if (_synchronizer != NULL)
        _synchronizer->decrementUsage();
}

void ScreenShakingAnimation::cycle()
{
    if (_synchronizer != NULL)
        if (! _synchronizer->isSynchronized())
            return;
    ++_iter;
    if (_iter > _duration) {
        if (_dc != NULL)
            _dc->setOffset(0, 0);
        _finishedFlag = true;
    }
    _sine += _sineStep;
}

void ScreenShakingAnimation::draw(int semiMove, DrawTarget *dt)
{
    if (_finishedFlag)
        return;
    if (_synchronizer != NULL)
        if (! _synchronizer->isSynchronized())
            return;
    DrawContext *dc = dynamic_cast<DrawContext *>(dt);
    if (dc != NULL) {
        _dc = dc;
        float sine = sin(_sine);
        float smooth = (float)(_duration - _iter) / (float)_duration;// * m_smoothFactor;
        _dc->setOffset((sine * _amplX) * smooth,  (sine * _amplY) * smooth);
    }
}
