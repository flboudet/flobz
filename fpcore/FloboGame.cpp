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

#include <stdio.h>
#include <stdlib.h>
#include "FloboGame.h"
#include "audio.h"
#include "mt19937ar.h"

static int fallingTable[FLOBOBAN_DIMX] = {0, 3, 1, 4, 2, 5};

RandomSystem::RandomSystem(int numColors) : numColors(numColors)
{
    init_genrand(fmod(ios_fc::getTimeMs(), 100000.));
}

RandomSystem::RandomSystem(unsigned long seed, int numColors) : numColors(numColors)
{
    init_genrand(seed);
}

FloboState RandomSystem::getFloboStateForSequence(int sequence)
{
    if (sequenceItems.size() <= sequence) {
        int newItem = (genrand_int32() % numColors) + FLOBO_FALLINGBLUE;
        sequenceItems.add(newItem);
        return (FloboState)newItem;
    }
    else
        return (FloboState)(sequenceItems[sequence]);
}

int Flobo::lastID = 0;

// FloboGame implementation
FloboGame::FloboGame()
{
    attachedFactory = new FloboDefaultFactory();
}

FloboGame::FloboGame(FloboFactory *attachedFactory) : attachedFactory(attachedFactory)
{
}

void FloboGame::addGameListener(GameListener *listener)
{
    m_listeners.push_back(listener);
}

// FloboLocalGame implementation
FloboLocalGame::FloboLocalGame(RandomSystem *attachedRandom,
           FloboFactory *attachedFactory) : FloboGame(attachedFactory)
{
  InitGame(attachedRandom);
}

FloboLocalGame::FloboLocalGame(RandomSystem *attachedRandom) : FloboGame()
{
  InitGame(attachedRandom);
}

void FloboLocalGame::InitGame(RandomSystem *attachedRandom)
{
    gameLevel = 1;
    nbFalled = 0;
    phaseReady = 2;
    int i, j;
    unmoveablePuyo = attachedFactory->createFlobo(FLOBO_UNMOVEABLE);
    for (i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        for (j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            if (j == FLOBOBAN_DIMY)
                setPuyoAt(i, j, unmoveablePuyo);
            else
                setPuyoAt(i, j, NULL);
        }
    }
    this->attachedRandom = attachedRandom;
    sequenceNr = 0;
    semiMove = 0;
    neutralFlobos = 0;

    endOfCycle = false;
    gameRunning = true;
    setFallingAtTop(true);

    gameStat = PlayerGameStat();
}

FloboLocalGame::~FloboLocalGame()
{
    delete unmoveablePuyo;
    for (int i = 0 ; i < puyoVector.size() ; i++) {
        delete puyoVector[i];
    }
}

void FloboLocalGame::cycle()
{
    if (!gameRunning)
        return;

    semiMove = 1 - semiMove;
    if (semiMove == 0) {
        if (!endOfCycle) {
            for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                 iter != m_listeners.end() ; ++iter)
                (*iter)->fallingsDidFallingStep(fallingFlobo, companionFlobo);
        }
        return;
    }
    if (endOfCycle) {
        cycleEnding();
        notifyReductions();
        return;
    }
    if ((fallingY >= 0)&&(getPuyoCellAt(fallingX, fallingY+1) > FLOBO_EMPTY) || (getPuyoCellAt(getFallingCompanionX(), getFallingCompanionY()+1) > FLOBO_EMPTY)) {
        setPuyoAt(fallingX, getFallY(fallingX, fallingY), fallingFlobo);
        fallingFlobo->setFloboState((FloboState)(fallingFlobo->getFloboState()+FLOBO_STILL));
        setPuyoAt(getFallingCompanionX(), getFallY(getFallingCompanionX(), getFallingCompanionY()), companionFlobo);
        companionFlobo->setFloboState((FloboState)(companionFlobo->getFloboState()+FLOBO_STILL));
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter) {
            (*iter)->floboDidFall(fallingFlobo, fallingX, fallingY, fallingCompanion == 0 ? 1 : 0);
            (*iter)->floboDidFall(companionFlobo, getFallingCompanionX(), getFallingCompanionY(), fallingCompanion == 0 ? 0 : 1);
        }
        gameStat.drop_count += 2;
        fallingY = -10;
        notifyReductions();
        endOfCycle = true;
    }
    else {
        fallingY++;
        if (phaseReady == 1) phaseReady = 2;
        fallingFlobo->setFloboXY(fallingX, fallingY);
        companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter) {
			(*iter)->fallingsDidFallingStep(fallingFlobo, companionFlobo);
		}
        // Increase points
        switch (gameLevel)
        {
            case 1:
                gameStat.points += 1;
                break;
            case 2:
                gameStat.points += 5;
                break;
            case 3:
                gameStat.points += 10;
                break;
            default:
                gameStat.points += gameLevel * 5;
        }
    }
}

