/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gborios@ios-software.com>
 *
 * iOS Software <http://ios.free.fr>
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
#include "PuyoIA.h"
#include "PuyoCommander.h"
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
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY);
    void puyoWillVanish(IosVector &puyoGroup, int groupNum, int phase);
    void gameLost();
    static IIM_Surface *getSurfaceForState(PuyoState state);
    IIM_Surface *getSurfaceForPuyo(PuyoPuyo *puyo);
    PuyoGame *getAttachedGame() const { return attachedGame; }
    // normalement c'est prive
    int xOffset, yOffset, nXOffset, nYOffset;
  private:
    bool cycleAllowed();
    bool skippedCycle;
    bool gameRunning;
    AnimatedPuyoFactory attachedPuyoFactory;
    PuyoGame *attachedGame, *enemyGame;
    IosVector viewAnimations;
};

class PuyoStarter : public PuyoDrawable {
  public:
    PuyoStarter(PuyoCommander *commander, bool aiLeft, int aiLevel, IA_Type aiType, int theme);
    virtual ~PuyoStarter();
    void run(int score1, int score2, int lives, int point1, int point2);
    void draw();

    bool leftPlayerWin() const  { return attachedGameA->isGameRunning(); }
    bool rightPlayerWin() const { return attachedGameB->isGameRunning(); }

    int leftPlayerPoints() const { return attachedGameA->getPoints(); }
    int rightPlayerPoints() const { return attachedGameB->getPoints(); }
    
  private:
    PuyoCommander *commander;
    PuyoView *areaA, *areaB;
    PuyoGame *attachedGameA, *attachedGameB;
    PuyoIA *randomPlayer;
    PuyoRandomSystem attachedRandom;
    int tickCounts;
    int lives;
    int score1;
    int score2;
    bool stopRendering;
    bool paused;
    int gameSpeed;

    void stopRender();
    void restartRender();

    int blinkingPointsA, blinkingPointsB, savePointsA, savePointsB;
};

#endif // _PUYOVIEW
