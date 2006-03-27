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
#include "IosImgProcess.h"
#include "HiScores.h"
#include "PuyoDoomMelt.h"

#include "SDL_Painter.h"

//IIM_Surface *neutral = NULL;
//IIM_Surface *bigNeutral = NULL;

PuyoView::PuyoView(PuyoGameFactory *attachedPuyoGameFactory,
		   AnimatedPuyoSetTheme *attachedThemeSet,
           PuyoLevelTheme *attachedLevelTheme,
		   int xOffset, int yOffset, int nXOffset, int nYOffset, SDL_Painter &painterToUse)
  :attachedThemeSet(attachedThemeSet), attachedLevelTheme(attachedLevelTheme),
   attachedPuyoFactory(this), attachedPainter(painterToUse), delayBeforeGameOver(60)
{
    //printf("Constructeur du PuyoView\n");
	attachedGame = attachedPuyoGameFactory->createPuyoGame(&attachedPuyoFactory);
    attachedGame->setDelegate(this);
    
	this->xOffset = xOffset;
	this->yOffset = yOffset - TSIZE;
	this->nXOffset = nXOffset;
	this->nYOffset = nYOffset;
	gameRunning = true;
	enemyGame = NULL;
    skippedCycle = false;
    cycleAllowance = 0;
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
    int i = puyo->getPuyoX();
    int j = puyo->getPuyoY();
    PuyoState currentPuyoState = puyo->getPuyoState();
    AnimatedPuyo *down  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j+1));
    AnimatedPuyo *right = (AnimatedPuyo *)(attachedGame->getPuyoAt(i+1, j));
    AnimatedPuyo *up    = (AnimatedPuyo *)(attachedGame->getPuyoAt(i, j-1));
    AnimatedPuyo *left  = (AnimatedPuyo *)(attachedGame->getPuyoAt(i-1, j));
    
    PuyoState downState = (down == NULL)   || (down->isRenderingAnimation())  ? PUYO_EMPTY  : down->getPuyoState();
    PuyoState rightState = (right == NULL) || (right->isRenderingAnimation()) ? PUYO_EMPTY  : right->getPuyoState();
    PuyoState upState = (up == NULL)       || (up->isRenderingAnimation())    ? PUYO_EMPTY  : up->getPuyoState();
    PuyoState leftState = (left == NULL)   || (left->isRenderingAnimation())  ? PUYO_EMPTY  : left->getPuyoState();
    
	return (leftState  == currentPuyoState ? 0x8 : 0) | (upState   == currentPuyoState ? 0x4 : 0) |
	       (rightState == currentPuyoState ? 0x2 : 0) | (downState == currentPuyoState ? 0x1 : 0);
}

bool PuyoView::isGameOver() const
{
    if ((!gameRunning) && (delayBeforeGameOver < 0))
        return true;
    return false;
}

void PuyoView::cycleAnimation()
{
    // Handle end of game
    if (!gameRunning)
        delayBeforeGameOver--;
    
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

    // If there is a skipped cycle to do, do it
    if ((skippedCycle || attachedGame->isEndOfCycle()) && attachedGame->isGameRunning() && cycleAllowed()) {
        attachedGame->cycle();
        skippedCycle = false;
    }
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


void PuyoView::render()
{
    
	SDL_Rect drect;
    SDL_Rect vrect;
	vrect.x = xOffset;
	vrect.y = yOffset;
	vrect.w = TSIZE * PUYODIMX;
	vrect.h = TSIZE * PUYODIMY;
        
        bool displayFallings = this->cycleAllowed();

    for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
        if (displayFallings || !currentPuyo->isFalling()) currentPuyo->renderShadow();
    }
 	for (int i = 0, j = attachedGame->getPuyoCount() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = (AnimatedPuyo *)(attachedGame->getPuyoAtIndex(i));
        if (displayFallings || !currentPuyo->isFalling()) currentPuyo->render();
    }
    // drawing the walhalla
    attachedPuyoFactory.renderWalhalla();
    
	drect.x = nXOffset;
	drect.y = nYOffset;
	drect.w = TSIZE;
	drect.h = TSIZE * 2;
	// Drawing next puyos
	AnimatedPuyoTheme *nextPuyoTheme =
	  attachedThemeSet->getAnimatedPuyoTheme(attachedGame->getNextFalling());
	IIM_Surface *currentSurface = nextPuyoTheme->getPuyoSurfaceForValence(0);
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset + TSIZE;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		attachedPainter.requestDraw(currentSurface, &drect);
		attachedPainter.requestDraw(nextPuyoTheme->getEyeSurfaceForIndex(0), &drect);
	}
	AnimatedPuyoTheme *nextCompanionTheme =
	  attachedThemeSet->getAnimatedPuyoTheme(attachedGame->getNextCompanion());
	currentSurface = nextCompanionTheme->getPuyoSurfaceForValence(0);
	if (currentSurface != NULL) {
		drect.x = nXOffset;
		drect.y = nYOffset;
		drect.w = currentSurface->w;
		drect.h = currentSurface->h;
		attachedPainter.requestDraw(currentSurface, &drect);
		attachedPainter.requestDraw(nextCompanionTheme->getEyeSurfaceForIndex(0), &drect);
	}
    
    // Drawing the view animation
    if (viewAnimations.size() > 0) {
        Animation *currentAnimation = viewAnimations[0];
        if (!currentAnimation->isFinished()) {
            currentAnimation->draw(0);
        }
    }
}

