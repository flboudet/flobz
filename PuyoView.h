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

#define TSIZE 32
#define ASIZE 32
#define BSIZE 32
#define CSIZE 32
#define DSIZE 192
#define ESIZE 32
#define FSIZE 16

extern SDL_Surface *display;
extern IIM_Surface *image;



class PuyoView : public virtual PuyoDelegate {
  public:
    PuyoView(PuyoRandomSystem *attachedRandom, int xOffset, int yOffset, int nXOffset, int nYOffset);
    void setEnemyGame(PuyoGame *enemyGame);
    void render();
    void renderNeutral();
    void cycleAnimation();
    void cycleGame();
    void allowCycle() { cycleAllowance++; }
    void disallowCycle() { cycleAllowance--; }
    
    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();

    static IIM_Surface *getSurfaceForState(PuyoState state);
    IIM_Surface *getSurfaceForPuyo(PuyoPuyo *puyo);
    PuyoGame *getAttachedGame() const { return attachedGame; }
    SDL_Painter & getPainter() const { return attachedPainter; }
    
    int getScreenCoordinateX(int X) const { return X * TSIZE + xOffset; }
    int getScreenCoordinateY(int Y) const { return Y * TSIZE + yOffset; }
    
    // PuyoDelegate methods
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY);
    void puyoWillVanish(IosVector &puyoGroup, int groupNum, int phase);
    void gameLost();
    
  private:
    bool cycleAllowed();
    bool skippedCycle;
    bool gameRunning;
    int xOffset, yOffset;
    int nXOffset, nYOffset;
    AnimatedPuyoFactory attachedPuyoFactory;
    PuyoGame *attachedGame, *enemyGame;
    IosVector viewAnimations;
    int cycleAllowance;
    SDL_Painter &attachedPainter;
};

#endif // _PUYOVIEW
