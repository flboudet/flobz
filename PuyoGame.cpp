#include <stdio.h>
#include <stdlib.h>
#include "PuyoGame.h"
#include "audio.h"
#include <SDL/SDL.h>


static int fallingTable[PUYODIMX] = {0, 3, 1, 4, 2, 5};

PuyoRandomSystem::PuyoRandomSystem()
{
    srandom(SDL_GetTicks());
}

PuyoState PuyoRandomSystem::getPuyoForSequence(int sequence)
{
	if (sequenceItems.getSize() <= sequence) {
		int newItem = (random() % 5) + PUYO_FALLINGBLUE;
		sequenceItems.addElement((void *)newItem);
		return (PuyoState)newItem;
	}
	else
		return (PuyoState)(int)(sequenceItems.getElementAt(sequence));
}

PuyoPuyo::PuyoPuyo(PuyoState state)
{
    this->state = state;
    X = 0; Y = 0;
    attachedObject = NULL;
}

PuyoState PuyoPuyo::getPuyoState()
{
  if (this != NULL)
    return state;
  return PUYO_EMPTY;
}

void PuyoPuyo::setPuyoState(PuyoState state)
{
  if (this != NULL)
  this->state = state;
}

int PuyoPuyo::getPuyoX() const
{
  if (this != NULL)
    return X;
  return 0;
}

int PuyoPuyo::getPuyoY() const
{
  if (this != NULL)
    return Y;
  return 0;
}

void PuyoPuyo::setPuyoXY(int X, int Y)
{
  if (this != NULL)
  {
    this->X = X; this->Y = Y;
  }
}

void PuyoPuyo::setAttachedObject(void *object)
{
  if (this != NULL)
    attachedObject = object;
}

void *PuyoPuyo::getAttachedObject()
{
  if (this != NULL)
    return attachedObject;
  return NULL;
}


PuyoGame::PuyoGame(PuyoRandomSystem *attachedRandom)
{
  nbFalled = 0;
	int i, j;
    unmoveablePuyo = new PuyoPuyo(PUYO_UNMOVEABLE);
	for (i = 0 ; i < PUYODIMX ; i++) {
		for (j = 0 ; j <= PUYODIMY ; j++) {
			if (j == PUYODIMY)
				setPuyoAt(i, j, unmoveablePuyo);
			else
				setPuyoAt(i, j, NULL);
		}
	}
	this->attachedRandom = attachedRandom;
	sequenceNr = 0;
    semiMove = 0;
	neutralPuyos = 0;
	delegate = NULL;
        endOfCycle = false;
	gameRunning = true;
	setFallingAtTop(true);
  points = 0;
}

PuyoGame::~PuyoGame()
{
    delete unmoveablePuyo;
}

void PuyoGame::setDelegate(PuyoDelegate *delegate)
{
	this->delegate = delegate;
}

void PuyoGame::cycle()
{
  if (!gameRunning)
    return;

  semiMove = 1 - semiMove;
  if (semiMove == 0)
      return;

	if (endOfCycle) {
		cycleEnding();
        if (delegate != NULL)
            notifyReductions();
		return;
	}
	if ((fallingY >= 0)&&(getPuyoCellAt(fallingX, fallingY+1) > PUYO_EMPTY) || (getPuyoCellAt(getFallingCompanionX(), getFallingCompanionY()+1) > PUYO_EMPTY)) {
		setPuyoAt(fallingX, getFallY(fallingX, fallingY), fallingPuyo);
        fallingPuyo->setPuyoState((PuyoState)(fallingPuyo->getPuyoState()+PUYO_STILL));
		setPuyoAt(getFallingCompanionX(), getFallY(getFallingCompanionX(), getFallingCompanionY()), companionPuyo);
        companionPuyo->setPuyoState((PuyoState)(companionPuyo->getPuyoState()+PUYO_STILL));
        if (delegate != NULL) {
            delegate->puyoDidFall(fallingPuyo, fallingX, fallingY);
            delegate->puyoDidFall(companionPuyo, getFallingCompanionX(), getFallingCompanionY());
            fallingY = -10;
            notifyReductions();
        }
		endOfCycle = true;
	}
	else {
		fallingY++;
        fallingPuyo->setPuyoXY(fallingX, fallingY);
        companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
    }
}

// Get the state of the puyo at the indicated coordinates
PuyoState PuyoGame::getPuyoCellAt(int X, int Y) const
{
    PuyoPuyo *thePuyo = getPuyoAt(X, Y);
    if (thePuyo)
        return thePuyo->getPuyoState();
    return PUYO_EMPTY;
}

