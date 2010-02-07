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
#include "PuyoView.h"
#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "PuyoGame.h"
#include "audio.h"
#include "HiScores.h"

PuyoView::PuyoView(PuyoGameFactory *attachedPuyoGameFactory,
                   int playerId,
                   AnimatedPuyoSetTheme *attachedThemeSet,
                   PuyoLevelTheme *attachedLevelTheme)
  : m_playerId(playerId), m_showNextPuyos(true), m_showShadows(true),
    attachedThemeSet(attachedThemeSet), attachedLevelTheme(attachedLevelTheme),
    attachedPuyoFactory(this), delayBeforeGameOver(60), haveDisplay(true),
    m_xOffset(0), m_yOffset(- TSIZE), m_nXOffset(0), m_nYOffset(0),
    neutralXOffset(-1), neutralYOffset(-1)
{
    setupLayout(playerId);
    initCommon(attachedPuyoGameFactory);
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
    setNeutralPuyosDisplayPosition(attachedLevelTheme->getNeutralDisplayX(playerId),
                                   attachedLevelTheme->getNeutralDisplayY(playerId));
    setShowNextPuyos(attachedLevelTheme->getShouldDisplayNext(playerId));
    setShowShadows(attachedLevelTheme->getShouldDisplayShadows(playerId));
    setShowEyes(attachedLevelTheme->getShouldDisplayEyes(playerId));
}

void PuyoView::initCommon(PuyoGameFactory *attachedPuyoGameFactory)
{
	attachedGame = attachedPuyoGameFactory->createPuyoGame(&attachedPuyoFactory);
    m_scoreDisplay.reset(new PlayerGameStatDisplay(attachedGame->getGameStat()));
    attachedGame->setDelegate(this);
	gameRunning = true;
	enemyGame = NULL;
    skippedCycle = false;
    cycleAllowance = 0;
    newMetaCycleStart = false;
}

PuyoView::PuyoView(PuyoGameFactory *attachedPuyoGameFactory)
  : attachedThemeSet(NULL), attachedLevelTheme(NULL),
    attachedPuyoFactory(this), delayBeforeGameOver(60), haveDisplay(false),
    neutralXOffset(-1), neutralYOffset(-1)
{
    m_scoreDisplay.reset(new PlayerGameStatDisplay(attachedGame->getGameStat()));
    initCommon(attachedPuyoGameFactory);
}

PuyoView::~PuyoView()
{
    delete attachedGame;
}

void PuyoView::setEnemyGame(PuyoGame *enemyGame)
{
	this->enemyGame = enemyGame;
}

int PuyoView::getValenceForPuyo(PuyoPuyo *puyo) const
{
    if (!haveDisplay) return 0;
    int i = puyo->getPuyoX();
    int j = puyo->getPuyoY();
    PuyoState currentPuyoState = puyo->getPuyoState();
    AnimatedPuyo *down  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j+1));
    AnimatedPuyo *right = (AnimatedPuyo *)(attachedGame->getPuyoAt(i+1, j));
    AnimatedPuyo *up    = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j-1));
    AnimatedPuyo *left  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i-1, j));

    PuyoState downState = (down == NULL) || (down->isRenderingAnimation()) ? PUYO_EMPTY : down->getPuyoState();
    PuyoState rightState = (right == NULL) || (right->isRenderingAnimation()) ? PUYO_EMPTY : right->getPuyoState();
    PuyoState upState = (up == NULL) || (up->isRenderingAnimation()) ? PUYO_EMPTY : up->getPuyoState();
    PuyoState leftState = (left == NULL)   || (left->isRenderingAnimation()) ? PUYO_EMPTY : left->getPuyoState();

	return (leftState  == currentPuyoState ? 0x8 : 0) | (upState  == currentPuyoState ? 0x4 : 0) |
	       (rightState == currentPuyoState ? 0x2 : 0) | (downState == currentPuyoState ? 0x1 : 0);
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
        for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
            AnimatedPuyo *currentPuyo =
              (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
            currentPuyo->cycleAnimation();
        }
        // Cycling dead puyo's animations
        attachedPuyoFactory.cycleWalhalla();

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
	vrect.w = TSIZE * PUYODIMX;
	vrect.h = TSIZE * PUYODIMY;

    bool displayFallings = this->cycleAllowed();
    // Render shadows
    if (m_showShadows) {
        for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
            AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
            if (displayFallings || !currentPuyo->isFalling()) currentPuyo->renderShadow(dt);
        }
    }
    // Render puyos
 	for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
        if (displayFallings || !currentPuyo->isFalling()) currentPuyo->render(dt);
    }
    // drawing the walhalla
    attachedPuyoFactory.renderWalhalla(dt);

    if (m_showNextPuyos) {
        drect.x = m_nXOffset;
        drect.y = m_nYOffset;
        drect.w = TSIZE;
        drect.h = TSIZE * 2;
        // Drawing next puyos
        AnimatedPuyoTheme *nextPuyoTheme =
            attachedThemeSet->getAnimatedPuyoTheme(attachedGame->getNextFalling());
        IosSurface *currentSurface = nextPuyoTheme->getPuyoSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset + TSIZE;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            dt->draw(nextPuyoTheme->getEyeSurfaceForIndex(0), NULL, &drect);
        }
        AnimatedPuyoTheme *nextCompanionTheme =
            attachedThemeSet->getAnimatedPuyoTheme(attachedGame->getNextCompanion());
        currentSurface = nextCompanionTheme->getPuyoSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset;
            drect.w = currentSurface->w;
            drect.h = currentSurface->h;
            dt->draw(currentSurface, NULL, &drect);
            dt->draw(nextCompanionTheme->getEyeSurfaceForIndex(0), NULL, &drect);
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
    int neutralPuyos = attachedGame->getNeutralPuyos();
    int numGiantNeutral = (neutralPuyos / PUYODIMX) / 4;
    int numBigNeutral = (neutralPuyos / PUYODIMX) % 4;
	int numNeutral = neutralPuyos % PUYODIMX;
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

