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

#include "PuyoGame.h"
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

/**
 * Abstract class serving as a factory
 * for the PuyoView to create the corresponding PuyoGame.
 * Implemented in the following classes:
 * - PuyoLocalGameFactory, to be used with a PuyoView on a game
 *   playing on the local computer,
 * - PuyoRemoteGameFactory, to be used with a PuyoView to display
 *   a game playing on a distant computer.
 */
class PuyoGameFactory {
public:
    virtual PuyoGame *createPuyoGame(PuyoFactory *attachedPuyoFactory) = 0;
    virtual ~PuyoGameFactory() {};
};

/**
 * PuyoView is the graphical representation of a PuyoGame.
 * The PuyoGame is owned by the PuyoView and created
 * through a PuyoGameFactory.
 */
class PuyoView : public PuyoDelegate {
  public:
    // Create a PuyoView with graphical feedback
    PuyoView(PuyoGameFactory *attachedPuyoGameFactory,
	     AnimatedPuyoSetTheme *attachedThemeSet,
         PuyoLevelTheme *attachedLevelTheme,
	     int xOffset, int yOffset, int nXOffset, int nYOffset);

    // Create a PuyoView without graphical feedback
    PuyoView(PuyoGameFactory *attachedPuyoGameFactory);

    void setPlayerNames(const char *p1, const char *p2) { p1name = p1; p2name = p2; }

    virtual ~PuyoView();
    void setEnemyGame(PuyoGame *enemyGame);
    void render(DrawTarget *dt);
    void renderOverlay(DrawTarget *dt);
    void renderNeutral(DrawTarget *dt);
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

    bool isGameOver() const;

    int getScreenCoordinateX(int X) const { return X * TSIZE + m_xOffset; }
    int getScreenCoordinateY(int Y) const { return Y * TSIZE + m_yOffset; }

    AnimatedPuyoSetTheme *getPuyoThemeSet() const { return attachedThemeSet; }

    bool isNewMetaCycleStart() { return newMetaCycleStart; }
    void clearMetaCycleStart() { newMetaCycleStart = false; }

    // PuyoDelegate methods
	virtual void fallingsDidMoveLeft(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
	virtual void fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
	virtual void fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, PuyoPuyo *fallingPuyo, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow);
    void puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase);
    virtual void gameLost();
    int getPlayerId() const { return (m_xOffset < 320) ? 1 : 2; }
    virtual void gameWin();
    void setShowNextPuyos(bool show) { m_showNextPuyos = show; }
    void setShowShadows(bool show) { m_showShadows = show; }
    void setShowEyes(bool show) { attachedPuyoFactory.setShowEyes(show); }
    void setNeutralPuyosDisplayPosition(int x, int y) {
        neutralXOffset = x;
        neutralYOffset = y;
    }
    void setPosition(int x, int y) {
        this->m_xOffset = x;
        this->m_yOffset = y - TSIZE;
    }
    void setNextPuyosPosition(int x, int y) {
        this->m_nXOffset = x;
        this->m_nYOffset = y;
    }
  protected:
    String p1name, p2name;
    bool cycleAllowed();
    bool m_showNextPuyos, m_showShadows, m_showEyes;
    bool skippedCycle;
    bool gameRunning;
    int m_xOffset, m_yOffset;
    int m_nXOffset, m_nYOffset;
    int neutralXOffset, neutralYOffset;
    AnimatedPuyoSetTheme *attachedThemeSet;
    PuyoLevelTheme *attachedLevelTheme;
    AnimatedPuyoFactory attachedPuyoFactory;
    PuyoGame *attachedGame, *enemyGame;
    AdvancedBuffer<Animation *> viewAnimations;
    int cycleAllowance;
    int delayBeforeGameOver;
    bool newMetaCycleStart;
    PlayerGameStatDisplay *attachedStatDisplay;

    void initCommon(PuyoGameFactory *attachedPuyoGameFactory);
    void initDisplay(int xOffset, int yOffset, int nXOffset, int nYOffset);
    bool haveDisplay;
};

#endif // _PUYOVIEW