void PuyoView::renderNeutral()
{
	SDL_Rect drect;
    int neutralPuyos = attachedGame->getNeutralPuyos();
    int numGiantNeutral = (neutralPuyos / PUYODIMX) / 4;
    int numBigNeutral = (neutralPuyos / PUYODIMX) % 4;
	int numNeutral = neutralPuyos % PUYODIMX;
    int drect_x = xOffset;
    int drect_y_base =  yOffset + 3 + TSIZE + TSIZE;
    IIM_Surface *neutral = attachedLevelTheme->getNeutralIndicator();
    IIM_Surface *bigNeutral = attachedLevelTheme->getBigNeutralIndicator();
    IIM_Surface *giantNeutral = attachedLevelTheme->getGiantNeutralIndicator();
    
    for (int cpt = 0 ; cpt < numGiantNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - giantNeutral->h;
		drect.w = giantNeutral->w;
		drect.h = giantNeutral->h;
		attachedPainter.requestDraw(giantNeutral, &drect);
		drect_x += giantNeutral->w;
	}
    for (int cpt = 0 ; cpt < numBigNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - bigNeutral->h;
		drect.w = bigNeutral->w;
		drect.h = bigNeutral->h;
		attachedPainter.requestDraw(bigNeutral, &drect);
		drect_x += bigNeutral->w;
	}
    for (int cpt = 0 ; cpt < numNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = drect_y_base - neutral->h;
		drect.w = neutral->w;
		drect.h = neutral->h;
		attachedPainter.requestDraw(neutral, &drect);
		drect_x += neutral->w;
	}
}

void PuyoView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex) {
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

void PuyoView::companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise)
{
    ((AnimatedPuyo *)companionPuyo)->addAnimation(new TurningAnimation(*(AnimatedPuyo *)companionPuyo,
                                                                       companionVector, counterclockwise));
}

void PuyoView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY)
{
    ((AnimatedPuyo *)puyo)->addAnimation(new FallingAnimation(*(AnimatedPuyo *)puyo, originY, xOffset, yOffset, 16));
}

void PuyoView::puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase)
{
    AnimationSynchronizer *synchronizer = new AnimationSynchronizer();
    viewAnimations.add(new VanishSoundAnimation(phase, synchronizer));
    for (int i = 0, j = puyoGroup.size() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(puyoGroup[i]);
        if (currentPuyo->getPuyoState() != PUYO_NEUTRAL)
            currentPuyo->addAnimation(new VanishAnimation(*currentPuyo, i*2 , xOffset, yOffset, synchronizer));
        else
            currentPuyo->addAnimation(new NeutralPopAnimation(*currentPuyo, i*2, synchronizer));
    }

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


void PuyoView::gameLost()
{
    gameRunning = false;
    for (int i = 0 ; i <= PUYODIMX ; i++) {
        for (int j = 0 ; j <= PUYODIMY ; j++) {
            if (attachedGame->getPuyoAt(i, j) != NULL) {
                AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(attachedGame->getPuyoAt(i, j));
                currentPuyo->addAnimation(new GameOverFallAnimation(*currentPuyo, (j - PUYODIMY) + abs((PUYODIMX / 2) - i) * 5));
            }
        }
    }
}




