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

#include "ios_memory.h"
using namespace ios_fc;

#include "IosVector.h"

#ifdef _WIN32
#define srandom srand
#define random rand
#endif

#ifndef PUYOGAME_H
#define PUYOGAME_H

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
	PuyoRandomSystem();
	PuyoState getPuyoForSequence(int sequence);
private:
	AdvancedBuffer<int> sequenceItems;
};

// A PuyoPuyo is an entity of the game
class PuyoPuyo {
public:
    PuyoPuyo(PuyoState state);
    virtual ~PuyoPuyo() {};
    PuyoState getPuyoState();
    void setPuyoState(PuyoState state);
    bool PuyoPuyo::isFalling();
    int getPuyoX() const;
    int getPuyoY() const;
    void setPuyoXY(int X, int Y);
    void setFlag() { flag = true; }
    void unsetFlag() { flag = false; }
    bool getFlag() const { return flag; }
    int getID() const { return puyoID; }
    void setID(int id) { puyoID = id; }
private:
    int puyoID;
    static int lastID;
    PuyoState state;
    int X, Y;
    bool flag;
};

// The puyos must be created by a factory to ensure custom puyo creation
class PuyoFactory {
 public:
  virtual PuyoPuyo *createPuyo(PuyoState state) = 0;
  virtual void deletePuyo(PuyoPuyo *target) { delete target; }
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
				int companionVector,
				bool counterclockwise) = 0;
  virtual void puyoDidFall(PuyoPuyo *puyo, int originX, int originY) = 0;
  virtual void puyoWillVanish(IosVector &puyoGroup, int groupNum, int phase) = 0;
  virtual void gameDidEndCycle() = 0;
  virtual void gameLost() = 0;
};

class PuyoGame {
public:
    PuyoGame(PuyoFactory *attachedFactory);
    PuyoGame();
    virtual ~PuyoGame() {}
    virtual void setDelegate(PuyoDelegate *delegate);
    virtual void cycle() = 0;
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
    virtual void dropNeutrals() = 0;
    virtual bool isGameRunning() const = 0;
    virtual bool isEndOfCycle() const = 0;
    virtual int  getColumnHeigth(int colNum) const = 0;
    virtual int  getMaxColumnHeight() const = 0;
    virtual int  getSamePuyoAround(int X, int Y, PuyoState color) = 0;
    virtual int  getSemiMove() const = 0;
    
    virtual int  getPoints()  { return points; }
    int points;
protected:
    PuyoDelegate *delegate;
    PuyoFactory *attachedFactory;
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
    void dropNeutrals();
    bool isGameRunning() const { return gameRunning; }
    bool isEndOfCycle() const { return endOfCycle; }
    int getColumnHeigth(int colNum) const;
    int getMaxColumnHeight() const;
    int getSamePuyoAround(int X, int Y, PuyoState color);
    
    int getSemiMove() const { return semiMove; }
    
private:
    void InitGame(PuyoRandomSystem *attachedRandom);
    // Set the state of the puyo at the indicated coordinates (not recommanded)
    void setPuyoCellAt(int X, int Y, PuyoState value);
    // Set the puyo at the indicated coordinates
    void setPuyoAt(int X, int Y, PuyoPuyo *newPuyo);
    
    void setFallingAtTop(bool gameConstruction = false);
    int getFallY(int X, int Y) const;
    void cycleEnding();
    void markPuyoAt(int X, int Y, PuyoState color, bool includeNeutral);
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
    int neutralPuyos;
    int phase;
    int semiMove;
    
    // This is not really a puyo, it is instead an indicator for the edges of the game
    PuyoPuyo *unmoveablePuyo;
    
    // We are keeping a list of current puyos
    AdvancedBuffer<PuyoPuyo *> puyoVector;
    int nbFalled;
};

#endif // PUYOGAME_H
