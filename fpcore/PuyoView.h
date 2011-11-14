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

#include <memory>
#include "FloboGame.h"
#include "PuyoAnimations.h"
#include "AnimatedPuyo.h"
#include "Theme.h"

#define TSIZE 32
#define ASIZE 32
#define BSIZE 32
#define CSIZE 32
#define DSIZE 192
#define ESIZE 32
#define FSIZE 16

/**
 * Abstract class serving as a factory
 * for the PuyoView to create the corresponding FloboGame.
 * Implemented in the following classes:
 * - FloboLocalGameFactory, to be used with a PuyoView on a game
 *   playing on the local computer,
 * - PuyoRemoteGameFactory, to be used with a PuyoView to display
 *   a game playing on a distant computer.
 */
class FloboGameFactory {
public:
    virtual FloboGame *createFloboGame(FloboFactory *attachedFloboFactory) = 0;
    virtual ~FloboGameFactory() {};
};

/**
 * PuyoView is the graphical representation of a FloboGame.
 * The FloboGame is owned by the PuyoView and created
 * through a FloboGameFactory.
 */
class PuyoView : public GameListener {
  public:
    // Create a PuyoView with graphical feedback
    PuyoView(FloboGameFactory *attachedFloboGameFactory,
             int playerId,
             PuyoSetTheme *attachedThemeSet,
             LevelTheme *attachedLevelTheme);

    // Create a PuyoView without graphical feedback
    PuyoView(FloboGameFactory *attachedFloboGameFactory);

    void setupLayout(int playerId);
    void setPlayerNames(const char *p1, const char *p2) { p1name = p1; p2name = p2; }

    virtual ~PuyoView();
    void setEnemyGame(FloboGame *enemyGame);
    void render(DrawTarget *dt);
    void renderNeutral(DrawTarget *dt);
    void renderScore(DrawTarget *dt);
    void cycleAnimation();
    virtual void cycleGame();
    void allowCycle() { cycleAllowance++; }
    void disallowCycle() { cycleAllowance--; }

    virtual void moveLeft();
    virtual void moveRight();
    virtual void rotateLeft();
    virtual void rotateRight();

    int getValenceForPuyo(Flobo *puyo) const;
    FloboGame *getAttachedGame() const { return attachedGame; }

    bool isGameOver() const;

    int getScreenCoordinateX(int X) const { return X * TSIZE + m_xOffset; }
    int getScreenCoordinateY(int Y) const { return Y * TSIZE + m_yOffset; }

    PuyoSetTheme *getPuyoThemeSet() const { return attachedThemeSet; }

    bool isNewMetaCycleStart() { return newMetaCycleStart; }
    void clearMetaCycleStart() { newMetaCycleStart = false; }

    // GameListener methods
	virtual void fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo);
	virtual void fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo);
	virtual void fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo);
    void gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral);
    void gameDidEndCycle();
    void companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise);
    void floboDidFall(Flobo *puyo, int originX, int originY, int nFalledBelow);
    void floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase);
    virtual void gameLost();

    // Accessors
    int getPlayerId() const { return m_playerId; }
    void setPlayerId(int playerId) { m_playerId = playerId; }
    virtual void gameWin();
    void setShowNextFlobos(bool show) { m_showNextFlobos = show; }
    void setShowShadows(bool show) { m_showShadows = show; }
    void setShowEyes(bool show) { attachedFloboFactory.setShowEyes(show); }
    void setNeutralFlobosDisplayPosition(int x, int y) {
        neutralXOffset = x;
        neutralYOffset = y;
    }
    void setPosition(int x, int y) {
        this->m_xOffset = x;
        this->m_yOffset = y - TSIZE;
    }
    void setNextFlobosPosition(int x, int y) {
        this->m_nXOffset = x;
        this->m_nYOffset = y;
    }
    void setScoreDisplayPosition(int x, int y) {
        m_scoreDisplay->setPosition(x, y);
    }
  protected:
    int m_playerId;
    String p1name, p2name;
    bool cycleAllowed();
    bool m_showNextFlobos, m_showShadows, m_showEyes;
    bool skippedCycle;
    bool gameRunning;
    int m_xOffset, m_yOffset;
    int m_nXOffset, m_nYOffset;
    int neutralXOffset, neutralYOffset;
    PuyoSetTheme *attachedThemeSet;
    LevelTheme *attachedLevelTheme;
    AnimatedFloboFactory attachedFloboFactory;
    FloboGame *attachedGame, *enemyGame;
    AdvancedBuffer<Animation *> viewAnimations;
    int cycleAllowance;
    int delayBeforeGameOver;
    bool newMetaCycleStart;
    std::auto_ptr<PlayerGameStatDisplay> m_scoreDisplay;

    void initCommon(FloboGameFactory *attachedFloboGameFactory);
    bool haveDisplay;
};

#endif // _PUYOVIEW