// Get the puyo at the indicated coordinates
PuyoPuyo *PuyoGame::getPuyoAt(int X, int Y) const
{
    if ((X >= PUYODIMX) || (Y >= PUYODIMY) || (X < 0) || (Y < 0))
		return unmoveablePuyo;
	if (!endOfCycle) {
		if ((X == fallingX) && (Y == fallingY))
			return fallingPuyo;
		if ((X == getFallingCompanionX()) && (Y == getFallingCompanionY()))
			return companionPuyo;
	}
    return puyoCells[X + Y * PUYODIMX];
}

// List access to the PuyoPuyo objects
int PuyoGame::getPuyoCount() const
{
    return puyoVector.getSize();
}

PuyoPuyo *PuyoGame::getPuyoAtIndex(int index) const
{
    return (PuyoPuyo *)(puyoVector.getElementAt(index));
}

void PuyoGame::moveLeft()
{
    if (endOfCycle) {
		return;
	}
	if (((fallingY<0)&&(fallingX>0))||((getPuyoCellAt(fallingX-1, fallingY) <= PUYO_EMPTY)
	 && (getPuyoCellAt(getFallingCompanionX()-1, getFallingCompanionY()) <= PUYO_EMPTY)))
        {
            fallingX--;
        }
    fallingPuyo->setPuyoXY(fallingX, fallingY);
    companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
}

void PuyoGame::moveRight()
{
    if (endOfCycle) {
		return;
	}
	if (((fallingY<0)&&(fallingX<PUYODIMX-1))||((getPuyoCellAt(fallingX+1, fallingY) <= PUYO_EMPTY)
	 && (getPuyoCellAt(getFallingCompanionX()+1, getFallingCompanionY()) <= PUYO_EMPTY)))
		fallingX++;
    fallingPuyo->setPuyoXY(fallingX, fallingY);
    companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
}

void PuyoGame::rotate(bool left)
{
    if (endOfCycle) {
        return;
    }
    audio_sound_play(sound_fff);
    unsigned char backupCompanion = fallingCompanion;
    /*int backupX = fallingX;
    int backupY = fallingY;*/
    int newX, newY;
    bool moved = true;
    fallingCompanion = (unsigned char)(fallingCompanion + (left?3:1)) % 4;
    unsigned char newCompanion = fallingCompanion;
    int newCompanionX = getFallingCompanionX();
    int newCompanionY = getFallingCompanionY();
    fallingCompanion = backupCompanion;
    if (getPuyoCellAt(newCompanionX, newCompanionY) > PUYO_EMPTY) {
        //if (((fallingY<0)&&(fallingX>0)&&(fallingX<PUYODIMX-1))||(getPuyoCellAt(newCompanionX, newCompanionY) > PUYO_EMPTY)) {
        newX = fallingX + (fallingX - newCompanionX);
        newY = fallingY + (fallingY - newCompanionY);
        //        if ((fallingY >= 0) && (getPuyoCellAt(newX, newY) > PUYO_EMPTY)) {
        if (getPuyoCellAt(newX, newY) > PUYO_EMPTY) {
            moved = false;
        }
        else {
            fallingCompanion = newCompanion;
            fallingX = newX;
            if (fallingY != newY)
            {
                semiMove = 0;
                fallingY = newY;
            }
            fallingPuyo->setPuyoXY(fallingX, fallingY);
        }
        }
    else
        fallingCompanion = newCompanion;
    
    companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
    
    if ((delegate != NULL) && (moved))
        delegate->companionDidTurn(companionPuyo, fallingCompanion, !left);
    
}

void PuyoGame::rotateLeft()
{
    rotate(true);
}

void PuyoGame::rotateRight()
{
    rotate(false);
}

PuyoState PuyoGame::getNextFalling()
{
	return attachedRandom->getPuyoForSequence(sequenceNr);
}

PuyoState PuyoGame::getNextCompanion()
{
	return attachedRandom->getPuyoForSequence(sequenceNr+1);
}

void PuyoGame::increaseNeutralPuyos(int incr)
{
	neutralPuyos += incr;
}

int PuyoGame::getNeutralPuyos() const
{
	return neutralPuyos;
}



// Set the state of the puyo at the indicated coordinates (not recommanded)
void PuyoGame::setPuyoCellAt(int X, int Y, PuyoState value)
{
	/*if ((X > PUYODIMX) || (Y > PUYODIMY))
		return;*/
  if (puyoCells[X + Y * PUYODIMX])
    puyoCells[X + Y * PUYODIMX]->setPuyoState(value);
};

