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

#include "FloboGameAbstract.h"

#include "ios_memory.h"
#include <vector>

using namespace ios_fc;

#ifndef FLOBOGAME_H
#define FLOBOGAME_H

#ifdef WIN32
#define srandom srand
#define random rand
#endif

#define FLOBOBAN_DIMX 6
#define FLOBOBAN_DIMY 14

#define FLOBO_STILL FLOBO_BLUE-FLOBO_FALLINGBLUE

enum FloboState {
	FLOBO_FALLINGBLUE = 0,
	FLOBO_FALLINGRED = 1,
	FLOBO_FALLINGGREEN = 2,
	FLOBO_FALLINGVIOLET = 3,
	FLOBO_FALLINGYELLOW = 4,
	FLOBO_EMPTY = 5,
	FLOBO_BLUE = 6,
	FLOBO_RED = 7,
	FLOBO_GREEN = 8,
	FLOBO_VIOLET = 9,
	FLOBO_YELLOW = 10,
	FLOBO_NEUTRAL = 11,
	FLOBO_UNMOVEABLE = 12,
	FLOBO_MARKED = 13,
	FLOBO_REMOVED = 14
};

class RandomSystem {
public:
	RandomSystem(int numColors);
    RandomSystem(unsigned long seed, int numColors);
	FloboState getFloboStateForSequence(int sequence);
private:
    int numColors;
	AdvancedBuffer<int> sequenceItems;
};

// A Flobo is an entity of the game
class Flobo {
public:
    Flobo(FloboState state) : floboID(lastID++), state(state), X(0), Y(0), flag(false), bmark(false) {}
    virtual ~Flobo() {};
    inline FloboState getFloboState() const {
        if (this != NULL)
            return state;
        return FLOBO_EMPTY;
    }
    inline void setFloboState(FloboState state) { if (this != NULL) this->state = state; }
    inline bool isFalling() { return (state < FLOBO_EMPTY); }
    inline int getFloboX() const {
        if (this != NULL)
            return X;
        return 0;
    }
    inline int getFloboY() const {
        if (this != NULL)
            return Y;
        return 0;
    }
    inline void setFloboXY(int X, int Y) {
        if (this != NULL)
            this->X = X; this->Y = Y;
    }
    inline void setFlag() { flag = true; }
    inline void unsetFlag() { flag = false; }
    inline bool getFlag() const { return flag; }
    inline int  getID() const { return floboID; }
    inline void setID(int id) { floboID = id; }
    inline void mark() { if (this != NULL) bmark = true; }
    inline void unmark() { if (this != NULL) bmark = false; }
    inline void setMark(bool pmark) { if (this != NULL) bmark = pmark; }
    inline bool isMarked() const { if (this != NULL) return bmark; return false; }
private:
    int floboID;
    static int lastID;
    FloboState state;
    int X, Y;
    bool flag, bmark;
};

// The flobos must be created by a factory to ensure custom flobo creation
class FloboFactory {
 public:
  virtual Flobo *createFlobo(FloboState state) = 0;
  virtual void deleteFlobo(Flobo *target) { delete target; }
  virtual ~FloboFactory() {};
};

class FloboDefaultFactory : public FloboFactory {
 public:
  Flobo *createFlobo(FloboState state) {
    return new Flobo(state);
  }
};

class GameListener {
public:
  virtual void gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral) {}
  virtual void companionDidTurn(Flobo *companionFlobo,
				Flobo *fallingFlobo,
				bool counterclockwise) {}
  virtual void fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo) {}
  virtual void fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo) {}
  virtual void fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo) {}
  virtual void floboDidFall(Flobo *flobo, int originX, int originY, int nFalledBelow) {}
  virtual void floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase) {}
  virtual void gameDidEndCycle() {}
  virtual void gameLost() {}
  virtual ~GameListener() {};
};

class FloboGame {
public:
    FloboGame(FloboFactory *attachedFactory);
    FloboGame();
    virtual ~FloboGame() {}
public: // Listeners management
    virtual void addGameListener(GameListener *listener);
public:
    // Perform a cycle in the game state machine
    virtual void cycle() = 0;
public: // Controls
    virtual void moveLeft() {}
    virtual void moveRight() {}
    virtual void rotate(bool left) {}
    virtual void rotateLeft() {}
    virtual void rotateRight() {}
public: // Accessors
    // Get the flobo at the indicated coordinates
    virtual Flobo *getFloboAt(int X, int Y) const = 0;

    // List access to the Flobo objects
    virtual int getFloboCount() const = 0;
    //virtual Flobo *getFloboAtIndex(int index) const = 0;

    virtual FloboState getNextFalling() = 0;
    virtual FloboState getNextCompanion() = 0;

    virtual FloboState getCompanionState() const = 0;
    virtual FloboState getFallingState() const = 0;
    virtual int getFallingX() const = 0;
    virtual int getFallingY() const = 0;

    virtual int getFallingCompanionDir() const = 0;
    virtual Flobo *getFallingFlobo() const = 0;

    virtual void increaseNeutralFlobos(int incr) = 0;
    virtual int  getNeutralFlobos() const = 0;
    virtual int  getGameTotalNeutralFlobos() const { return 0; }
    virtual void dropNeutrals() = 0;
    virtual bool isGameRunning() const = 0;
    virtual bool isEndOfCycle() const = 0;
    virtual int  getColumnHeigth(int colNum) const = 0;
    virtual int  getMaxColumnHeight() const = 0;
    virtual int  getSameFloboAround(int X, int Y, FloboState color) = 0;
    virtual int  getSemiMove() const = 0;
    virtual int  getComboPhase() const = 0;

