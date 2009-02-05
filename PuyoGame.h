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

#include "PuyoGameAbstract.h"

#include "ios_memory.h"
using namespace ios_fc;

#ifndef PUYOGAME_H
#define PUYOGAME_H

#ifdef WIN32
#define srandom srand
#define random rand
#endif

#define PUYODIMX 6
#define PUYODIMY 14

#define PUYO_STILL PUYO_BLUE-PUYO_FALLINGBLUE

enum PuyoState {
	PUYO_FALLINGBLUE = 0,
	PUYO_FALLINGRED = 1,
	PUYO_FALLINGGREEN = 2,
	PUYO_FALLINGVIOLET = 3,
	PUYO_FALLINGYELLOW = 4,
	PUYO_EMPTY = 5,
	PUYO_BLUE = 6,
	PUYO_RED = 7,
	PUYO_GREEN = 8,
	PUYO_VIOLET = 9,
	PUYO_YELLOW = 10,
	PUYO_NEUTRAL = 11,
	PUYO_UNMOVEABLE = 12,
	PUYO_MARKED = 13,
	PUYO_REMOVED = 14
};

class PuyoRandomSystem {
public:
	PuyoRandomSystem(int numColors);
  PuyoRandomSystem(unsigned long seed, int numColors);
	PuyoState getPuyoForSequence(int sequence);
private:
    int numColors;
	AdvancedBuffer<int> sequenceItems;
};

// A PuyoPuyo is an entity of the game
class PuyoPuyo {
public:
    PuyoPuyo(PuyoState state) : puyoID(lastID++), state(state), X(0), Y(0), flag(false), bmark(false) {}
    virtual ~PuyoPuyo() {};
    inline PuyoState getPuyoState() const {
        if (this != NULL)
            return state;
        return PUYO_EMPTY;
    }
    inline void setPuyoState(PuyoState state) { if (this != NULL) this->state = state; }
    inline bool isFalling() { return (state < PUYO_EMPTY); }
    inline int getPuyoX() const {
        if (this != NULL)
            return X;
        return 0;
    }
    inline int getPuyoY() const {
        if (this != NULL)
            return Y;
        return 0;
    }
    inline void setPuyoXY(int X, int Y) {
        if (this != NULL)
            this->X = X; this->Y = Y;
    }
    inline void setFlag() { flag = true; }
    inline void unsetFlag() { flag = false; }
    inline bool getFlag() const { return flag; }
    inline int getID() const { return puyoID; }
    inline void setID(int id) { puyoID = id; }
    inline void mark() { if (this != NULL) bmark = true; }
    inline void unmark() { if (this != NULL) bmark = false; }
    inline void setMark(bool pmark) { if (this != NULL) bmark = pmark; }
    inline bool isMarked() const { if (this != NULL) return bmark; return false; }
private:
    int puyoID;
    static int lastID;
    PuyoState state;
    int X, Y;
    bool flag, bmark;
};

// The puyos must be created by a factory to ensure custom puyo creation
class PuyoFactory {
 public:
  virtual PuyoPuyo *createPuyo(PuyoState state) = 0;
  virtual void deletePuyo(PuyoPuyo *target) { delete target; }
  virtual ~PuyoFactory() {};
};

class PuyoDefaultFactory : public PuyoFactory {
 public:
  PuyoPuyo *createPuyo(PuyoState state) {
    return new PuyoPuyo(state);
  }
};

class PuyoDelegate {
public:
  virtual void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex) = 0;
  virtual void companionDidTurn(PuyoPuyo *companionPuyo,
				PuyoPuyo *fallingPuyo,
				bool counterclockwise) = 0;
  virtual void fallingsDidMoveLeft(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo) = 0;
  virtual void fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo) = 0;
  virtual void fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo) = 0;
  virtual void puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow) = 0;
  virtual void puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase) = 0;
  virtual void gameDidEndCycle() = 0;
  virtual void gameLost() = 0;
  virtual ~PuyoDelegate() {};
};

class PuyoGame {
public:
    PuyoGame(PuyoFactory *attachedFactory);
    PuyoGame();
    
    virtual ~PuyoGame() {}
    virtual void setDelegate(PuyoDelegate *delegate);
    virtual void cycle() = 0;
    
    static const char * getPlayerName(int n);
    static String getDefaultPlayerName(int n);
    static String getDefaultPlayerKey(int n);
    static void setDefaultPlayerName(int n, const char * playerName);
    /*  
        // Get the state of the puyo at the indicated coordinates
        PuyoState getPuyoCellAt(int X, int Y) const;
    */
    // Get the puyo at the indicated coordinates
    virtual PuyoPuyo *getPuyoAt(int X, int Y) const = 0;
    
    // List access to the PuyoPuyo objects
    virtual int getPuyoCount() const = 0;
    virtual PuyoPuyo *getPuyoAtIndex(int index) const = 0;
    
    virtual void moveLeft() {}
    virtual void moveRight() {}
    virtual void rotate(bool left) {}
    virtual void rotateLeft() {}
    virtual void rotateRight() {}
    
    virtual PuyoState getNextFalling() = 0;
    virtual PuyoState getNextCompanion() = 0;
    
