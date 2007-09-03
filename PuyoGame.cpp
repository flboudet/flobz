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
#include "PuyoGame.h"
#include "audio.h"
#include "glSDL.h"
#include "preferences.h"
#include "mt19937ar.h"

static int fallingTable[PUYODIMX] = {0, 3, 1, 4, 2, 5};

PuyoRandomSystem::PuyoRandomSystem()
{
    init_genrand(SDL_GetTicks());
}

PuyoRandomSystem::PuyoRandomSystem(unsigned long seed)
{
    init_genrand(seed);
}

PuyoState PuyoRandomSystem::getPuyoForSequence(int sequence)
{
    if (sequenceItems.size() <= sequence) {
        int newItem = (genrand_int32() % 5) + PUYO_FALLINGBLUE;
        sequenceItems.add(newItem);
        return (PuyoState)newItem;
    }
    else
        return (PuyoState)(sequenceItems[sequence]);
}

int PuyoPuyo::lastID = 0;

// PuyoGame implementation
PuyoGame::PuyoGame()
{
    attachedFactory = new PuyoDefaultFactory();
    delegate = NULL;
}

PuyoGame::PuyoGame(PuyoFactory *attachedFactory) : attachedFactory(attachedFactory), points(0)
{
    delegate = NULL;
}

void PuyoGame::setDelegate(PuyoDelegate *delegate)
{
    this->delegate = delegate;
}

const char * PuyoGame::getPlayerName(int n)
{
  static char playerName[256];
  GetStrPreference(PuyoGame::getDefaultPlayerKey(n), playerName, PuyoGame::getDefaultPlayerName(n));
  return playerName;
}

String PuyoGame::getDefaultPlayerName(int n)
{
  char defaultPlayerName[256];
  if (n <= 0)
  {
    char * defaultName = getenv("USER");
    if ((defaultName == NULL) || (defaultName[0]<32))
    {
      defaultName = getenv("USERNAME");
      if ((defaultName == NULL) || (defaultName[0]<32))
      {            
        sprintf(defaultName,"Kaori");
      }
    }
    
    strncpy(defaultPlayerName,defaultName,255);
    defaultPlayerName[255]=0;
    
    if ((defaultPlayerName[0]>='a') && (defaultPlayerName[0]<='z'))
    {
      defaultPlayerName[0] += 'A' - 'a';
    }
  }
  else
  {
    sprintf(defaultPlayerName,"Player %d",n);
  }
  return String(defaultPlayerName);
}

String PuyoGame::getDefaultPlayerKey(int n)
{
  char playerKey[50];
  sprintf(playerKey,"Game.Player.Name.%d",n);
  return String(playerKey);
}

void PuyoGame::setDefaultPlayerName(int n, const char * playerName)
{
  static char playerKey[50];
  sprintf(playerKey,"Game.Player.Name.%d",n);  
  SetStrPreference(playerKey, playerName);
}


// PuyoLocalGame implementation
PuyoLocalGame::PuyoLocalGame(PuyoRandomSystem *attachedRandom,
           PuyoFactory *attachedFactory) : PuyoGame(attachedFactory)
{
  InitGame(attachedRandom);
}

PuyoLocalGame::PuyoLocalGame(PuyoRandomSystem *attachedRandom) : PuyoGame()
{
  InitGame(attachedRandom);
}

void PuyoLocalGame::InitGame(PuyoRandomSystem *attachedRandom)
{
  gameLevel = 1;
  nbFalled = 0;
    int i, j;
    unmoveablePuyo = attachedFactory->createPuyo(PUYO_UNMOVEABLE);
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
    
    endOfCycle = false;
    gameRunning = true;
    setFallingAtTop(true);
    points = 0;
}

PuyoLocalGame::~PuyoLocalGame()
{
    delete unmoveablePuyo;
    for (int i = 0 ; i < puyoVector.size() ; i++) {
        delete puyoVector[i];
    }
}

