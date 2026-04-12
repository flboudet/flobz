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

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "GTLog.h"
#include "GameView.h"
#include "FloboAnimations.h"
#include "AnimatedFlobo.h"
#include "FloboGame.h"
#include "audio.h"
#include "HiScores.h"
#include "FPCommander.h"

GameView::GameView(FloboGameFactory *attachedFloboGameFactory,
                   int playerId,
                   FloboSetTheme *attachedThemeSet,
                   LevelTheme *attachedLevelTheme)
  : _playerId(playerId), _showNextFlobos(true), _showShadows(true),
    _attachedThemeSet(attachedThemeSet), _attachedLevelTheme(attachedLevelTheme),
    _attachedFloboFactory(this), _delayBeforeGameOver(60), _haveDisplay(true),
    _xOffset(0), _yOffset(- TSIZE), _nXOffset(0), _nYOffset(0),
    _neutralXOffset(-1), _neutralYOffset(-1)
{
    setupLayout(playerId);
    initCommon(attachedFloboGameFactory);
    setScoreDisplayPosition(attachedLevelTheme->getScoreDisplayX(playerId),
                            attachedLevelTheme->getScoreDisplayY(playerId));
    _scoreDisplay->setFont(attachedLevelTheme->getScoreFont());
}

void GameView::setupLayout(int playerId)
{
    setPlayerId(playerId);
    setPosition(_attachedLevelTheme->getFlobobanX(playerId),
                _attachedLevelTheme->getFlobobanY(playerId));
    setNextFlobosPosition(_attachedLevelTheme->getNextFlobosX(playerId),
                         _attachedLevelTheme->getNextFlobosY(playerId));
    setNeutralFlobosDisplayPosition(_attachedLevelTheme->getNeutralDisplayX(playerId),
                                   _attachedLevelTheme->getNeutralDisplayY(playerId));
    setShowNextFlobos(_attachedLevelTheme->getShouldDisplayNext(playerId));
    setShowShadows(_attachedLevelTheme->getShouldDisplayShadows(playerId));
    setShowEyes(_attachedLevelTheme->getShouldDisplayEyes(playerId));
}

void GameView::initCommon(FloboGameFactory *attachedFloboGameFactory)
{
	attachedGame = attachedFloboGameFactory->createFloboGame(&_attachedFloboFactory);
    _scoreDisplay.reset(new PlayerGameStatDisplay(attachedGame->getGameStat()));
    attachedGame->addGameListener(this);
	_gameRunning = true;
	_enemyGame = NULL;
    _skippedCycle = false;
    _cycleAllowance = 0;
    _newMetaCycleStart = false;
}

GameView::GameView(FloboGameFactory *attachedFloboGameFactory)
  : _attachedThemeSet(NULL), _attachedLevelTheme(NULL),
    _attachedFloboFactory(this), _delayBeforeGameOver(60), _haveDisplay(false),
    _neutralXOffset(-1), _neutralYOffset(-1)
{
    initCommon(attachedFloboGameFactory);
}

GameView::~GameView()
{
    while (_viewAnimations.size() > 0) {
        Animation *currentAnimation = _viewAnimations[0];
        _viewAnimations.removeAtKeepOrder(0);
        delete currentAnimation;
    }
    delete attachedGame;
}

void GameView::setEnemyGame(FloboGame *enemyGame)
{
	this->_enemyGame = enemyGame;
}

int GameView::getValenceForFlobo(Flobo *flobo) const
{
    if (!_haveDisplay) return 0;
    int i = flobo->getFloboX();
    int j = flobo->getFloboY();
    FloboState currentFloboState = flobo->getFloboState();
    AnimatedFlobo *down  = (AnimatedFlobo *)(attachedGame->getFloboAt(i, j+1).get());
    AnimatedFlobo *right = (AnimatedFlobo *)(attachedGame->getFloboAt(i+1, j).get());
    AnimatedFlobo *up    = (AnimatedFlobo *)(attachedGame->getFloboAt(i, j-1).get());
    AnimatedFlobo *left  = (AnimatedFlobo *)(attachedGame->getFloboAt(i-1, j).get());

    FloboState downState = (down == nullptr) || (down->isRenderingAnimation()) ? FLOBO_EMPTY : down->getFloboState();
    FloboState rightState = (right == nullptr) || (right->isRenderingAnimation()) ? FLOBO_EMPTY : right->getFloboState();
    FloboState upState = (up == nullptr) || (up->isRenderingAnimation()) ? FLOBO_EMPTY : up->getFloboState();
    FloboState leftState = (left == nullptr)   || (left->isRenderingAnimation()) ? FLOBO_EMPTY : left->getFloboState();

	return (leftState  == currentFloboState ? 0x8 : 0) | (upState  == currentFloboState ? 0x4 : 0) |
	       (rightState == currentFloboState ? 0x2 : 0) | (downState == currentFloboState ? 0x1 : 0);
}

