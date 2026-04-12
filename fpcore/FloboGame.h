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
#include <memory>

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
    int _numColors;
	std::vector<int> _sequenceItems;
};

// A Flobo is an entity of the game
class Flobo {
public:
    Flobo(FloboState state) : _floboID(_lastID++), _state(state), _X(0), _Y(0), _flag(false), _bmark(false) {}
    virtual ~Flobo() {};
    inline FloboState getFloboState() const {
        if (this != NULL)
            return _state;
        return FLOBO_EMPTY;
    }
    inline void setFloboState(FloboState state) { if (this != NULL) this->_state = state; }
    inline bool isFalling() { return (_state < FLOBO_EMPTY); }
    inline int getFloboX() const {
        if (this != NULL)
            return _X;
        return 0;
    }
    inline int getFloboY() const {
        if (this != NULL)
            return _Y;
        return 0;
    }
    inline void setFloboXY(int X, int Y) {
        if (this != NULL)
            this->_X = X; this->_Y = Y;
    }
    inline void setFlag() { _flag = true; }
    inline void unsetFlag() { _flag = false; }
    inline bool getFlag() const { return _flag; }
    inline int  getID() const { return _floboID; }
    inline void setID(int id) { _floboID = id; }
    inline void mark() { if (this != NULL) _bmark = true; }
    inline void unmark() { if (this != NULL) _bmark = false; }
    inline void setMark(bool pmark) { if (this != NULL) _bmark = pmark; }
    inline bool isMarked() const { if (this != NULL) return _bmark; return false; }
private:
    int _floboID;
    static int _lastID;
    FloboState _state;
    int _X, _Y;
    bool _flag, _bmark;
};

// The flobos must be created by a factory to ensure custom flobo creation
class FloboFactory {
 public:
  virtual std::shared_ptr<Flobo> createFlobo(FloboState state) = 0;
  virtual ~FloboFactory() {};
};

class FloboDefaultFactory : public FloboFactory {
 public:
  std::shared_ptr<Flobo> createFlobo(FloboState state) {
    return std::make_shared<Flobo>(state);
  }
};

class GameListener {
public:
  virtual void gameDidAddNeutral(std::shared_ptr<Flobo> neutralFlobo, int neutralIndex, int totalNeutral) {}
  virtual void companionDidTurn(std::shared_ptr<Flobo> companionFlobo,
				std::shared_ptr<Flobo> fallingFlobo,
				bool counterclockwise) {}
  virtual void fallingsDidMoveLeft(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo) {}
  virtual void fallingsDidMoveRight(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo) {}
  virtual void fallingsDidFallingStep(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo) {}
  virtual void floboDidFall(std::shared_ptr<Flobo> flobo, int originX, int originY, int nFalledBelow) {}
  virtual void floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase) {}
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
    virtual std::shared_ptr<Flobo> getFloboAt(int X, int Y) const = 0;

    // List access to the Flobo objects
    virtual int getFloboCount() const = 0;
    //virtual std::shared_ptr<Flobo> getFloboAtIndex(int index) const = 0;

    virtual FloboState getNextFalling() = 0;
    virtual FloboState getNextCompanion() = 0;

    virtual FloboState getCompanionState() const = 0;
    virtual FloboState getFallingState() const = 0;
    virtual int getFallingX() const = 0;
    virtual int getFallingY() const = 0;

    virtual int getFallingCompanionDir() const = 0;
    virtual std::shared_ptr<Flobo> getFallingFlobo() const = 0;

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

    virtual PlayerGameStat &getGameStat() { return _gameStat; }
    virtual void setGameStat(PlayerGameStat &stat) { _gameStat = stat; }

	virtual void setScoringLevel(int gameLevel) {}

    virtual void addNeutralLayer() {}
protected:
    typedef std::vector<GameListener *> GameListenerPtrVector;
    GameListenerPtrVector _listeners;
    FloboFactory *_attachedFactory;
    PlayerGameStat _gameStat;
};

class FloboIterator {
public:
    virtual std::shared_ptr<Flobo> get() = 0;
    virtual bool end() = 0;
    virtual FloboIterator & operator ++() = 0;
};