void PuyoLocalGame::cycle()
{
  if (!gameRunning)
    return;

  // Increase points
  switch (gameLevel)
  {
  case 1:
      points += 1;
      break;
  case 2:
      points += 5;
      break;
  case 3:
      points += 10;
      break;
  default:
      points += gameLevel * 5;
  }

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
PuyoState PuyoLocalGame::getPuyoCellAt(int X, int Y) const
{
    PuyoPuyo *thePuyo = getPuyoAt(X, Y);
    if (thePuyo)
        return thePuyo->getPuyoState();
    return PUYO_EMPTY;
}

// Get the puyo at the indicated coordinates
PuyoPuyo *PuyoLocalGame::getPuyoAt(int X, int Y) const
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
int PuyoLocalGame::getPuyoCount() const
{
    return puyoVector.size();
}

PuyoPuyo *PuyoLocalGame::getPuyoAtIndex(int index) const
{
    return puyoVector[index];
}

void PuyoLocalGame::moveLeft()
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

void PuyoLocalGame::moveRight()
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

void PuyoLocalGame::rotate(bool left)
{
    if (endOfCycle) {
        return;
    }
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

void PuyoLocalGame::rotateLeft()
{
    rotate(true);
}

void PuyoLocalGame::rotateRight()
{
    rotate(false);
}

PuyoState PuyoLocalGame::getNextFalling()
{
    return attachedRandom->getPuyoForSequence(sequenceNr);
}

PuyoState PuyoLocalGame::getNextCompanion()
{
    return attachedRandom->getPuyoForSequence(sequenceNr+1);
}

void PuyoLocalGame::increaseNeutralPuyos(int incr)
{
    neutralPuyos += incr;
}

int PuyoLocalGame::getNeutralPuyos() const
{
    return neutralPuyos;
}



// Set the state of the puyo at the indicated coordinates (not recommanded)
void PuyoLocalGame::setPuyoCellAt(int X, int Y, PuyoState value)
{
    /*if ((X > PUYODIMX) || (Y > PUYODIMY))
        return;*/
  if (puyoCells[X + Y * PUYODIMX])
    puyoCells[X + Y * PUYODIMX]->setPuyoState(value);
};

// Set the puyo at the indicated coordinates
void PuyoLocalGame::setPuyoAt(int X, int Y, PuyoPuyo *newPuyo)
{
    puyoCells[X + Y * PUYODIMX] = newPuyo;
    if (newPuyo != NULL)
        newPuyo->setPuyoXY(X, Y);
}

void PuyoLocalGame::dropNeutrals()
{
  if (neutralPuyos < 0) {
    points -= gameLevel * neutralPuyos * 1000;
  }

    int idNeutral = 0;
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
        PuyoPuyo *newNeutral = attachedFactory->createPuyo(PUYO_NEUTRAL);
        puyoVector.add(newNeutral);
        setPuyoAt(posX, posY, newNeutral);
        if (delegate != NULL)
          delegate->gameDidAddNeutral(newNeutral, idNeutral++);
      }
    }
    neutralPuyos = 0;
}

void PuyoLocalGame::setFallingAtTop(bool gameConstruction)
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
  fallingPuyo = attachedFactory->createPuyo(attachedRandom->getPuyoForSequence(sequenceNr++));
  companionPuyo = attachedFactory->createPuyo(attachedRandom->getPuyoForSequence(sequenceNr++));
  fallingPuyo->setPuyoXY(fallingX, fallingY);
  companionPuyo->setPuyoXY(getFallingCompanionX(), getFallingCompanionY());
  puyoVector.add(fallingPuyo);
  puyoVector.add(companionPuyo);

  endOfCycle = false;
  semiMove = 0;
  phase = 0;
}

int PuyoLocalGame::getFallingCompanionX() const
{
    if (fallingCompanion == 1)
        return fallingX - 1;
    if (fallingCompanion == 3)
        return fallingX + 1;
    return fallingX;
}

int PuyoLocalGame::getFallingCompanionY() const
{
    if (fallingCompanion == 0)
        return fallingY + 1;
    if (fallingCompanion == 2)
        return fallingY - 1;
    return fallingY;
}

int PuyoLocalGame::getFallY(int X, int Y) const
{
    int result = Y + 1;
    while (getPuyoCellAt(X, result) == PUYO_EMPTY)
        result++;
    return result - 1;
}