// Get the state of the puyo at the indicated coordinates
FloboState FloboLocalGame::getPuyoCellAt(int X, int Y) const
{
    Flobo *thePuyo = getFloboAt(X, Y);
    if (thePuyo)
        return thePuyo->getFloboState();
    return FLOBO_EMPTY;
}

// Get the puyo at the indicated coordinates
Flobo *FloboLocalGame::getFloboAt(int X, int Y) const
{
    if ((X >= FLOBOBAN_DIMX) || (Y >= FLOBOBAN_DIMY) || (X < 0) || (Y < 0))
        return unmoveablePuyo;
    if (!endOfCycle) {
        if ((X == fallingX) && (Y == fallingY))
            return fallingFlobo;
        if ((X == getFallingCompanionX()) && (Y == getFallingCompanionY()))
            return companionFlobo;
    }
    return puyoCells[X + Y * FLOBOBAN_DIMX];
}

// List access to the Flobo objects
int FloboLocalGame::getFloboCount() const
{
    return puyoVector.size();
}

Flobo *FloboLocalGame::getFloboAtIndex(int index) const
{
    return puyoVector[index];
}

void FloboLocalGame::moveLeft()
{
    if (endOfCycle) {
        return;
    }
    bool moved = false;
    if (((fallingY<0)&&(fallingX>0))||((getPuyoCellAt(fallingX-1, fallingY) <= FLOBO_EMPTY)
     && (getPuyoCellAt(getFallingCompanionX()-1, getFallingCompanionY()) <= FLOBO_EMPTY))) {
		moved = true;
		fallingX--;
    }

    fallingFlobo->setFloboXY(fallingX, fallingY);
    companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter)
        (*iter)->fallingsDidMoveLeft(fallingFlobo, companionFlobo);
}

void FloboLocalGame::moveRight()
{
    if (endOfCycle) {
        return;
    }
    bool moved = false;
    if (((fallingY<0)&&(fallingX<FLOBOBAN_DIMX-1))||((getPuyoCellAt(fallingX+1, fallingY) <= FLOBO_EMPTY)
        && (getPuyoCellAt(getFallingCompanionX()+1, getFallingCompanionY()) <= FLOBO_EMPTY))) {
	moved = true;
        fallingX++;
    }
    fallingFlobo->setFloboXY(fallingX, fallingY);
    companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter)
            (*iter)->fallingsDidMoveRight(fallingFlobo, companionFlobo);
}

void FloboLocalGame::rotate(bool left)
{
    if (endOfCycle) {
        return;
    }
    unsigned char backupCompanion = fallingCompanion;
    int newX, newY;
    bool moved = true;
    fallingCompanion = (unsigned char)(fallingCompanion + (left?3:1)) % 4;
    unsigned char newCompanion = fallingCompanion;
    int newCompanionX = getFallingCompanionX();
    int newCompanionY = getFallingCompanionY();
    fallingCompanion = backupCompanion;
    if (getPuyoCellAt(newCompanionX, newCompanionY) > FLOBO_EMPTY) {
        newX = fallingX + (fallingX - newCompanionX);
        newY = fallingY + (fallingY - newCompanionY);
        if (getPuyoCellAt(newX, newY) > FLOBO_EMPTY) {
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
            fallingFlobo->setFloboXY(fallingX, fallingY);
        }
        }
    else
        fallingCompanion = newCompanion;

    companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter)
            (*iter)->companionDidTurn(companionFlobo, fallingFlobo, !left);

}

