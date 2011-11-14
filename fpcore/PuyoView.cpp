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

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "GTLog.h"
#include "PuyoView.h"
#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "FloboGame.h"
#include "audio.h"
#include "HiScores.h"

PuyoView::PuyoView(FloboGameFactory *attachedFloboGameFactory,
                   int playerId,
                   PuyoSetTheme *attachedThemeSet,
                   LevelTheme *attachedLevelTheme)
  : m_playerId(playerId), m_showNextPuyos(true), m_showShadows(true),
    attachedThemeSet(attachedThemeSet), attachedLevelTheme(attachedLevelTheme),
    attachedFloboFactory(this), delayBeforeGameOver(60), haveDisplay(true),
    m_xOffset(0), m_yOffset(- TSIZE), m_nXOffset(0), m_nYOffset(0),
    neutralXOffset(-1), neutralYOffset(-1)
{
    setupLayout(playerId);
    initCommon(attachedFloboGameFactory);
    setScoreDisplayPosition(attachedLevelTheme->getScoreDisplayX(playerId),
                            attachedLevelTheme->getScoreDisplayY(playerId));
    m_scoreDisplay->setFont(attachedLevelTheme->getScoreFont());
}

void PuyoView::setupLayout(int playerId)
{
    setPlayerId(playerId);
    setPosition(attachedLevelTheme->getPuyobanX(playerId),
                attachedLevelTheme->getPuyobanY(playerId));
    setNextPuyosPosition(attachedLevelTheme->getNextPuyosX(playerId),
                         attachedLevelTheme->getNextPuyosY(playerId));
    setNeutralFlobosDisplayPosition(attachedLevelTheme->getNeutralDisplayX(playerId),
                                   attachedLevelTheme->getNeutralDisplayY(playerId));
    setShowNextPuyos(attachedLevelTheme->getShouldDisplayNext(playerId));
    setShowShadows(attachedLevelTheme->getShouldDisplayShadows(playerId));
    setShowEyes(attachedLevelTheme->getShouldDisplayEyes(playerId));
}

void PuyoView::initCommon(FloboGameFactory *attachedFloboGameFactory)
{
	attachedGame = attachedFloboGameFactory->createFloboGame(&attachedFloboFactory);
    m_scoreDisplay.reset(new PlayerGameStatDisplay(attachedGame->getGameStat()));
    attachedGame->addGameListener(this);
	gameRunning = true;
	enemyGame = NULL;
    skippedCycle = false;
    cycleAllowance = 0;
    newMetaCycleStart = false;
}

PuyoView::PuyoView(FloboGameFactory *attachedFloboGameFactory)
  : attachedThemeSet(NULL), attachedLevelTheme(NULL),
    attachedFloboFactory(this), delayBeforeGameOver(60), haveDisplay(false),
    neutralXOffset(-1), neutralYOffset(-1)
{
    initCommon(attachedFloboGameFactory);
}

PuyoView::~PuyoView()
{
    delete attachedGame;
}

void PuyoView::setEnemyGame(FloboGame *enemyGame)
{
	this->enemyGame = enemyGame;
}

int PuyoView::getValenceForPuyo(Flobo *puyo) const
{
    if (!haveDisplay) return 0;
    int i = puyo->getFloboX();
    int j = puyo->getFloboY();
    FloboState currentFloboState = puyo->getFloboState();
    AnimatedPuyo *down  = (AnimatedPuyo *)(attachedGame->getFloboAt(i, j+1));
    AnimatedPuyo *right = (AnimatedPuyo *)(attachedGame->getFloboAt(i+1, j));
    AnimatedPuyo *up    = (AnimatedPuyo *)(attachedGame->getFloboAt(i, j-1));
    AnimatedPuyo *left  = (AnimatedPuyo *)(attachedGame->getFloboAt(i-1, j));

    FloboState downState = (down == NULL) || (down->isRenderingAnimation()) ? FLOBO_EMPTY : down->getFloboState();
    FloboState rightState = (right == NULL) || (right->isRenderingAnimation()) ? FLOBO_EMPTY : right->getFloboState();
    FloboState upState = (up == NULL) || (up->isRenderingAnimation()) ? FLOBO_EMPTY : up->getFloboState();
    FloboState leftState = (left == NULL)   || (left->isRenderingAnimation()) ? FLOBO_EMPTY : left->getFloboState();

	return (leftState  == currentFloboState ? 0x8 : 0) | (upState  == currentFloboState ? 0x4 : 0) |
	       (rightState == currentFloboState ? 0x2 : 0) | (downState == currentFloboState ? 0x1 : 0);
}

