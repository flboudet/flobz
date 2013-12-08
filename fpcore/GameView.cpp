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
  : m_playerId(playerId), m_showNextFlobos(true), m_showShadows(true),
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

void GameView::setupLayout(int playerId)
{
    setPlayerId(playerId);
    setPosition(attachedLevelTheme->getFlobobanX(playerId),
                attachedLevelTheme->getFlobobanY(playerId));
    setNextFlobosPosition(attachedLevelTheme->getNextFlobosX(playerId),
                         attachedLevelTheme->getNextFlobosY(playerId));
    setNeutralFlobosDisplayPosition(attachedLevelTheme->getNeutralDisplayX(playerId),
                                   attachedLevelTheme->getNeutralDisplayY(playerId));
    setShowNextFlobos(attachedLevelTheme->getShouldDisplayNext(playerId));
    setShowShadows(attachedLevelTheme->getShouldDisplayShadows(playerId));
    setShowEyes(attachedLevelTheme->getShouldDisplayEyes(playerId));
}

void GameView::initCommon(FloboGameFactory *attachedFloboGameFactory)
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

GameView::GameView(FloboGameFactory *attachedFloboGameFactory)
  : attachedThemeSet(NULL), attachedLevelTheme(NULL),
    attachedFloboFactory(this), delayBeforeGameOver(60), haveDisplay(false),
    neutralXOffset(-1), neutralYOffset(-1)
{
    initCommon(attachedFloboGameFactory);
}

GameView::~GameView()
{
    delete attachedGame;
}

void GameView::setEnemyGame(FloboGame *enemyGame)
{
	this->enemyGame = enemyGame;
}

int GameView::getValenceForFlobo(Flobo *flobo) const
{
    if (!haveDisplay) return 0;
    int i = flobo->getFloboX();
    int j = flobo->getFloboY();
    FloboState currentFloboState = flobo->getFloboState();
    AnimatedFlobo *down  = (AnimatedFlobo *)(attachedGame->getFloboAt(i, j+1));
    AnimatedFlobo *right = (AnimatedFlobo *)(attachedGame->getFloboAt(i+1, j));
    AnimatedFlobo *up    = (AnimatedFlobo *)(attachedGame->getFloboAt(i, j-1));
    AnimatedFlobo *left  = (AnimatedFlobo *)(attachedGame->getFloboAt(i-1, j));

    FloboState downState = (down == NULL) || (down->isRenderingAnimation()) ? FLOBO_EMPTY : down->getFloboState();
    FloboState rightState = (right == NULL) || (right->isRenderingAnimation()) ? FLOBO_EMPTY : right->getFloboState();
    FloboState upState = (up == NULL) || (up->isRenderingAnimation()) ? FLOBO_EMPTY : up->getFloboState();
    FloboState leftState = (left == NULL)   || (left->isRenderingAnimation()) ? FLOBO_EMPTY : left->getFloboState();

	return (leftState  == currentFloboState ? 0x8 : 0) | (upState  == currentFloboState ? 0x4 : 0) |
	       (rightState == currentFloboState ? 0x2 : 0) | (downState == currentFloboState ? 0x1 : 0);
}

bool GameView::isGameOver() const
{
    if ((!gameRunning) && (delayBeforeGameOver < 0))
        return true;
    return false;
}

void GameView::cycleAnimation(void)
{
    if (haveDisplay) {
        // Handle end of game
        if (!gameRunning) {
            delayBeforeGameOver--;
        }

        // Cycling every flobo's animation
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            static_cast<AnimatedFlobo *>(iter.get())->cycleAnimation();
        }
        // Cycling dead flobo's animations
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

void GameView::cycleGame()
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
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get());
            if (displayFallings || !currentFlobo->isFalling()) currentFlobo->renderShadow(dt);
        }
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get());
            if (displayFallings || !currentFlobo->isFalling()) currentFlobo->renderShadow(dt);
        }
    }
    // Render flobos
    for (FloboDefaultIterator iter(attachedGame) ;
         ! iter.end() ; ++iter) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(iter.get());
        if (displayFallings || !currentFlobo->isFalling()) currentFlobo->render(dt);
    }
    // drawing the walhalla
    attachedFloboFactory.renderWalhalla(dt);

    if (m_showNextFlobos) {
        drect.x = m_nXOffset;
        drect.y = m_nYOffset;
        drect.w = TSIZE;
        drect.h = TSIZE * 2;
        // Drawing next flobos
        const FloboTheme &nextFloboTheme =
            attachedThemeSet->getFloboTheme(attachedGame->getNextFalling());
        IosSurface *currentSurface = nextFloboTheme.getFloboSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset + TSIZE;
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
            attachedThemeSet->getFloboTheme(attachedGame->getNextCompanion());
        currentSurface = nextCompanionTheme.getFloboSurfaceForValence(0);
        if (currentSurface != NULL) {
            drect.x = m_nXOffset;
            drect.y = m_nYOffset;
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
    if (viewAnimations.size() > 0) {
        Animation *currentAnimation = viewAnimations[0];
        if (!currentAnimation->isFinished()) {
            currentAnimation->draw(0, dt);
        }
    }
}

void GameView::renderNeutral(DrawTarget *dt)
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
    if (neutral == NULL)
        throw Exception(String("Neutral indicator not found!"));
    if (bigNeutral == NULL)
        throw Exception(String("Big neutral indicator not found!"));
    if (giantNeutral == NULL)
        throw Exception(String("Giant neutral indicator not found!"));
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
    m_scoreDisplay->draw(dt);
}

