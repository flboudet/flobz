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
	IosVector sequenceItems;
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
private:
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
  PuyoGame(PuyoRandomSystem *attachedRandom, PuyoFactory *attachedFactory);
  PuyoGame(PuyoRandomSystem *attachedRandom);
  virtual ~PuyoGame();
  void setDelegate(PuyoDelegate *delegate);
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
  int getPoints() const { return points; }
  int points;

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
  PuyoDelegate *delegate;
  PuyoFactory *attachedFactory;
  int phase;
  int semiMove;
  
  // This is not really a puyo, it is instead an indicator for the edges of the game
  PuyoPuyo *unmoveablePuyo;
  
  // We are keeping a list of current puyos
  IosVector puyoVector;
  int nbFalled;
};

#endif // PUYOGAME_H