void FloboLocalGame::rotateLeft()
{
    rotate(true);
}

void FloboLocalGame::rotateRight()
{
    rotate(false);
}

FloboState FloboLocalGame::getNextFalling()
{
    return attachedRandom->getFloboStateForSequence(sequenceNr);
}

FloboState FloboLocalGame::getNextCompanion()
{
    return attachedRandom->getFloboStateForSequence(sequenceNr+1);
}

void FloboLocalGame::increaseNeutralFlobos(int incr)
{
    neutralFlobos += incr;
}

int FloboLocalGame::getNeutralFlobos() const
{
    return neutralFlobos;
}



// Set the state of the puyo at the indicated coordinates (not recommanded)
void FloboLocalGame::setPuyoCellAt(int X, int Y, FloboState value)
{
    /*if ((X > FLOBOBAN_DIMX) || (Y > FLOBOBAN_DIMY))
        return;*/
  if (puyoCells[X + Y * FLOBOBAN_DIMX])
    puyoCells[X + Y * FLOBOBAN_DIMX]->setFloboState(value);
};

// Set the puyo at the indicated coordinates
void FloboLocalGame::setPuyoAt(int X, int Y, Flobo *newPuyo)
{
    puyoCells[X + Y * FLOBOBAN_DIMX] = newPuyo;
    if (newPuyo != NULL)
        newPuyo->setFloboXY(X, Y);
}

int FloboLocalGame::getGameTotalNeutralFlobos() const
{
    return nbFalled;
}

void FloboLocalGame::dropNeutrals()
{
    if (neutralFlobos < 0) {
        gameStat.points -= gameLevel * neutralFlobos * 1000;
    }
    int totalNeutral = neutralFlobos;
    int idNeutral = 0;
    while (neutralFlobos > 0)
    {
      int cycleNeutral;
      if (neutralFlobos >= FLOBOBAN_DIMX)
        cycleNeutral = FLOBOBAN_DIMX;
      else
        cycleNeutral = neutralFlobos;
      for (int i = 0 ; i < cycleNeutral ; i++)
      {
        int posX = fallingTable[(nbFalled++) % FLOBOBAN_DIMX];
        int posY = getFallY(posX, 2);
        neutralFlobos -= 1;
        if (getPuyoCellAt(posX, posY) != FLOBO_EMPTY)
            continue;
        // Creating a new neutral puyo
        Flobo *newNeutral = attachedFactory->createFlobo(FLOBO_NEUTRAL);
        puyoVector.add(newNeutral);
        setPuyoAt(posX, posY, newNeutral);
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter)
            (*iter)->gameDidAddNeutral(newNeutral, idNeutral++, totalNeutral);
      }
    }
    neutralFlobos = 0;
  phaseReady = 1;
}

bool FloboLocalGame::isPhaseReady(void)
{
  bool r = (phaseReady == 2);
  if (r) phaseReady = 0;
  return r;
}

void FloboLocalGame::addNeutralLayer()
{
    // Raise everything up
    for (int j = 1 ; j < FLOBOBAN_DIMY ; ++j) {
        for (int i = 0 ; i < FLOBOBAN_DIMX ; ++i) {
            Flobo *currentPuyo = getFloboAt(i, j);
            if (currentPuyo != NULL) {
                setPuyoAt(i, j, NULL);
                setPuyoAt(i, j-1, currentPuyo);
                for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                     iter != m_listeners.end() ; ++iter)
                    (*iter)->floboDidFall(currentPuyo, i, j, 1);
            }
        }
    }
    for (int i = 0 ; i < FLOBOBAN_DIMX ; ++i) {
        Flobo *newNeutral = attachedFactory->createFlobo(FLOBO_NEUTRAL);
        puyoVector.add(newNeutral);
        setPuyoAt(i, FLOBOBAN_DIMY-1, newNeutral);
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter)
            (*iter)->floboDidFall(newNeutral, i, FLOBOBAN_DIMY, 1);
    }
}