// Set the puyo at the indicated coordinates
void PuyoGame::setPuyoAt(int X, int Y, PuyoPuyo *newPuyo)
{
    puyoCells[X + Y * PUYODIMX] = newPuyo;
    if (newPuyo != NULL)
        newPuyo->setPuyoXY(X, Y);
}

void PuyoGame::dropNeutrals()
{
  if (neutralPuyos < 0) {
    extern int gameLevel;
    points -= gameLevel * neutralPuyos * 1000;
  }

    while (neutralPuyos > 0)
    {
      int cycleNeutral;
      if (neutralPuyos >=  PUYODIMX)
        cycleNeutral = PUYODIMX;
      else
        cycleNeutral = neutralPuyos;
      for (int i = 0 ; i < cycleNeutral ; i++)
      {
        int posX = fallingTable[(nbFalled++) % PUYODIMX];
        int posY = getFallY(posX, 2);
        neutralPuyos -= 1;
        if (getPuyoCellAt(posX, posY) != PUYO_EMPTY)
            continue;
        // Creating a new neutral puyo
        PuyoPuyo *newNeutral = new PuyoPuyo(PUYO_NEUTRAL);
        puyoVector.addElement(newNeutral);
        setPuyoAt(posX, posY, newNeutral);
        if (delegate != NULL)
          delegate->gameDidAddNeutral(newNeutral);
      }
    }
    neutralPuyos = 0;
}

void PuyoGame::setFallingAtTop(bool gameConstruction)
{
  if (delegate != NULL)
    delegate->gameDidEndCycle();

  if (!gameConstruction) {
    dropNeutrals();
        if (getPuyoCellAt((PUYODIMX-1)/2, 1) != PUYO_EMPTY) {
            gameRunning = false;
            if (delegate != NULL)
                delegate->gameLost();
            return;
        }
  }

  // Creating the new falling puyo and its companion
  fallingX = (PUYODIMX-1)/2;
  fallingY = 1;
  fallingCompanion = 2;
  fallingPuyo = new PuyoPuyo(attachedRandom->getPuyoForSequence(sequenceNr++));
  companionPuyo = new PuyoPuyo(attachedRandom->getPuyoForSequence(sequenceNr++));
  fallingPuyo->setPuyoXY(fallingX, fallingY);
  companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
  puyoVector.addElement(fallingPuyo);
  puyoVector.addElement(companionPuyo);

  endOfCycle = false;
  semiMove = 0;
  phase = 0;
}

int PuyoGame::getFallingCompanionX() const
{
	if (fallingCompanion == 1)
		return fallingX - 1;
	if (fallingCompanion == 3)
		return fallingX + 1;
	return fallingX;
}

int PuyoGame::getFallingCompanionY() const
{
	if (fallingCompanion == 0)
		return fallingY + 1;
	if (fallingCompanion == 2)
		return fallingY - 1;
	return fallingY;
}

int PuyoGame::getFallY(int X, int Y) const
{
	int result = Y + 1;
	while (getPuyoCellAt(X, result) == PUYO_EMPTY)
		result++;
	return result - 1;
}

int PuyoGame::getColumnHeigth(int colNum) const
{
  int result = 0;
  for (int i = 0 ; i < PUYODIMY ; i++) {
    if (getPuyoCellAt(colNum, i) > PUYO_EMPTY)
      result++;
  }
  return result;
}

int PuyoGame::getMaxColumnHeight() const
{
  int max = 0;
  for (int i=0;i<PUYODIMX;++i) {
    int v = getColumnHeigth(i);
    if (v>max) max=v;
  }
  return max;
}