    virtual PuyoState getCompanionState() const = 0;
    virtual PuyoState getFallingState() const = 0;
    virtual int getFallingX() const = 0;
    virtual int getFallingY() const = 0;
    /*
     int getCompanionX() const { return companionPuyo->getPuyoX(); }
     int getCompanionY() const { return companionPuyo->getPuyoY(); }
     int getFallingCompanionX() const;
     int getFallingCompanionY() const;
     */
    virtual int getFallingCompanionDir() const = 0;
    virtual PuyoPuyo *getFallingPuyo() const = 0;
    
    virtual void increaseNeutralPuyos(int incr) = 0;
    virtual int  getNeutralPuyos() const = 0;
    virtual int  getGameTotalNeutralPuyos() const { return 0; }
    virtual void dropNeutrals() = 0;
    virtual bool isGameRunning() const = 0;
    virtual bool isEndOfCycle() const = 0;
    virtual int  getColumnHeigth(int colNum) const = 0;
    virtual int  getMaxColumnHeight() const = 0;
    virtual int  getSamePuyoAround(int X, int Y, PuyoState color) = 0;
    virtual int  getSemiMove() const = 0;
    virtual int  getComboPhase() const = 0;
    
    virtual bool isPhaseReady(void) { return true; }

    virtual PlayerGameStat &getGameStat() { return gameStat; }
    virtual void setGameStat(PlayerGameStat &stat) { gameStat = stat; }
protected:
    PuyoDelegate *delegate;
    PuyoFactory *attachedFactory;
    PlayerGameStat gameStat;
};

class PuyoLocalGame : public PuyoGame {
public:
    PuyoLocalGame(PuyoRandomSystem *attachedRandom, PuyoFactory *attachedFactory);
    PuyoLocalGame(PuyoRandomSystem *attachedRandom);
    virtual ~PuyoLocalGame();
    void cycle();
    
    // Get the state of the puyo at the indicated coordinates
    PuyoState getPuyoCellAt(int X, int Y) const;
    // Get the puyo at the indicated coordinates
    PuyoPuyo *getPuyoAt(int X, int Y) const;
    
    // List access to the PuyoPuyo objects
    int getPuyoCount() const;
    PuyoPuyo *getPuyoAtIndex(int index) const;
    
    void moveLeft();
    void moveRight();
    void rotate(bool left);
    void rotateLeft();
    void rotateRight();
    PuyoState getNextFalling();
    PuyoState getNextCompanion();
    PuyoState getCompanionState() const { return companionPuyo->getPuyoState(); }
    PuyoState getFallingState() const { return fallingPuyo->getPuyoState(); }
    
    int getFallingX() const { return fallingPuyo->getPuyoX(); }
    int getFallingY() const { return fallingPuyo->getPuyoY(); }
    int getCompanionX() const { return companionPuyo->getPuyoX(); }
    int getCompanionY() const { return companionPuyo->getPuyoY(); }
    int getFallingCompanionX() const;
    int getFallingCompanionY() const;
    int getFallingCompanionDir() const { return fallingCompanion; }
    PuyoPuyo *getFallingPuyo() const { return fallingPuyo; }
    
    void increaseNeutralPuyos(int incr);
    int getNeutralPuyos() const;
    int getGameTotalNeutralPuyos() const;
    void dropNeutrals();
    bool isGameRunning() const { return gameRunning; }
    bool isEndOfCycle() const { return endOfCycle; }
    int getColumnHeigth(int colNum) const;
    int getMaxColumnHeight() const;
    int getSamePuyoAround(int X, int Y, PuyoState color);
    int  getComboPhase() const {return phase; }
    
    int getSemiMove() const { return semiMove; }
    bool isPhaseReady(void);
    
    void setGameLevel(int gameLevel) { this->gameLevel = gameLevel; }

private:
    void InitGame(PuyoRandomSystem *attachedRandom);
    // Set the state of the puyo at the indicated coordinates (not recommanded)
    void setPuyoCellAt(int X, int Y, PuyoState value);
    // Set the puyo at the indicated coordinates
    void setPuyoAt(int X, int Y, PuyoPuyo *newPuyo);
    
    void setFallingAtTop(bool gameConstruction = false);
    int getFallY(int X, int Y) const;
    void cycleEnding();
    void markPuyoAt(int X, int Y, bool mark, bool includeNeutral);
    void deleteMarkedPuyosAt(int X, int Y);
    int removePuyos();
    void notifyReductions();
    
    bool gameRunning;
    bool endOfCycle;
    
    // The falling is the puyo you couldn't control,
    // whereas you can make the companion turn around the falling puyo
    PuyoPuyo *fallingPuyo, *companionPuyo;
    int fallingX, fallingY;
    
    // Position of the companion is relative of the falling puyo
    // 0 = up 1 = left 2 = down 3 = up
    unsigned char fallingCompanion;
    
    PuyoPuyo *puyoCells[PUYODIMX * (PUYODIMY+1)];
    PuyoRandomSystem *attachedRandom;
    int sequenceNr;
    int phaseReady;
    int neutralPuyos;
    int phase;
    int semiMove;
    
    // This is not really a puyo, it is instead an indicator for the edges of the game
    PuyoPuyo *unmoveablePuyo;
    
    // We are keeping a list of current puyos
    AdvancedBuffer<PuyoPuyo *> puyoVector;
    int nbFalled;

    // Game level for points calculation
    int gameLevel;
};

#endif // PUYOGAME_H