int PuyoLocalGame::getColumnHeigth(int colNum) const
{
  int result = 0;
  for (int i = 0 ; i < PUYODIMY ; i++) {
    if (getPuyoCellAt(colNum, i) > PUYO_EMPTY)
      result++;
  }
  return result;
}

int PuyoLocalGame::getMaxColumnHeight() const
{
  int max = 0;
  for (int i=0;i<PUYODIMX;++i) {
    int v = getColumnHeigth(i);
    if (v>max) max=v;
  }
  return max;
}

int PuyoLocalGame::getSamePuyoAround(int X, int Y, PuyoState color)
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

void PuyoLocalGame::markPuyoAt(int X, int Y, bool mark, bool includeNeutral)
{
    PuyoPuyo *currentPuyo = getPuyoAt(X, Y);
    if (currentPuyo->isMarked() == mark)
        return;
    PuyoState currentPuyoState = getPuyoCellAt(X, Y);
    currentPuyo->setMark(mark);
    if (getPuyoCellAt(X-1, Y) == currentPuyoState)
        markPuyoAt(X-1, Y, mark, includeNeutral);
    if (getPuyoCellAt(X+1, Y) == currentPuyoState)
        markPuyoAt(X+1, Y, mark, includeNeutral);
    if (getPuyoCellAt(X, Y-1) == currentPuyoState)
        markPuyoAt(X, Y-1, mark, includeNeutral);
    if (getPuyoCellAt(X, Y+1) == currentPuyoState)
        markPuyoAt(X, Y+1, mark, includeNeutral);
    if (includeNeutral) {
        if (getPuyoCellAt(X-1, Y) == PUYO_NEUTRAL)
            getPuyoAt(X-1, Y)->setMark(mark);
        if (getPuyoCellAt(X+1, Y) == PUYO_NEUTRAL)
            getPuyoAt(X+1, Y)->setMark(mark);
        if (getPuyoCellAt(X, Y-1) == PUYO_NEUTRAL)
            getPuyoAt(X, Y-1)->setMark(mark);
        if (getPuyoCellAt(X, Y+1) == PUYO_NEUTRAL)
            getPuyoAt(X, Y+1)->setMark(mark);
    }
}

// delete the marked puyos and the neutral next to them
void PuyoLocalGame::deleteMarkedPuyosAt(int X, int Y)
{
  puyoVector.remove(getPuyoAt(X, Y));
  if (getPuyoAt(X,Y) == companionPuyo) {
    attachedFactory->deletePuyo(getPuyoAt(X, Y));
    companionPuyo = NULL;
  }
  else if (getPuyoAt(X,Y) == fallingPuyo) {
    attachedFactory->deletePuyo(getPuyoAt(X, Y));
    fallingPuyo = NULL;
  } else {
    attachedFactory->deletePuyo(getPuyoAt(X, Y));
  }
  setPuyoAt(X, Y, NULL);
    if (getPuyoAt(X-1, Y)->isMarked())
        deleteMarkedPuyosAt(X-1, Y);
    if (getPuyoAt(X+1, Y)->isMarked())
        deleteMarkedPuyosAt(X+1, Y);
    if (getPuyoAt(X, Y-1)->isMarked())
        deleteMarkedPuyosAt(X, Y-1);
    if (getPuyoAt(X, Y+1)->isMarked())
        deleteMarkedPuyosAt(X, Y+1);
    if (getPuyoCellAt(X-1, Y) == PUYO_NEUTRAL) {
        puyoVector.remove(getPuyoAt(X-1, Y));
        attachedFactory->deletePuyo(getPuyoAt(X-1, Y));
        setPuyoAt(X-1, Y, NULL);
    }
    if (getPuyoCellAt(X+1, Y) == PUYO_NEUTRAL) {
        puyoVector.remove(getPuyoAt(X+1, Y));
        attachedFactory->deletePuyo(getPuyoAt(X+1, Y));
        setPuyoAt(X+1, Y, NULL);
    }
    if (getPuyoCellAt(X, Y-1) == PUYO_NEUTRAL) {
        puyoVector.remove(getPuyoAt(X, Y-1));
        attachedFactory->deletePuyo(getPuyoAt(X, Y-1));
        setPuyoAt(X, Y-1, NULL);
    }
    if (getPuyoCellAt(X, Y+1) == PUYO_NEUTRAL) {
        puyoVector.remove(getPuyoAt(X, Y+1));
        attachedFactory->deletePuyo(getPuyoAt(X, Y+1));
        setPuyoAt(X, Y+1, NULL);
    }
}