int PuyoGame::getSamePuyoAround(int X, int Y, PuyoState color)
{
  char marked[PUYODIMX][PUYODIMY];
  int  mx[PUYODIMY*PUYODIMX];
  int  my[PUYODIMX*PUYODIMY];
  int  nFound = 1;
  bool again = true;
  mx[0] = X;
  my[0] = Y;

  for (int x=0;x<PUYODIMX;++x)
    for (int y=0;y<PUYODIMY;++y)
      marked[x][y] = 0;
  marked[X][Y] = 1;

  while (again) {
    again = false;
    for (int i=nFound-1;i>=0;--i) {
      X = mx[i];
      Y = my[i];
      if ((Y+1<PUYODIMY)&&!marked[X][Y+1] && (getPuyoCellAt(X,Y+1) == color)) {
          again = true;
          mx[nFound] = X;
          my[nFound] = Y+1;
          marked[X][Y+1] = 1;
          nFound++;
      }
      if ((X+1<PUYODIMX) && !marked[X+1][Y] && (getPuyoCellAt(X+1,Y) == color)) {
          again = true;
          mx[nFound] = X+1;
          my[nFound] = Y;
          marked[X+1][Y] = 1;
          nFound++;
      }
      if ((X-1>=0) && !marked[X-1][Y] && (getPuyoCellAt(X-1,Y) == color)) {
          again = true;
          mx[nFound] = X-1;
          my[nFound] = Y;
          marked[X-1][Y] = 1;
          nFound++;
      }
      if ((Y-1>=0) && !marked[X][Y-1] && (getPuyoCellAt(X,Y-1) == color)) {
          again = true;
          mx[nFound] = X;
          my[nFound] = Y-1;
          marked[X][Y-1] = 1;
          nFound++;
      }
    }
  }
  return nFound;
}

void PuyoGame::markPuyoAt(int X, int Y, PuyoState color, bool includeNeutral)
{
	PuyoState currentPuyo = getPuyoCellAt(X, Y);
	setPuyoCellAt(X, Y, color);
	if (getPuyoCellAt(X-1, Y) == currentPuyo)
		markPuyoAt(X-1, Y, color, includeNeutral);
	if (getPuyoCellAt(X+1, Y) == currentPuyo)
		markPuyoAt(X+1, Y, color, includeNeutral);
	if (getPuyoCellAt(X, Y-1) == currentPuyo)
		markPuyoAt(X, Y-1, color, includeNeutral);
	if (getPuyoCellAt(X, Y+1) == currentPuyo)
		markPuyoAt(X, Y+1, color, includeNeutral);
	if (includeNeutral) {
		if (getPuyoCellAt(X-1, Y) == PUYO_NEUTRAL)
			setPuyoCellAt(X-1, Y, color);
		if (getPuyoCellAt(X+1, Y) == PUYO_NEUTRAL)
			setPuyoCellAt(X+1, Y, color);
		if (getPuyoCellAt(X, Y-1) == PUYO_NEUTRAL)
			setPuyoCellAt(X, Y-1, color);
		if (getPuyoCellAt(X, Y+1) == PUYO_NEUTRAL)
			setPuyoCellAt(X, Y+1, color);
	}
}

// delete the marked puyos and the neutral next to them
void PuyoGame::deleteMarkedPuyosAt(int X, int Y)
{
	PuyoState currentPuyo = getPuyoCellAt(X, Y);
  puyoVector.removeElement(getPuyoAt(X, Y));
  if (getPuyoAt(X,Y) == companionPuyo) {
    delete (getPuyoAt(X, Y));
    companionPuyo = NULL;
  }
  else if (getPuyoAt(X,Y) == fallingPuyo) {
    delete (getPuyoAt(X, Y));
    fallingPuyo = NULL;
  } else {
    delete (getPuyoAt(X, Y));
  }
  setPuyoAt(X, Y, NULL);
	if (getPuyoCellAt(X-1, Y) == currentPuyo)
		deleteMarkedPuyosAt(X-1, Y);
	if (getPuyoCellAt(X+1, Y) == currentPuyo)
		deleteMarkedPuyosAt(X+1, Y);
	if (getPuyoCellAt(X, Y-1) == currentPuyo)
		deleteMarkedPuyosAt(X, Y-1);
	if (getPuyoCellAt(X, Y+1) == currentPuyo)
		deleteMarkedPuyosAt(X, Y+1);
    if (getPuyoCellAt(X-1, Y) == PUYO_NEUTRAL) {
        puyoVector.removeElement(getPuyoAt(X-1, Y));
        delete (getPuyoAt(X-1, Y));
        setPuyoAt(X-1, Y, NULL);
    }
    if (getPuyoCellAt(X+1, Y) == PUYO_NEUTRAL) {
        puyoVector.removeElement(getPuyoAt(X+1, Y));
        delete (getPuyoAt(X+1, Y));
        setPuyoAt(X+1, Y, NULL);
    }
    if (getPuyoCellAt(X, Y-1) == PUYO_NEUTRAL) {
        puyoVector.removeElement(getPuyoAt(X, Y-1));
        delete (getPuyoAt(X, Y-1));
        setPuyoAt(X, Y-1, NULL);
    }
    if (getPuyoCellAt(X, Y+1) == PUYO_NEUTRAL) {
        puyoVector.removeElement(getPuyoAt(X, Y+1));
        delete (getPuyoAt(X, Y+1));
        setPuyoAt(X, Y+1, NULL);
    }
}