void FloboLocalGame::setFallingAtTop(bool gameConstruction)
{
    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
         iter != m_listeners.end() ; ++iter)
        (*iter)->gameDidEndCycle();

    if (!gameConstruction) {
        dropNeutrals();
        if (getPuyoCellAt((FLOBOBAN_DIMX-1)/2, 1) != FLOBO_EMPTY) {
            gameRunning = false;
            for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                 iter != m_listeners.end() ; ++iter)
                (*iter)->gameLost();
            return;
        }
    }

    // Creating the new falling puyo and its companion
    fallingX = (FLOBOBAN_DIMX-1)/2;
    fallingY = 1;
    fallingCompanion = 2;
    fallingFlobo = attachedFactory->createFlobo(attachedRandom->getFloboStateForSequence(sequenceNr++));
    companionFlobo = attachedFactory->createFlobo(attachedRandom->getFloboStateForSequence(sequenceNr++));
    fallingFlobo->setFloboXY(fallingX, fallingY);
    companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());
    puyoVector.add(fallingFlobo);
    puyoVector.add(companionFlobo);

    endOfCycle = false;
    semiMove = 0;
    phase = 0;
}

int FloboLocalGame::getFallingCompanionX() const
{
    if (fallingCompanion == 1)
        return fallingX - 1;
    if (fallingCompanion == 3)
        return fallingX + 1;
    return fallingX;
}

int FloboLocalGame::getFallingCompanionY() const
{
    if (fallingCompanion == 0)
        return fallingY + 1;
    if (fallingCompanion == 2)
        return fallingY - 1;
    return fallingY;
}

int FloboLocalGame::getFallY(int X, int Y) const
{
    int result = Y + 1;
    while (getPuyoCellAt(X, result) == FLOBO_EMPTY)
        result++;
    return result - 1;
}

int FloboLocalGame::getColumnHeigth(int colNum) const
{
  int result = 0;
  for (int i = 0 ; i < FLOBOBAN_DIMY ; i++) {
    if (getPuyoCellAt(colNum, i) > FLOBO_EMPTY)
      result++;
  }
  return result;
}

int FloboLocalGame::getMaxColumnHeight() const
{
  int max = 0;
  for (int i=0;i<FLOBOBAN_DIMX;++i) {
    int v = getColumnHeigth(i);
    if (v>max) max=v;
  }
  return max;
}