bool PuyoView::isGameOver() const
{
    if ((!gameRunning) && (delayBeforeGameOver < 0))
        return true;
    return false;
}

void PuyoView::cycleAnimation(void)
{
    if (haveDisplay) {
        // Handle end of game
        if (!gameRunning) {
            delayBeforeGameOver--;
        }

        // Cycling every puyo's animation
        for (int i = 0, j = attachedGame->getFloboCount() ; i < j ; i++) {
            AnimatedPuyo *currentPuyo =
              (AnimatedPuyo *)(attachedGame->getFloboAtIndex(i));
            currentPuyo->cycleAnimation();
        }
        // Cycling dead puyo's animations
        attachedFloboFactory.cycleWalhalla();

        // Cycling view's animations
        if (viewAnimations.size() > 0) {
            Animation *currentAnimation = viewAnimations[0];
            if (currentAnimation->isFinished()) {
                viewAnimations.removeKeepOrder(currentAnimation);
                delete currentAnimation;
            }
            else {
                currentAnimation->cycle();
            }
        }
    }

    // If there is a skipped cycle to do, do it
    if (attachedGame->isEndOfCycle() && attachedGame->isGameRunning() && !newMetaCycleStart) cycleGame();
}

void PuyoView::cycleGame()
{
    // If we are not allowed to cycle the game, mark it
    if (cycleAllowed()) {
        skippedCycle = false;
        attachedGame->cycle();
    }
    else {
        skippedCycle = true;
    }
}

void PuyoView::moveLeft()
{
    if (cycleAllowed()) attachedGame->moveLeft();
}

void PuyoView::moveRight()
{
    if (cycleAllowed()) attachedGame->moveRight();
}

void PuyoView::rotateLeft()
{
    if (cycleAllowed()) attachedGame->rotate(true);
}

void PuyoView::rotateRight()
{
    if (cycleAllowed()) attachedGame->rotate(false);
}


void PuyoView::render(DrawTarget *dt)
{
    if (!haveDisplay) return;
	IosRect drect;
    IosRect vrect;
	vrect.x = m_xOffset;
	vrect.y = m_yOffset;
	vrect.w = TSIZE * FLOBOBAN_DIMX;
	vrect.h = TSIZE * FLOBOBAN_DIMY;

    bool displayFallings = this->cycleAllowed();
    // Render shadows
    if (m_showShadows) {
        for (int i = 0, j = attachedGame->getFloboCount() ; i < j ; i++) {
            AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getFloboAtIndex(i));
            if (displayFallings || !currentPuyo->isFalling()) currentPuyo->renderShadow(dt);
        }
    }
    // Render flobos
 	for (int i = 0, j = attachedGame->getFloboCount() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getFloboAtIndex(i));
        if (displayFallings || !currentPuyo->isFalling()) currentPuyo->render(dt);
    }
    // drawing the walhalla
    attachedFloboFactory.renderWalhalla(dt);

    if (m_showNextPuyos) {
        drect.x = m_nXOffset;
        drect.y = m_nYOffset;
        drect.w = TSIZE;
        drect.h = TSIZE * 2;
        // Drawing next flobos
        const PuyoTheme &nextPuyoTheme =
            attachedThemeSet->getPuyoTheme(attachedGame->getNextFalling());
        IosSurface *currentSurface = nextPuyoTheme.getPuyoSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset + TSIZE;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            dt->draw(nextPuyoTheme.getEyeSurfaceForIndex(0), NULL, &drect);
        }
        const PuyoTheme &nextCompanionTheme =
            attachedThemeSet->getPuyoTheme(attachedGame->getNextCompanion());
        currentSurface = nextCompanionTheme.getPuyoSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            dt->draw(nextCompanionTheme.getEyeSurfaceForIndex(0), NULL, &drect);
        }
    }

    // Drawing the view animation
    if (viewAnimations.size() > 0) {
        Animation *currentAnimation = viewAnimations[0];
        if (!currentAnimation->isFinished()) {
            currentAnimation->draw(0, dt);
        }
    }
}