void PuyoView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex) {
    if (!haveDisplay) return;
    int x = neutralPuyo->getPuyoX();
    int y = neutralPuyo->getPuyoY();
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    ((AnimatedPuyo *)neutralPuyo)->addAnimation(new NeutralAnimation(*((AnimatedPuyo *)neutralPuyo), neutralIndex * 4, synchronizer));
    for (int i = y ; i < PUYODIMY ; i++) {
        AnimatedPuyo *belowPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAt(x, i));
        if (belowPuyo != NULL) {
            belowPuyo->addAnimation(new SmoothBounceAnimation(*belowPuyo, synchronizer));
        }
    }
}

void PuyoView::fallingsDidMoveLeft(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingPuyo)->flushAnimations(ANIMATION_H);
    ((AnimatedPuyo *)companionPuyo)->flushAnimations(ANIMATION_H);
	((AnimatedPuyo *)fallingPuyo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)fallingPuyo, TSIZE, 4));
    ((AnimatedPuyo *)companionPuyo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)companionPuyo, TSIZE, 4));
}

void PuyoView::fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingPuyo)->flushAnimations(ANIMATION_H);
    ((AnimatedPuyo *)companionPuyo)->flushAnimations(ANIMATION_H);
	((AnimatedPuyo *)fallingPuyo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)fallingPuyo, -TSIZE, 4));
    ((AnimatedPuyo *)companionPuyo)->addAnimation(new MovingHAnimation(*(AnimatedPuyo *)companionPuyo, -TSIZE, 4));
}

void PuyoView::fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)fallingPuyo)->flushAnimations(ANIMATION_V);
    ((AnimatedPuyo *)companionPuyo)->flushAnimations(ANIMATION_V);
	((AnimatedPuyo *)fallingPuyo)->addAnimation(new MovingVAnimation(*(AnimatedPuyo *)fallingPuyo, -TSIZE/2, 4));
    ((AnimatedPuyo *)companionPuyo)->addAnimation(new MovingVAnimation(*(AnimatedPuyo *)companionPuyo, -TSIZE/2, 4));
}

void PuyoView::companionDidTurn(PuyoPuyo *companionPuyo, PuyoPuyo *fallingPuyo, bool counterclockwise)
{
    if (!haveDisplay) return;
    if ((companionPuyo != NULL) && (fallingPuyo != NULL)) { // Just to be sure of what we get if data comes from network
        ((AnimatedPuyo *)companionPuyo)->setPartner(((AnimatedPuyo *)fallingPuyo));
        ((AnimatedPuyo *)companionPuyo)->flushAnimations(ANIMATION_ROTATE);
        ((AnimatedPuyo *)companionPuyo)->addAnimation(new TurningAnimation(*(AnimatedPuyo *)companionPuyo,
                                                                           counterclockwise));
    }
}

void PuyoView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow)
{
    if (!haveDisplay) return;
    ((AnimatedPuyo *)puyo)->flushAnimations();
    ((AnimatedPuyo *)puyo)->addAnimation(new FallingAnimation(*(AnimatedPuyo *)puyo, originY, m_xOffset, m_yOffset, nFalledBelow));
}

void PuyoView::puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase)
{
    if (!haveDisplay) return;
    double groupPadding = 0.;
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    for (int i = 0, j = puyoGroup.size() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(puyoGroup[i]);
        PuyoAnimation *newAnimation;
        if (currentPuyo->getPuyoState() != PUYO_NEUTRAL)
            newAnimation = new VanishAnimation(*currentPuyo, i*2 , m_xOffset, m_yOffset, synchronizer, i, puyoGroup.size(), groupNum, phase);
        else
            newAnimation = new NeutralPopAnimation(*currentPuyo, i*2, synchronizer);
        currentPuyo->addAnimation(newAnimation);
        // Compute the center of the vanishing puyos padding
        groupPadding += newAnimation->getPuyoSoundPadding();
    }
    viewAnimations.add(new VanishSoundAnimation(phase, synchronizer, groupPadding / puyoGroup.size()));

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
		if (attachedGame->getNeutralPuyos() < 0)
			enemyGame->increaseNeutralPuyos(- attachedGame->getNeutralPuyos());
	}
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
    for (int i = 0 ; i <= PUYODIMX ; i++) {
        for (int j = 0 ; j <= PUYODIMY ; j++) {
            if (attachedGame->getPuyoAt(i, j) != NULL) {
                AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(attachedGame->getPuyoAt(i, j));
                currentPuyo->addAnimation(new GameOverFallAnimation(*currentPuyo, (j - PUYODIMY) + abs((PUYODIMX / 2) - i) * 5));
            }
        }
    }
}