class FloboDefaultIterator : public FloboIterator {
public:
    FloboDefaultIterator(FloboGame *game)
        : _game(game), _peek(nullptr), _x(0), _y(FLOBOBAN_DIMY-1), _finished(false) {
        ++(*this);
    }
    std::shared_ptr<Flobo> get() { return _peek; }
    bool end() { return _finished; }
    FloboIterator & operator ++() {
        do {
            _peek = _game->getFloboAt(_x++, _y);
            if (_x == FLOBOBAN_DIMX) {
                _x = 0; --_y;
            }
            else if (_y == -1)
                _finished = true;
        } while ((_peek == nullptr) && (! _finished));
        return *this;
    }
private:
    FloboGame *_game;
    std::shared_ptr<Flobo> _peek;
    bool _finished;
    int _x, _y;
};

class FloboLocalGame : public FloboGame {
public:
    FloboLocalGame(RandomSystem *attachedRandom, FloboFactory *attachedFactory);
    FloboLocalGame(RandomSystem *attachedRandom);
    virtual ~FloboLocalGame();
    void cycle();

    // Get the flobo at the indicated coordinates
    std::shared_ptr<Flobo> getFloboAt(int X, int Y) const;

    // List access to the Flobo objects
    int getFloboCount() const;
    std::shared_ptr<Flobo> getFloboAtIndex(int index) const;

    void moveLeft();
    void moveRight();
    void rotate(bool left);
    void rotateLeft();
    void rotateRight();
    FloboState getNextFalling();
    FloboState getNextCompanion();
    FloboState getCompanionState() const { return _companionFlobo->getFloboState(); }
    FloboState getFallingState() const { return _fallingFlobo->getFloboState(); }

    int getFallingX() const { return _fallingFlobo->getFloboX(); }
    int getFallingY() const { return _fallingFlobo->getFloboY(); }
    int getCompanionX() const { return _companionFlobo->getFloboX(); }
    int getCompanionY() const { return _companionFlobo->getFloboY(); }
    int getFallingCompanionX() const;
    int getFallingCompanionY() const;
    int getFallingCompanionDir() const { return _fallingCompanion; }
    std::shared_ptr<Flobo> getFallingFlobo() const { return _fallingFlobo; }

    void increaseNeutralFlobos(int incr);
    int getNeutralFlobos() const;
    int getGameTotalNeutralFlobos() const;
    void dropNeutrals();
    bool isGameRunning() const { return _gameRunning; }
    bool isEndOfCycle() const { return _endOfCycle; }
    int getColumnHeigth(int colNum) const;
    int getMaxColumnHeight() const;
    int getSameFloboAround(int X, int Y, FloboState color);
    int  getComboPhase() const {return _phase; }

    int getSemiMove() const { return _semiMove; }
    bool isPhaseReady(void);

    void setScoringLevel(int gameLevel) { this->_gameLevel = gameLevel; }

    virtual void addNeutralLayer();
private:
    void InitGame(RandomSystem *attachedRandom);
    // Get the state of the flobo at the indicated coordinates
    FloboState getFloboCellAt(int X, int Y) const;
    // Set the state of the flobo at the indicated coordinates (not recommanded)
    void setFloboCellAt(int X, int Y, FloboState value);
    // Set the flobo at the indicated coordinates
    void setFloboAt(int X, int Y, std::shared_ptr<Flobo> newFlobo);

    void setFallingAtTop(bool gameConstruction = false);
    int getFallY(int X, int Y) const;
    void cycleEnding();
    void markFloboAt(int X, int Y, bool mark, bool includeNeutral);
    void deleteMarkedFlobosAt(int X, int Y);
    int removeFlobos();
    void notifyReductions();

    bool _gameRunning;
    bool _endOfCycle;

    // The falling is the flobo you couldn't control,
    // whereas you can make the companion turn around the falling flobo
    std::shared_ptr<Flobo> _fallingFlobo, _companionFlobo;
    int _fallingX, _fallingY;

    // Position of the companion is relative of the falling flobo
    // 0 = up 1 = left 2 = down 3 = up
    unsigned char _fallingCompanion;

    std::shared_ptr<Flobo> floboCells[FLOBOBAN_DIMX * (FLOBOBAN_DIMY+1)];
    RandomSystem *_attachedRandom;
    int _sequenceNr;
    int _phaseReady;
    int _neutralFlobos;
    int _phase;
    int _semiMove;

    // This is not really a flobo, it is instead an indicator for the edges of the game
    std::shared_ptr<Flobo> _unmoveableFlobo;

    // We are keeping a list of current flobos
    std::vector<std::shared_ptr<Flobo>> _floboVector;
    int _nbFalled;

    // Game level for points calculation
    int _gameLevel;
};

#endif // FLOBOGAME_H