void PuyoView::renderNeutral(DrawTarget *dt)
{
    if (!haveDisplay) return;
	IosRect drect;
    int neutralFlobos = attachedGame->getNeutralFlobos();
    int numGiantNeutral = (neutralFlobos / FLOBOBAN_DIMX) / 4;
    int numBigNeutral = (neutralFlobos / FLOBOBAN_DIMX) % 4;
	int numNeutral = neutralFlobos % FLOBOBAN_DIMX;
    int drect_x = (neutralXOffset == -1 ? m_xOffset : neutralXOffset);
    int drect_y_base =  (neutralYOffset == -1 ?
                         m_yOffset + 3 + TSIZE + TSIZE : neutralYOffset);
    IosSurface *neutral = attachedLevelTheme->getNeutralIndicator();
    IosSurface *bigNeutral = attachedLevelTheme->getBigNeutralIndicator();
    IosSurface *giantNeutral = attachedLevelTheme->getGiantNeutralIndicator();

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

void PuyoView::renderScore(DrawTarget *dt)
{
    m_scoreDisplay->draw(dt);
}

void PuyoView::gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral) {
    if (!haveDisplay) return;
    int x = neutralFlobo->getFloboX();
    int y = neutralFlobo->getFloboY();
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    ((AnimatedPuyo *)neutralFlobo)->addAnimation(new NeutralAnimation(*((AnimatedPuyo *)neutralFlobo), neutralIndex * 2, synchronizer));
    for (int i = y ; i < FLOBOBAN_DIMY ; i++) {
        AnimatedPuyo *belowPuyo = (AnimatedPuyo *)(attachedGame->getFloboAt(x, i));
        if (belowPuyo != NULL) {
            belowPuyo->addAnimation(new SmoothBounceAnimation(*belowPuyo, synchronizer));
        }
    }
    if ((neutralIndex == 2) && (totalNeutral > 4)) {
        if (totalNeutral > 20)
            totalNeutral = 20;
        int duration = totalNeutral * 3;
        int shakeCount = totalNeutral / 4.f;
        float amplY = totalNeutral * 2.f;
        Animation *shakingAnimation = new ScreenShakingAnimation(duration, shakeCount, 0.f, amplY, 1.0f, synchronizer);
        viewAnimations.add(shakingAnimation);
    }
}

void PuyoView::fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingFlobo)->flushAnimations(ANIMATION_H);
    ((AnimatedPuyo *)companionFlobo)->flushAnimations(ANIMATION_H);
	((AnimatedPuyo *)fallingFlobo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)fallingFlobo, TSIZE, 4));
    ((AnimatedPuyo *)companionFlobo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)companionFlobo, TSIZE, 4));
}

void PuyoView::fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingFlobo)->flushAnimations(ANIMATION_H);
    ((AnimatedPuyo *)companionFlobo)->flushAnimations(ANIMATION_H);
	((AnimatedPuyo *)fallingFlobo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)fallingFlobo, -TSIZE, 4));
    ((AnimatedPuyo *)companionFlobo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)companionFlobo, -TSIZE, 4));
}