int PuyoGame::removePuyos()
{
	int globalRemovedPuyos = 0;
	/* First, we will mark all the puyos that need to be removed */
	for (int i = 0 ; i < PUYODIMX ; i++) {
		for (int j = 0 ; j <= PUYODIMY ; j++) {
			PuyoState currentPuyo = getPuyoCellAt(i, j);
			if ((currentPuyo >= PUYO_BLUE) && (currentPuyo <= PUYO_YELLOW)) {
				int removedPuyos = 0;
				markPuyoAt(i, j, PUYO_MARKED, false);
				for (int u = 0 ; u < PUYODIMX ; u++) {
					for (int v = 0 ; v <= PUYODIMY ; v++) {
						if (getPuyoCellAt(u, v) == PUYO_MARKED) {
							removedPuyos++;
						}
					}
				}
				//printf("Removed for %d, %d : %d\n", i, j, removedPuyos);
				if (removedPuyos >= 4) {
					globalRemovedPuyos += removedPuyos;
					deleteMarkedPuyosAt(i, j);
				}
				else
					markPuyoAt(i, j, currentPuyo, false);
			}
		}
	}

	/* Next we make the other puyos fall */
	int fallPuyo;
	do {
		fallPuyo = 0;
		for (int i = 0 ; i < PUYODIMX ; i++) {
			for (int j = 0 ; j <= PUYODIMY ; j++) {
				PuyoPuyo *currentPuyo = getPuyoAt(i, j);
                PuyoState currentPuyoState = getPuyoCellAt(i, j);
				if ((currentPuyoState >= PUYO_BLUE) && (currentPuyoState <= PUYO_NEUTRAL)) {
					if (getPuyoCellAt(i, j+1) == PUYO_EMPTY) {
						setPuyoAt(i, j, NULL);
						setPuyoAt(i, j+1, currentPuyo);
                        if (delegate != NULL)
                            delegate->puyoDidFall(currentPuyo, i, j);
						fallPuyo++;
					}
				}
			}
		} 
	} while (fallPuyo > 0);
	return globalRemovedPuyos;
}


void PuyoGame::notifyReductions()
{
    for (int i = 0 ; i < PUYODIMX ; i++) {
		for (int j = 0 ; j <= PUYODIMY ; j++) {
			PuyoState currentPuyo = getPuyoCellAt(i, j);
			if ((currentPuyo >= PUYO_BLUE) && (currentPuyo <= PUYO_YELLOW)) {
				int removedPuyos = 0;
				markPuyoAt(i, j, PUYO_MARKED, false);
				for (int u = 0 ; u < PUYODIMX ; u++) {
					for (int v = 0 ; v <= PUYODIMY ; v++) {
						if (getPuyoCellAt(u, v) == PUYO_MARKED) {
                            removedPuyos++;
						}
					}
				}
                if (removedPuyos >= 4) {
                    markPuyoAt(i, j, currentPuyo, false);
                    if (delegate != NULL)
                        delegate->puyoWillVanish(getPuyoAt(i, j));
                }
                else
                    markPuyoAt(i, j, currentPuyo, false);
			}
		}
	}
}

void PuyoGame::cycleEnding()
{
  static int cmpt = 0;
	int score = removePuyos();
	if (score >= 4) {
    audio_sound_play(sound_splash[phase>7?7:phase]);
    if (phase>=2) {
      audio_sound_play(sound_yahoohoo3[(int)((float)NB_YAHOOHOO3 * random()/(RAND_MAX+1.0))]);
    }
    if (phase==1) {
      audio_sound_play(sound_yahoohoo2[(int)((float)NB_YAHOOHOO2 * random()/(RAND_MAX+1.0))]);
    }
    else {
      audio_sound_play(sound_yahoohoo1[(int)((float)NB_YAHOOHOO1 * random()/(RAND_MAX+1.0))]);
    }
    score -= 3;
	  if (phase > 0) {
	    neutralPuyos -= PUYODIMX;
    }
	  phase++;
	}
  
  extern int gameLevel;
  points += gameLevel * 100 + gameLevel * (phase>0?phase-1:0) * 5000;
  
	neutralPuyos -= score;
	if (score == 0)
		setFallingAtTop();
}