int FloboLocalGame::getSameFloboAround(int X, int Y, FloboState color)
{
  char marked[FLOBOBAN_DIMX][FLOBOBAN_DIMY];
  int  mx[FLOBOBAN_DIMY*FLOBOBAN_DIMX];
  int  my[FLOBOBAN_DIMX*FLOBOBAN_DIMY];
  int  nFound = 1;
  bool again = true;
  mx[0] = X;
  my[0] = Y;

  for (int x=0;x<FLOBOBAN_DIMX;++x)
    for (int y=0;y<FLOBOBAN_DIMY;++y)
      marked[x][y] = 0;
  marked[X][Y] = 1;

  while (again) {
    again = false;
    for (int i=nFound-1;i>=0;--i) {
      X = mx[i];
      Y = my[i];
      if ((Y+1<FLOBOBAN_DIMY)&&!marked[X][Y+1] && (getPuyoCellAt(X,Y+1) == color)) {
          again = true;
          mx[nFound] = X;
          my[nFound] = Y+1;
          marked[X][Y+1] = 1;
          nFound++;
      }
      if ((X+1<FLOBOBAN_DIMX) && !marked[X+1][Y] && (getPuyoCellAt(X+1,Y) == color)) {
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

void FloboLocalGame::markPuyoAt(int X, int Y, bool mark, bool includeNeutral)
{
    Flobo *currentPuyo = getFloboAt(X, Y);
    if (currentPuyo->isMarked() == mark)
        return;
    FloboState currentFloboState = getPuyoCellAt(X, Y);
    currentPuyo->setMark(mark);
    if (getPuyoCellAt(X-1, Y) == currentFloboState)
        markPuyoAt(X-1, Y, mark, includeNeutral);
    if (getPuyoCellAt(X+1, Y) == currentFloboState)
        markPuyoAt(X+1, Y, mark, includeNeutral);
    if (getPuyoCellAt(X, Y-1) == currentFloboState)
        markPuyoAt(X, Y-1, mark, includeNeutral);
    if (getPuyoCellAt(X, Y+1) == currentFloboState)
        markPuyoAt(X, Y+1, mark, includeNeutral);
    if (includeNeutral) {
        if (getPuyoCellAt(X-1, Y) == FLOBO_NEUTRAL)
            getFloboAt(X-1, Y)->setMark(mark);
        if (getPuyoCellAt(X+1, Y) == FLOBO_NEUTRAL)
            getFloboAt(X+1, Y)->setMark(mark);
        if (getPuyoCellAt(X, Y-1) == FLOBO_NEUTRAL)
            getFloboAt(X, Y-1)->setMark(mark);
        if (getPuyoCellAt(X, Y+1) == FLOBO_NEUTRAL)
            getFloboAt(X, Y+1)->setMark(mark);
    }
}

// delete the marked flobos and the neutral next to them
void FloboLocalGame::deleteMarkedPuyosAt(int X, int Y)
{
  puyoVector.remove(getFloboAt(X, Y));
  if (getFloboAt(X,Y) == companionFlobo) {
    attachedFactory->deleteFlobo(getFloboAt(X, Y));
    companionFlobo = NULL;
  }
  else if (getFloboAt(X,Y) == fallingFlobo) {
    attachedFactory->deleteFlobo(getFloboAt(X, Y));
    fallingFlobo = NULL;
  } else {
    attachedFactory->deleteFlobo(getFloboAt(X, Y));
  }
  setPuyoAt(X, Y, NULL);
    if (getFloboAt(X-1, Y)->isMarked())
        deleteMarkedPuyosAt(X-1, Y);
    if (getFloboAt(X+1, Y)->isMarked())
        deleteMarkedPuyosAt(X+1, Y);
    if (getFloboAt(X, Y-1)->isMarked())
        deleteMarkedPuyosAt(X, Y-1);
    if (getFloboAt(X, Y+1)->isMarked())
        deleteMarkedPuyosAt(X, Y+1);
    if (getPuyoCellAt(X-1, Y) == FLOBO_NEUTRAL) {
        puyoVector.remove(getFloboAt(X-1, Y));
        attachedFactory->deleteFlobo(getFloboAt(X-1, Y));
        setPuyoAt(X-1, Y, NULL);
    }
    if (getPuyoCellAt(X+1, Y) == FLOBO_NEUTRAL) {
        puyoVector.remove(getFloboAt(X+1, Y));
        attachedFactory->deleteFlobo(getFloboAt(X+1, Y));
        setPuyoAt(X+1, Y, NULL);
    }
    if (getPuyoCellAt(X, Y-1) == FLOBO_NEUTRAL) {
        puyoVector.remove(getFloboAt(X, Y-1));
        attachedFactory->deleteFlobo(getFloboAt(X, Y-1));
        setPuyoAt(X, Y-1, NULL);
    }
    if (getPuyoCellAt(X, Y+1) == FLOBO_NEUTRAL) {
        puyoVector.remove(getFloboAt(X, Y+1));
        attachedFactory->deleteFlobo(getFloboAt(X, Y+1));
        setPuyoAt(X, Y+1, NULL);
    }
}

int FloboLocalGame::removePuyos()
{
    int globalRemovedPuyos = 0;
    /* First, we will mark all the flobos that need to be removed */
    for (int i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            FloboState currentPuyo = getPuyoCellAt(i, j);
            if ((currentPuyo >= FLOBO_BLUE) && (currentPuyo <= FLOBO_YELLOW)) {
                int removedPuyos = 0;
                markPuyoAt(i, j, true, false);
                for (int u = 0, v = puyoVector.size() ; u < v ; u++) {
                    if (puyoVector[u]->isMarked()) {
                        removedPuyos++;
                    }
                }
                //printf("Removed for %d, %d : %d\n", i, j, removedPuyos);
                if (removedPuyos >= 4) {
                    globalRemovedPuyos += removedPuyos;
                    deleteMarkedPuyosAt(i, j);
                }
                else
                    markPuyoAt(i, j, false, false);
            }
        }
    }
    /* Next we make the other flobos fall */
    for (int i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        int feltBelow = 0;
        for (int j = FLOBOBAN_DIMY - 1 ; j > 0 ; j--) {
            FloboState currentFloboState = getPuyoCellAt(i, j);
            if ((currentFloboState >= FLOBO_BLUE) && (currentFloboState <= FLOBO_NEUTRAL)) {
                int newJ = getFallY(i, j);
                if (newJ != j) {
                    Flobo *currentPuyo = getFloboAt(i, j);
                    setPuyoAt(i, j, NULL);
                    setPuyoAt(i, newJ, currentPuyo);
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter) {
                        (*iter)->floboDidFall(currentPuyo, i, j, feltBelow);
                        feltBelow++;
                    }
                }
            }
        }
    }
    return globalRemovedPuyos;
}


void FloboLocalGame::notifyReductions()
{
    AdvancedBuffer<Flobo *> removedPuyos;
    // Clearing every puyo's flag
    for (int i = 0, j = getFloboCount() ; i < j ; i++) {
        getFloboAtIndex(i)->unsetFlag();
    }
    // Search for groupped flobos
    int floboGroupNumber = 0;
    for (int j = 0 ; j < FLOBOBAN_DIMY ; j++) {
        for (int i = 0 ; i <= FLOBOBAN_DIMX ; i++) {
            Flobo *puyoToMark = getFloboAt(i, j);
            // If the puyo exists and is not flagged, then
            if ((puyoToMark != NULL) && (! puyoToMark->getFlag())) {
                FloboState initialFloboState = puyoToMark->getFloboState();
                // I really would have liked to skip this stupid test
                if ((initialFloboState >= FLOBO_BLUE) && (initialFloboState <= FLOBO_YELLOW)) {
                    markPuyoAt(i, j, true, true);

                    // Collecting every marked puyo in a vector
                    removedPuyos.clear();
                    int removedCount = 0;
                    for (int u = 0, v = puyoVector.size() ; u < v ; u++) {
                        Flobo *markedPuyo = puyoVector[u];
                        if (markedPuyo->isMarked()) {
                            markedPuyo->setFlag();
                            removedPuyos.add(markedPuyo);
                            if (markedPuyo->getFloboState() != FLOBO_NEUTRAL)
                                removedCount++;
                        }
                    }
                    // If there is more than 4 puyo in the group, let's notify it
                    if (removedCount >= 4) {
                        markPuyoAt(i, j, false, true);
                        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                             iter != m_listeners.end() ; ++iter)
                            (*iter)->floboWillVanish(removedPuyos, floboGroupNumber++, phase+1);
                    }
                    else {
                        markPuyoAt(i, j, false, true);
                    }
                }
            }
        }
    }
}

void FloboLocalGame::cycleEnding()
{
    int score = removePuyos();
    gameStat.explode_count += score;

    if (score >= 4) {
#ifdef DESACTIVE
        audio_sound_play(sound_splash[phase>7?7:phase]);
#endif
        score -= 3;
        if (phase > 0) {
            neutralFlobos -= FLOBOBAN_DIMX;
            gameStat.ghost_sent_count += FLOBOBAN_DIMX;
        }
        ++gameStat.combo_count[phase];
        phase++;
    }

    gameStat.points += gameLevel * 100 + gameLevel * (phase>0?phase-1:0) * 5000;

    neutralFlobos -= score;
    gameStat.ghost_sent_count += score;

    if (score == 0)
        setFallingAtTop();
}