    virtual bool isPhaseReady(void) { return true; }

    virtual PlayerGameStat &getGameStat() { return gameStat; }
    virtual void setGameStat(PlayerGameStat &stat) { gameStat = stat; }

	virtual void setScoringLevel(int gameLevel) {}

    virtual void addNeutralLayer() {}
protected:
    typedef std::vector<GameListener *> GameListenerPtrVector;
    GameListenerPtrVector m_listeners;
    FloboFactory *attachedFactory;
    PlayerGameStat gameStat;
};

class FloboIterator {
public:
    virtual Flobo *get() = 0;
    virtual bool end() = 0;
    virtual FloboIterator & operator ++() = 0;
};

class FloboDefaultIterator : public FloboIterator {
public:
    FloboDefaultIterator(FloboGame *game)
        : m_game(game), m_peek(NULL), m_x(0), m_y(0), m_finished(false) {
        ++(*this);
    }
    Flobo *get() { return m_peek; }
    bool end() { return m_finished; }
    FloboIterator & operator ++() {
        do {
            m_peek = m_game->getFloboAt(m_x++, m_y);
            if (m_x == FLOBOBAN_DIMX) {
                m_x = 0; ++m_y;
            }
            else if (m_y == FLOBOBAN_DIMY)
                m_finished = true;
        } while ((m_peek == NULL) && (! m_finished));
        return *this;
    }
private:
    FloboGame *m_game;
    Flobo *m_peek;
    bool m_finished;
    int m_x, m_y;
};

class FloboLocalGame : public FloboGame {
public:
    FloboLocalGame(RandomSystem *attachedRandom, FloboFactory *attachedFactory);
    FloboLocalGame(RandomSystem *attachedRandom);
    virtual ~FloboLocalGame();
    void cycle();

    // Get the flobo at the indicated coordinates
    Flobo *getFloboAt(int X, int Y) const;

    // List access to the Flobo objects
    int getFloboCount() const;
    Flobo *getFloboAtIndex(int index) const;

    void moveLeft();
    void moveRight();
    void rotate(bool left);
    void rotateLeft();
    void rotateRight();
    FloboState getNextFalling();
    FloboState getNextCompanion();
    FloboState getCompanionState() const { return companionFlobo->getFloboState(); }
    FloboState getFallingState() const { return fallingFlobo->getFloboState(); }

    int getFallingX() const { return fallingFlobo->getFloboX(); }
    int getFallingY() const { return fallingFlobo->getFloboY(); }
    int getCompanionX() const { return companionFlobo->getFloboX(); }
    int getCompanionY() const { return companionFlobo->getFloboY(); }
    int getFallingCompanionX() const;
    int getFallingCompanionY() const;
    int getFallingCompanionDir() const { return fallingCompanion; }
    Flobo *getFallingFlobo() const { return fallingFlobo; }

    void increaseNeutralFlobos(int incr);
    int getNeutralFlobos() const;
    int getGameTotalNeutralFlobos() const;
    void dropNeutrals();
    bool isGameRunning() const { return gameRunning; }
    bool isEndOfCycle() const { return endOfCycle; }
    int getColumnHeigth(int colNum) const;
    int getMaxColumnHeight() const;
    int getSameFloboAround(int X, int Y, FloboState color);
    int  getComboPhase() const {return phase; }

    int getSemiMove() const { return semiMove; }
    bool isPhaseReady(void);

    void setScoringLevel(int gameLevel) { this->gameLevel = gameLevel; }

    virtual void addNeutralLayer();
private:
    void InitGame(RandomSystem *attachedRandom);
    // Get the state of the flobo at the indicated coordinates
    FloboState getFloboCellAt(int X, int Y) const;
    // Set the state of the flobo at the indicated coordinates (not recommanded)
    void setFloboCellAt(int X, int Y, FloboState value);
    // Set the flobo at the indicated coordinates
    void setFloboAt(int X, int Y, Flobo *newFlobo);

    void setFallingAtTop(bool gameConstruction = false);
    int getFallY(int X, int Y) const;
    void cycleEnding();
    void markFloboAt(int X, int Y, bool mark, bool includeNeutral);
    void deleteMarkedFlobosAt(int X, int Y);
    int removeFlobos();
    void notifyReductions();

    bool gameRunning;
    bool endOfCycle;

    // The falling is the flobo you couldn't control,
    // whereas you can make the companion turn around the falling flobo
    Flobo *fallingFlobo, *companionFlobo;
    int fallingX, fallingY;

    // Position of the companion is relative of the falling flobo
    // 0 = up 1 = left 2 = down 3 = up
    unsigned char fallingCompanion;

    Flobo *floboCells[FLOBOBAN_DIMX * (FLOBOBAN_DIMY+1)];
    RandomSystem *attachedRandom;
    int sequenceNr;
    int phaseReady;
    int neutralFlobos;
    int phase;
    int semiMove;

    // This is not really a flobo, it is instead an indicator for the edges of the game
    Flobo *unmoveableFlobo;

    // We are keeping a list of current flobos
    AdvancedBuffer<Flobo *> floboVector;
    int nbFalled;

    // Game level for points calculation
    int gameLevel;
};

#endif // FLOBOGAME_H