void GameView::gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral) {
    if (!haveDisplay) return;
    int x = neutralFlobo->getFloboX();
    int y = neutralFlobo->getFloboY();
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    ((AnimatedFlobo *)neutralFlobo)->addAnimation(new NeutralAnimation(*((AnimatedFlobo *)neutralFlobo), neutralIndex * 2, synchronizer));
    for (int i = y ; i < FLOBOBAN_DIMY ; i++) {
        AnimatedFlobo *belowFlobo = (AnimatedFlobo *)(attachedGame->getFloboAt(x, i));
        if (belowFlobo != NULL) {
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
        viewAnimations.add(shakingAnimation);
    }
}

void GameView::fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedFlobo *)fallingFlobo)->flushAnimations(ANIMATION_H);
    ((AnimatedFlobo *)companionFlobo)->flushAnimations(ANIMATION_H);
	((AnimatedFlobo *)fallingFlobo)->addAnimation(new MovingHAnimation(*(AnimatedFlobo *)fallingFlobo, TSIZE, 4));
    ((AnimatedFlobo *)companionFlobo)->addAnimation(new MovingHAnimation(*(AnimatedFlobo *)companionFlobo, TSIZE, 4));
}

void GameView::fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedFlobo *)fallingFlobo)->flushAnimations(ANIMATION_H);
    ((AnimatedFlobo *)companionFlobo)->flushAnimations(ANIMATION_H);
	((AnimatedFlobo *)fallingFlobo)->addAnimation(new MovingHAnimation(*(AnimatedFlobo *)fallingFlobo, -TSIZE, 4));
    ((AnimatedFlobo *)companionFlobo)->addAnimation(new MovingHAnimation(*(AnimatedFlobo *)companionFlobo, -TSIZE, 4));
}

void GameView::fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    if (!haveDisplay) return;
    ((AnimatedFlobo *)fallingFlobo)->flushAnimations(ANIMATION_V);
    ((AnimatedFlobo *)companionFlobo)->flushAnimations(ANIMATION_V);
	((AnimatedFlobo *)fallingFlobo)->addAnimation(new MovingVAnimation(*(AnimatedFlobo *)fallingFlobo, -TSIZE/2, 4));
    ((AnimatedFlobo *)companionFlobo)->addAnimation(new MovingVAnimation(*(AnimatedFlobo *)companionFlobo, -TSIZE/2, 4));
}

void GameView::companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise)
{
    if (!haveDisplay) return;
    if ((companionFlobo != NULL) && (fallingFlobo != NULL)) { // Just to be sure of what we get if data comes from network
        ((AnimatedFlobo *)companionFlobo)->setPartner(((AnimatedFlobo *)fallingFlobo));
        ((AnimatedFlobo *)companionFlobo)->flushAnimations(ANIMATION_ROTATE);
        ((AnimatedFlobo *)companionFlobo)->addAnimation(new TurningAnimation(*(AnimatedFlobo *)companionFlobo,
                                                                           counterclockwise));
    }
}

void GameView::floboDidFall(Flobo *flobo, int originX, int originY, int nFalledBelow)
{
    if (!haveDisplay) return;
    ((AnimatedFlobo *)flobo)->flushAnimations();
    ((AnimatedFlobo *)flobo)->addAnimation(new FallingAnimation(*(AnimatedFlobo *)flobo, originY, m_xOffset, m_yOffset, nFalledBelow));
}

void GameView::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    if (!haveDisplay) return;
    double groupPadding = 0.;
    // Exploding flobo animation
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    for (int i = 0, j = floboGroup.size() ; i < j ; i++) {
        AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(floboGroup[i]);
        FloboAnimation *newAnimation;
        if (currentFlobo->getFloboState() != FLOBO_NEUTRAL)
            newAnimation = new VanishAnimation(*currentFlobo, i*2 , m_xOffset, m_yOffset, synchronizer, i, floboGroup.size(), groupNum, phase);
        else
            newAnimation = new NeutralPopAnimation(*currentFlobo, i*2, synchronizer);
        currentFlobo->addAnimation(newAnimation);
        // Compute the center of the vanishing flobos padding
        groupPadding += newAnimation->getSoundPadding();
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
      if (index>1)
          theCommander->playSound(sound_yahoohoo[index], volume_yahoohoo[index]);
    }

}

void GameView::gameDidEndCycle()
{
	if (enemyGame != NULL) {
		if (attachedGame->getNeutralFlobos() < 0)
			enemyGame->increaseNeutralFlobos(- attachedGame->getNeutralFlobos());
	}
    GTLogTrace("NewMetaCycleStart");
    newMetaCycleStart = true;
}

bool GameView::cycleAllowed()
{
    if (enemyGame != NULL)
        if (!enemyGame->isGameRunning())
            return false;
    if (cycleAllowance < 0)
        return false;
    return true;
}

void GameView::gameWin()
{
    attachedGame->getGameStat().is_winner = true;
    gameRunning = false;
}

void GameView::gameLost()
{
    attachedGame->getGameStat().is_winner = false;
    gameRunning = false;
    if (!haveDisplay) return;
    for (int i = 0 ; i <= FLOBOBAN_DIMX ; i++) {
        for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            if (attachedGame->getFloboAt(i, j) != NULL) {
                AnimatedFlobo *currentFlobo = static_cast<AnimatedFlobo *>(attachedGame->getFloboAt(i, j));
                currentFlobo->addAnimation(new GameOverFallAnimation(*currentFlobo, (j - FLOBOBAN_DIMY) + abs((FLOBOBAN_DIMX / 2) - i) * 5));
            }
        }
    }
    Animation *shakingAnimation = new ScreenShakingAnimation(80, 12, 10.f, 5.f, 1.f);
    viewAnimations.add(shakingAnimation);
    theCommander->playSound("earthquake.wav", 1.0);
}

