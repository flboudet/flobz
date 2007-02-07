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

#ifndef _PUYOVIEW
#define _PUYOVIEW

#include "glSDL.h"
#include "PuyoGame.h"
#include "IosImgProcess.h"
#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "AnimatedPuyoTheme.h"

#define TSIZE 32
#define ASIZE 32
#define BSIZE 32
#define CSIZE 32
#define DSIZE 192
#define ESIZE 32
#define FSIZE 16

class PuyoGameFactory {
public:
    virtual PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) = 0;
};

class PuyoView : public PuyoDelegate {
  public:
    PuyoView(PuyoGameFactory *attachedPuyoGameFactory,
	     AnimatedPuyoSetTheme *attachedThemeSet,
         PuyoLevelTheme *attachedLevelTheme,
	     int xOffset, int yOffset, int nXOffset, int nYOffset, SDL_Painter &painterToUse);
    virtual ~PuyoView();
    void setEnemyGame(PuyoGame *enemyGame);
    void render();
    void renderNeutral();
    void cycleAnimation();
    virtual void cycleGame();
    void allowCycle() { cycleAllowance++; }
    void disallowCycle() { cycleAllowance--; }
    
    virtual void moveLeft();
    virtual void moveRight();
    virtual void rotateLeft();
    virtual void rotateRight();

    int getValenceForPuyo(PuyoPuyo *puyo) const;
    PuyoGame *getAttachedGame() const { return attachedGame; }
    SDL_Painter & getPainter() const { return attachedPainter; }
    
    bool isGameOver() const;
    
    int getScreenCoordinateX(int X) const { return X * TSIZE + xOffset; }
    int getScreenCoordinateY(int Y) const { return Y * TSIZE + yOffset; }
    
    AnimatedPuyoSetTheme *getPuyoThemeSet() const { return attachedThemeSet; }

    // PuyoDelegate methods
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY);
    void puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase);
    void gameLost();
    
  protected:
    bool cycleAllowed();
    bool skippedCycle;
    bool gameRunning;
    int xOffset, yOffset;
    int nXOffset, nYOffset;
    AnimatedPuyoSetTheme *attachedThemeSet;
    PuyoLevelTheme *attachedLevelTheme;
    AnimatedPuyoFactory attachedPuyoFactory;
    PuyoGame *attachedGame, *enemyGame;
    AdvancedBuffer<Animation *> viewAnimations;
    int cycleAllowance;
    SDL_Painter &attachedPainter;
    int delayBeforeGameOver;
};

#endif // _PUYOVIEW
