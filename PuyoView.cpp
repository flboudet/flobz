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
SDL_Painter painter;

//extern SDL_Surface *display;
IIM_Surface *background, *neutral = NULL;
IIM_Surface *bigNeutral = NULL;

#ifdef DESACTIVE
static char PuyoGroupImageIndex[2][2][2][2] =
{ {  // empty bottom
{  // empty right
{  // empty top
	0, // empty left
	4, // full left
},
{  // full top
	3, // empty left
	10, // full left
}
},
{  // full right
	{  // empty top
        2, // empty left
        9, // full left
	},
	{  // full top
        8, // empty left
        14, // full left
	}
}
},
{  // full bottom
    {  // empty right
	{  // empty top
        1, // empty left
        7, // full left
	},
	{  // full top
        6, // empty left
        13, // full left
	}
    },
    {  // full right
	{  // empty top
        5, // empty left
        12, // full left
	},
	{  // full top
        11, // empty left
        15, // full left
	}
    }
}
};
#endif



PuyoView::PuyoView(PuyoGameFactory *attachedPuyoGameFactory,
		   AnimatedPuyoSetTheme *attachedThemeSet,
		   int xOffset, int yOffset, int nXOffset, int nYOffset, SDL_Painter &painterToUse)
  :attachedThemeSet(attachedThemeSet),
   attachedPuyoFactory(this), attachedPainter(painterToUse), delayBeforeGameOver(60)
{
    printf("Constructeur du PuyoView\n");
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
    if (viewAnimations.getSize() > 0) {
        PuyoAnimation *currentAnimation = (PuyoAnimation *)(viewAnimations.getElementAt(0));
        if (currentAnimation->isFinished()) {
            viewAnimations.removeElementAt(0);
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
    if (viewAnimations.getSize() > 0) {
        PuyoAnimation *currentAnimation = (PuyoAnimation *)(viewAnimations.getElementAt(0));
        if (!currentAnimation->isFinished()) {
            currentAnimation->draw(0);
        }
    }
}

void PuyoView::renderNeutral()
{
	SDL_Rect drect;
	
	// Drawing neutral puyos
	int numBigNeutral = attachedGame->getNeutralPuyos() / PUYODIMX;
	int numNeutral = attachedGame->getNeutralPuyos() % PUYODIMX;
	int drect_x = xOffset;
	int compressor = 0;
	
	int width = numBigNeutral * bigNeutral->w + numNeutral * neutral->w;
	if (width > TSIZE * PUYODIMX) {
		compressor = (width - TSIZE * PUYODIMX) / (numNeutral + numBigNeutral);
	} 
	
	for (int cpt = 0 ; cpt < numBigNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = yOffset + 3 + TSIZE;
		drect.w = bigNeutral->w;
		drect.h = bigNeutral->h;
		attachedPainter.requestDraw(bigNeutral, &drect);
		drect_x += bigNeutral->w - compressor;
	}
	for (int cpt = 0 ; cpt < numNeutral ; cpt++) {
		drect.x = drect_x;
		drect.y = yOffset + 3 + TSIZE;
		drect.w = neutral->w;
		drect.h = neutral->h;
		attachedPainter.requestDraw(neutral, &drect);
		drect_x += neutral->w - compressor;
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
    viewAnimations.addElement(new VanishSoundAnimation(phase, synchronizer));
    for (int i = 0, j = puyoGroup.size() ; i < j ; i++) {
        AnimatedPuyo *currentPuyo = static_cast<AnimatedPuyo *>(puyoGroup[i]);
        if (currentPuyo->getPuyoState() != PUYO_NEUTRAL)
            currentPuyo->addAnimation(new VanishAnimation(*currentPuyo, i*2 , xOffset, yOffset, synchronizer));
        else
            currentPuyo->addAnimation(new NeutralPopAnimation(*currentPuyo, i*2, synchronizer));
    }
    // A revoir
    if (groupNum == 0) {
        if (phase>=2) {
            audio_sound_play(sound_yahoohoo3[(int)((float)NB_YAHOOHOO3 * random()/(RAND_MAX+1.0))]);
        }
        if (phase==1) {
            audio_sound_play(sound_yahoohoo2[(int)((float)NB_YAHOOHOO2 * random()/(RAND_MAX+1.0))]);
        }
        else {
            audio_sound_play(sound_yahoohoo1[(int)((float)NB_YAHOOHOO1 * random()/(RAND_MAX+1.0))]);
        }
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