void PuyoView::fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingFlobo)->flushAnimations(ANIMATION_V);
    ((AnimatedPuyo *)companionFlobo)->flushAnimations(ANIMATION_V);
	((AnimatedPuyo *)fallingFlobo)->addAnimation(new MovingVAnimation(*(AnimatedPuyo *)fallingFlobo, -TSIZE/2, 4));
    ((AnimatedPuyo *)companionFlobo)->addAnimation(new MovingVAnimation(*(AnimatedPuyo *)companionFlobo, -TSIZE/2, 4));
}

void PuyoView::companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise)
{
    if (!haveDisplay) return;
    if ((companionFlobo != NULL) && (fallingFlobo != NULL)) { // Just to be sure of what we get if data comes from network
        ((AnimatedPuyo *)companionFlobo)->setPartner(((AnimatedPuyo *)fallingFlobo));
        ((AnimatedPuyo *)companionFlobo)->flushAnimations(ANIMATION_ROTATE);
        ((AnimatedPuyo *)companionFlobo)->addAnimation(new TurningAnimation(*(AnimatedPuyo *)companionFlobo,
                                                                           counterclockwise));
    }
}

void PuyoView::floboDidFall(Flobo *puyo, int originX, int originY, int nFalledBelow)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)puyo)->flushAnimations();
    ((AnimatedPuyo *)puyo)->addAnimation(new FallingAnimation(*(AnimatedPuyo *)puyo, originY, m_xOffset, m_yOffset, nFalledBelow));
}

void PuyoView::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    if (!haveDisplay) return;
    double groupPadding = 0.;
    // Exploding puyo animation
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    for (int i = 0, j = floboGroup.size() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(floboGroup[i]);
        PuyoAnimation *newAnimation;
        if (currentPuyo->getFloboState() != FLOBO_NEUTRAL)
            newAnimation = new VanishAnimation(*currentPuyo, i*2 , m_xOffset, m_yOffset, synchronizer, i, floboGroup.size(), groupNum, phase);
        else
            newAnimation = new NeutralPopAnimation(*currentPuyo, i*2, synchronizer);
        currentPuyo->addAnimation(newAnimation);
        // Compute the center of the vanishing flobos padding
        groupPadding += newAnimation->getPuyoSoundPadding();
    }
    viewAnimations.add(new VanishSoundAnimation(phase, synchronizer, groupPadding / floboGroup.size()));
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
      if (index>1) AudioManager::playSound(sound_yahoohoo[index], volume_yahoohoo[index]);
    }

}

void PuyoView::gameDidEndCycle()
{
	if (enemyGame != NULL) {
		if (attachedGame->getNeutralFlobos() < 0)
			enemyGame->increaseNeutralFlobos(- attachedGame->getNeutralFlobos());
	}
    GTLogTrace("NewMetaCycleStart");
    newMetaCycleStart = true;
}

bool PuyoView::cycleAllowed()
{
    if (enemyGame != NULL)
        if (!enemyGame->isGameRunning())
            return false;
    if (cycleAllowance < 0)
        return false;
    return true;
}

void PuyoView::gameWin()
{
    attachedGame->getGameStat().is_winner = true;
    gameRunning = false;
}

void PuyoView::gameLost()
{
    attachedGame->getGameStat().is_winner = false;
    gameRunning = false;
    if (!haveDisplay) return;
    for (int i = 0 ; i <= FLOBOBAN_DIMX ; i++) {
        for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            if (attachedGame->getFloboAt(i, j) != NULL) {
                AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(attachedGame->getFloboAt(i, j));
                currentPuyo->addAnimation(new GameOverFallAnimation(*currentPuyo, (j - FLOBOBAN_DIMY) + abs((FLOBOBAN_DIMX / 2) - i) * 5));
            }
        }
    }
    Animation *shakingAnimation = new ScreenShakingAnimation(80, 12, 10.f, 5.f, 1.f);
    viewAnimations.add(shakingAnimation);
    AudioManager::playSound("earthquake.wav", 1.0);
}