bool GameView::isGameOver() const
{
    if ((!_gameRunning) && (_delayBeforeGameOver < 0))
        return true;
    return false;
}

void GameView::cycleAnimation(void)
{
    if (_haveDisplay) {
        // Handle end of game
        if (!_gameRunning) {
            _delayBeforeGameOver--;
        }

        // Cycling every flobo's animation
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            static_cast<AnimatedFlobo *>(iter.get().get())->cycleAnimation();
        }

        // Cycling view's animations
        if (_viewAnimations.size() > 0) {
            Animation *currentAnimation = _viewAnimations[0];
            if (currentAnimation->isFinished()) {
                _viewAnimations.removeKeepOrder(currentAnimation);
                delete currentAnimation;
            }
            else {
                currentAnimation->cycle();
            }
        }
    }

    // If there is a skipped cycle to do, do it
    if (attachedGame->isEndOfCycle() && attachedGame->isGameRunning() && !_newMetaCycleStart) cycleGame();
}

void GameView::cycleGame()
{
    // If we are not allowed to cycle the game, mark it
    if (cycleAllowed()) {
        _skippedCycle = false;
        attachedGame->cycle();
    }
    else {
        _skippedCycle = true;
    }
}

void GameView::moveLeft()
{
    if (cycleAllowed()) attachedGame->moveLeft();
}

void GameView::moveRight()
{
    if (cycleAllowed()) attachedGame->moveRight();
}

void GameView::rotateLeft()
{
    if (cycleAllowed()) attachedGame->rotate(true);
}

void GameView::rotateRight()
{
    if (cycleAllowed()) attachedGame->rotate(false);
}


void GameView::render(DrawTarget *dt)
{
    if (!_haveDisplay) return;
	IosRect drect;
    IosRect vrect;
	vrect.x = _xOffset;
	vrect.y = _yOffset;
	vrect.w = TSIZE * FLOBOBAN_DIMX;
	vrect.h = TSIZE * FLOBOBAN_DIMY;

    bool displayFallings = this->cycleAllowed();
    // Render shadows
    if (_showShadows) {
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get().get());
            if (displayFallings || !currentFlobo->isFalling()) currentFlobo->renderShadow(dt);
        }
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get().get());
            if (displayFallings || !currentFlobo->isFalling()) currentFlobo->renderShadow(dt);
        }
    }
    // Render flobos
    for (FloboDefaultIterator iter(attachedGame) ;
         ! iter.end() ; ++iter) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get().get());
        if (displayFallings || !currentFlobo->isFalling()) currentFlobo->render(dt);
    }

    if (_showNextFlobos) {
        drect.x = _nXOffset;
        drect.y = _nYOffset;
        drect.w = TSIZE;
        drect.h = TSIZE * 2;
        // Drawing next flobos
        const FloboTheme &nextFloboTheme =
            _attachedThemeSet->getFloboTheme(attachedGame->getNextFalling());
        IosSurface *currentSurface = nextFloboTheme.getFloboSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = _nXOffset;
            drect.y = _nYOffset + TSIZE;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            IosSurface *s = nextFloboTheme.getEyeSurfaceForIndex(0);
            drect.x += nextFloboTheme.getEyeSurfaceOffsetX();
            drect.y += nextFloboTheme.getEyeSurfaceOffsetY();
            drect.w = s->w;
            drect.h = s->h;
            dt->draw(s, NULL, &drect);
        }
        const FloboTheme &nextCompanionTheme =
            _attachedThemeSet->getFloboTheme(attachedGame->getNextCompanion());
        currentSurface = nextCompanionTheme.getFloboSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = _nXOffset;
            drect.y = _nYOffset;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            IosSurface *s = nextCompanionTheme.getEyeSurfaceForIndex(0);
            drect.x += nextCompanionTheme.getEyeSurfaceOffsetX();
            drect.y += nextCompanionTheme.getEyeSurfaceOffsetY();
            drect.w = s->w;
            drect.h = s->h;
            dt->draw(s, NULL, &drect);
        }
    }

    // Drawing the view animation
    if (_viewAnimations.size() > 0) {
        Animation *currentAnimation = _viewAnimations[0];
        if (!currentAnimation->isFinished()) {
            currentAnimation->draw(0, dt);
        }
    }
}