int PuyoLocalGame::removePuyos()
{
    int globalRemovedPuyos = 0;
    /* First, we will mark all the puyos that need to be removed */
    for (int i = 0 ; i < PUYODIMX ; i++) {
        for (int j = 0 ; j <= PUYODIMY ; j++) {
            PuyoState currentPuyo = getPuyoCellAt(i, j);
            if ((currentPuyo >= PUYO_BLUE) && (currentPuyo <= PUYO_YELLOW)) {
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
    /* Next we make the other puyos fall */
    for (int i = 0 ; i < PUYODIMX ; i++) {
        for (int j = PUYODIMY - 1 ; j > 0 ; j--) {
            PuyoState currentPuyoState = getPuyoCellAt(i, j);
            if ((currentPuyoState >= PUYO_BLUE) && (currentPuyoState <= PUYO_NEUTRAL)) {
                int newJ = getFallY(i, j);
                if (newJ != j) {
                    PuyoPuyo *currentPuyo = getPuyoAt(i, j);
                    setPuyoAt(i, j, NULL);
                    setPuyoAt(i, newJ, currentPuyo);
                    if (delegate != NULL) {
                        delegate->puyoDidFall(currentPuyo, i, j);
                    }
                }
            }
        }
    }
    return globalRemovedPuyos;
}


void PuyoLocalGame::notifyReductions()
{
    AdvancedBuffer<PuyoPuyo *> removedPuyos;
    // Clearing every puyo's flag
    for (int i = 0, j = getPuyoCount() ; i < j ; i++) {
        getPuyoAtIndex(i)->unsetFlag();
    }
    // Search for groupped puyos
    int puyoGroupNumber = 0;
    for (int j = 0 ; j < PUYODIMY ; j++) {
        for (int i = 0 ; i <= PUYODIMX ; i++) {
            PuyoPuyo *puyoToMark = getPuyoAt(i, j);
            // If the puyo exists and is not flagged, then
            if ((puyoToMark != NULL) && (! puyoToMark->getFlag())) {
                PuyoState initialPuyoState = puyoToMark->getPuyoState();
                // I really would have liked to skip this stupid test
                if ((initialPuyoState >= PUYO_BLUE) && (initialPuyoState <= PUYO_YELLOW)) {
                    markPuyoAt(i, j, true, true);

                    // Collecting every marked puyo in a vector
                    removedPuyos.clear();
                    int removedCount = 0;
                    for (int u = 0, v = puyoVector.size() ; u < v ; u++) {
                        PuyoPuyo *markedPuyo = puyoVector[u];
                        if (markedPuyo->isMarked()) {
                            markedPuyo->setFlag();
                            removedPuyos.add(markedPuyo);
                            if (markedPuyo->getPuyoState() != PUYO_NEUTRAL)
                                removedCount++;
                        }
                    }
                    // If there is more than 4 puyo in the group, let's notify it
                    if (removedCount >= 4) {
                        markPuyoAt(i, j, false, true);
                        if (delegate != NULL)
                            delegate->puyoWillVanish(removedPuyos, puyoGroupNumber++, phase);
                    }
                    else {
                        markPuyoAt(i, j, false, true);
                    }
                }
            }
        }
    }
}

void PuyoLocalGame::cycleEnding()
{
    static int cmpt = 0;
    int score = removePuyos();
    
    if (score >= 4) {
#ifdef DESACTIVE
        audio_sound_play(sound_splash[phase>7?7:phase]);
#endif
        score -= 3;
        if (phase > 0) {
            neutralPuyos -= PUYODIMX;
        }
        phase++;
    }
  
    points += gameLevel * 100 + gameLevel * (phase>0?phase-1:0) * 5000;
  
    neutralPuyos -= score;
    if (score == 0)
        setFallingAtTop();
}