void GameView::renderNeutral(DrawTarget *dt)
{
    if (!_haveDisplay) return;
	IosRect drect;
    int neutralFlobos = attachedGame->getNeutralFlobos();
    int numGiantNeutral = (neutralFlobos / FLOBOBAN_DIMX) / 4;
    int numBigNeutral = (neutralFlobos / FLOBOBAN_DIMX) % 4;
	int numNeutral = neutralFlobos % FLOBOBAN_DIMX;
    int drect_x = (_neutralXOffset == -1 ? _xOffset : _neutralXOffset);
    int drect_y_base =  (_neutralYOffset == -1 ?
                         _yOffset + 3 + TSIZE + TSIZE : _neutralYOffset);
    IosSurface *neutral = _attachedLevelTheme->getNeutralIndicator();
    IosSurface *bigNeutral = _attachedLevelTheme->getBigNeutralIndicator();
    IosSurface *giantNeutral = _attachedLevelTheme->getGiantNeutralIndicator();
    if (neutral == NULL)
        throw std::runtime_error("Neutral indicator not found!");
    if (bigNeutral == NULL)
        throw std::runtime_error("Big neutral indicator not found!");
    if (giantNeutral == NULL)
        throw std::runtime_error("Giant neutral indicator not found!");
    for (int cpt = 0 ; cpt < numGiantNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - giantNeutral->h;
		drect.w = giantNeutral->w;
		drect.h = giantNeutral->h;
		dt->draw(giantNeutral, NULL, &drect);
		drect_x += giantNeutral->w;
	}
    for (int cpt = 0 ; cpt < numBigNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - bigNeutral->h;
		drect.w = bigNeutral->w;
		drect.h = bigNeutral->h;
		dt->draw(bigNeutral, NULL, &drect);
		drect_x += bigNeutral->w;
	}
    for (int cpt = 0 ; cpt < numNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - neutral->h;
		drect.w = neutral->w;
		drect.h = neutral->h;
		dt->draw(neutral, NULL, &drect);
		drect_x += neutral->w;
	}
}

void GameView::renderScore(DrawTarget *dt)
{
    _scoreDisplay->draw(dt);
}

void GameView::gameDidAddNeutral(std::shared_ptr<Flobo> neutralFlobo, int neutralIndex, int totalNeutral) {
    if (!_haveDisplay) return;
    int x = neutralFlobo->getFloboX();
    int y = neutralFlobo->getFloboY();
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    auto animatedNeutral = std::static_pointer_cast<AnimatedFlobo>(neutralFlobo);
    animatedNeutral->addAnimation(new NeutralAnimation(*animatedNeutral, neutralIndex * 2, synchronizer));
    for (int i = y ; i < FLOBOBAN_DIMY ; i++) {
        auto belowFlobo = std::static_pointer_cast<AnimatedFlobo>(attachedGame->getFloboAt(x, i));
        if (belowFlobo != nullptr) {
            belowFlobo->addAnimation(new SmoothBounceAnimation(*belowFlobo, synchronizer));
        }
    }
    if ((neutralIndex == 2) && (totalNeutral > 4)) {
        if (totalNeutral > 20)
            totalNeutral = 20;
        int duration = totalNeutral * 3;
        int shakeCount = totalNeutral / 4.f;
        float amplY = totalNeutral * 2.f;
        Animation *shakingAnimation = new ScreenShakingAnimation(duration, shakeCount, 0.f, amplY, 1.0f, synchronizer);
        _viewAnimations.add(shakingAnimation);
    }
}

void GameView::fallingsDidMoveLeft(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    if (!_haveDisplay) return;
    std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->flushAnimations(ANIMATION_H);
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->flushAnimations(ANIMATION_H);
	std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->addAnimation(new MovingHAnimation(*std::static_pointer_cast<AnimatedFlobo>(fallingFlobo), TSIZE, 4));
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->addAnimation(new MovingHAnimation(*std::static_pointer_cast<AnimatedFlobo>(companionFlobo), TSIZE, 4));
}

void GameView::fallingsDidMoveRight(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    if (!_haveDisplay) return;
    std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->flushAnimations(ANIMATION_H);
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->flushAnimations(ANIMATION_H);
	std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->addAnimation(new MovingHAnimation(*std::static_pointer_cast<AnimatedFlobo>(fallingFlobo), -TSIZE, 4));
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->addAnimation(new MovingHAnimation(*std::static_pointer_cast<AnimatedFlobo>(companionFlobo), -TSIZE, 4));
}

void GameView::fallingsDidFallingStep(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    if (!_haveDisplay) return;
    std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->flushAnimations(ANIMATION_V);
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->flushAnimations(ANIMATION_V);
	std::static_pointer_cast<AnimatedFlobo>(fallingFlobo)->addAnimation(new MovingVAnimation(*std::static_pointer_cast<AnimatedFlobo>(fallingFlobo), -TSIZE/2, 4));
    std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->addAnimation(new MovingVAnimation(*std::static_pointer_cast<AnimatedFlobo>(companionFlobo), -TSIZE/2, 4));
}

void GameView::companionDidTurn(std::shared_ptr<Flobo> companionFlobo, std::shared_ptr<Flobo> fallingFlobo, bool counterclockwise)
{
    if (!_haveDisplay) return;
    if ((companionFlobo != nullptr) && (fallingFlobo != nullptr)) { // Just to be sure of what we get if data comes from network
        std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->setPartner(std::static_pointer_cast<AnimatedFlobo>(fallingFlobo).get());
        std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->flushAnimations(ANIMATION_ROTATE);
        std::static_pointer_cast<AnimatedFlobo>(companionFlobo)->addAnimation(new TurningAnimation(*std::static_pointer_cast<AnimatedFlobo>(companionFlobo),
                                                                           counterclockwise));
    }
}

void GameView::floboDidFall(std::shared_ptr<Flobo> flobo, int originX, int originY, int nFalledBelow)
{
    if (!_haveDisplay) return;
    std::static_pointer_cast<AnimatedFlobo>(flobo)->flushAnimations();
    std::static_pointer_cast<AnimatedFlobo>(flobo)->addAnimation(new FallingAnimation(*std::static_pointer_cast<AnimatedFlobo>(flobo), originY, _xOffset, _yOffset, nFalledBelow));
}

void GameView::floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase)
{
    if (!_haveDisplay) return;
    double groupPadding = 0.;
    // Exploding flobo animation
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    for (int i = 0, j = floboGroup.size() ; i < j ; i++) {
        AnimatedFlobo *currentFlobo = std::static_pointer_cast<AnimatedFlobo>(floboGroup[i]).get();
        FloboAnimation *newAnimation;
        if (currentFlobo->getFloboState() != FLOBO_NEUTRAL)
            newAnimation = new VanishAnimation(*currentFlobo, i*2 , _xOffset, _yOffset, synchronizer, i, floboGroup.size(), groupNum, phase);
        else
            newAnimation = new NeutralPopAnimation(*currentFlobo, i*2, synchronizer);
        currentFlobo->addAnimation(newAnimation);
        // Compute the center of the vanishing flobos padding
        groupPadding += newAnimation->getSoundPadding();
    }
    _viewAnimations.add(new VanishSoundAnimation(phase, synchronizer, groupPadding / floboGroup.size()));
    // "pastaga" management
    if (groupNum == 0) {
      static const char * sound_yahoohoo[7] = {
        NULL, "yahoohoo.wav", "woho.wav", "pastaga.wav",
        "woho.wav", "woo.wav", "applose.wav"};
      static const float volume_yahoohoo[7] = {0.0, 0.39, 0.25, 0.55, 0.25, 0.35, 0.70};

      int index = 0;

      if (phase>=2)
      {
        index = 6;
      }
      else
      {
        if (phase==1)
        {
          index = 4 + (random()/(RAND_MAX>>1));
        }
        else
        {
          index = random()/(RAND_MAX>>2);
        }
      }
      if (index>1)
          theCommander->playSound(sound_yahoohoo[index], volume_yahoohoo[index]);
    }

}

void GameView::gameDidEndCycle()
{
	if (_enemyGame != NULL) {
		if (attachedGame->getNeutralFlobos() < 0)
			_enemyGame->increaseNeutralFlobos(- attachedGame->getNeutralFlobos());
	}
    GTLogTrace("NewMetaCycleStart");
    _newMetaCycleStart = true;
}

bool GameView::cycleAllowed()
{
    if (_enemyGame != NULL)
        if (!_enemyGame->isGameRunning())
            return false;
    if (_cycleAllowance < 0)
        return false;
    return true;
}

void GameView::gameWin()
{
    attachedGame->getGameStat().is_winner = true;
    _gameRunning = false;
}

void GameView::gameLost()
{
    attachedGame->getGameStat().is_winner = false;
    _gameRunning = false;
    if (!_haveDisplay) return;
    for (int i = 0 ; i <= FLOBOBAN_DIMX ; i++) {
        for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            if (attachedGame->getFloboAt(i, j) != nullptr) {
                AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(attachedGame->getFloboAt(i, j).get());
                currentFlobo->addAnimation(new GameOverFallAnimation(*currentFlobo, (j - FLOBOBAN_DIMY) + abs((FLOBOBAN_DIMX / 2) - i) * 5));
            }
        }
    }
    Animation *shakingAnimation = new ScreenShakingAnimation(80, 12, 10.f, 5.f, 1.f);
    _viewAnimations.add(shakingAnimation);
    theCommander->playSound("earthquake.wav", 1.0);
}

