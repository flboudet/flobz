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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "FloboGame.h"
#include "audio.h"
#include "mt19937ar.h"

static void removeFloboFromVector(std::vector<std::shared_ptr<Flobo>> &container, std::shared_ptr<Flobo> flobo)
{
    auto it = std::find(container.begin(), container.end(), flobo);
    if (it != container.end()) {
        container.erase(it);
    }
}

static int fallingTable[FLOBOBAN_DIMX] = {0, 3, 1, 4, 2, 5};

RandomSystem::RandomSystem(int numColors) : _numColors(numColors)
{
    init_genrand(fmod(ios_fc::getTimeMs(), 100000.));
}

RandomSystem::RandomSystem(unsigned long seed, int numColors) : _numColors(numColors)
{
    init_genrand(seed);
}

FloboState RandomSystem::getFloboStateForSequence(int sequence)
{
    if (_sequenceItems.size() <= sequence) {
        int newItem = (genrand_int32() % _numColors) + FLOBO_FALLINGBLUE;
        _sequenceItems.push_back(newItem);
        return (FloboState)newItem;
    }
    else
        return (FloboState)(_sequenceItems[sequence]);
}

int Flobo::_lastID = 0;

// FloboGame implementation
FloboGame::FloboGame()
{
    _attachedFactory = new FloboDefaultFactory();
}

FloboGame::FloboGame(FloboFactory *attachedFactory) : _attachedFactory(attachedFactory)
{
}

void FloboGame::addGameListener(GameListener *listener)
{
    _listeners.push_back(listener);
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
    _gameLevel = 1;
    _nbFalled = 0;
    _phaseReady = 2;
    int i, j;
    _unmoveableFlobo = _attachedFactory->createFlobo(FLOBO_UNMOVEABLE);
    for (i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        for (j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            if (j == FLOBOBAN_DIMY)
                setFloboAt(i, j, _unmoveableFlobo);
            else
                setFloboAt(i, j, nullptr);
        }
    }
    this->_attachedRandom = attachedRandom;
    _sequenceNr = 0;
    _semiMove = 0;
    _neutralFlobos = 0;

    _endOfCycle = false;
    _gameRunning = true;
    setFallingAtTop(true);

    _gameStat = PlayerGameStat();
}

FloboLocalGame::~FloboLocalGame()
{
    // unmoveableFlobo and floboVector objects are cleaned up automatically by shared_ptr.
}

void FloboLocalGame::cycle()
{
    if (!_gameRunning)
        return;

    _semiMove = 1 - _semiMove;
    if (_semiMove == 0) {
        if (!_endOfCycle) {
            for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
                 iter != _listeners.end() ; ++iter)
                (*iter)->fallingsDidFallingStep(_fallingFlobo, _companionFlobo);
        }
        return;
    }
    if (_endOfCycle) {
        cycleEnding();
        notifyReductions();
        return;
    }
    if (((_fallingY >= 0)&&(getFloboCellAt(_fallingX, _fallingY+1) > FLOBO_EMPTY)) || (getFloboCellAt(getFallingCompanionX(), getFallingCompanionY()+1) > FLOBO_EMPTY)) {
        setFloboAt(_fallingX, getFallY(_fallingX, _fallingY), _fallingFlobo);
        _fallingFlobo->setFloboState((FloboState)(_fallingFlobo->getFloboState()+FLOBO_STILL));
        setFloboAt(getFallingCompanionX(), getFallY(getFallingCompanionX(), getFallingCompanionY()), _companionFlobo);
        _companionFlobo->setFloboState((FloboState)(_companionFlobo->getFloboState()+FLOBO_STILL));
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter) {
            (*iter)->floboDidFall(_fallingFlobo, _fallingX, _fallingY, _fallingCompanion == 0 ? 1 : 0);
            (*iter)->floboDidFall(_companionFlobo, getFallingCompanionX(), getFallingCompanionY(), _fallingCompanion == 0 ? 0 : 1);
        }
        _gameStat.drop_count += 2;
        _fallingY = -10;
        notifyReductions();
        _endOfCycle = true;
    }
    else {
        _fallingY++;
        if (_phaseReady == 1) _phaseReady = 2;
        _fallingFlobo->setFloboXY(_fallingX, _fallingY);
        _companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter) {
			(*iter)->fallingsDidFallingStep(_fallingFlobo, _companionFlobo);
		}
        // Increase points
        switch (_gameLevel)
        {
            case 1:
                _gameStat.points += 1;
                break;
            case 2:
                _gameStat.points += 5;
                break;
            case 3:
                _gameStat.points += 10;
                break;
            default:
                _gameStat.points += _gameLevel * 5;
        }
    }
}

// Get the state of the flobo at the indicated coordinates
FloboState FloboLocalGame::getFloboCellAt(int X, int Y) const
{
    auto theFlobo = getFloboAt(X, Y);
    if (theFlobo)
        return theFlobo->getFloboState();
    return FLOBO_EMPTY;
}

// Get the flobo at the indicated coordinates
std::shared_ptr<Flobo> FloboLocalGame::getFloboAt(int X, int Y) const
{
    if ((X >= FLOBOBAN_DIMX) || (Y >= FLOBOBAN_DIMY) || (X < 0) || (Y < 0))
        return _unmoveableFlobo;
    if (!_endOfCycle) {
        if ((X == _fallingX) && (Y == _fallingY))
            return _fallingFlobo;
        if ((X == getFallingCompanionX()) && (Y == getFallingCompanionY()))
            return _companionFlobo;
    }
    return floboCells[X + Y * FLOBOBAN_DIMX];
}

// List access to the Flobo objects
int FloboLocalGame::getFloboCount() const
{
    return _floboVector.size();
}

std::shared_ptr<Flobo> FloboLocalGame::getFloboAtIndex(int index) const
{
    return _floboVector[index];
}

void FloboLocalGame::moveLeft()
{
    if (_endOfCycle) {
        return;
    }
    bool moved = false;
    if (((_fallingY<0)&&(_fallingX>0))||((getFloboCellAt(_fallingX-1, _fallingY) <= FLOBO_EMPTY)
     && (getFloboCellAt(getFallingCompanionX()-1, getFallingCompanionY()) <= FLOBO_EMPTY))) {
		moved = true;
		_fallingX--;
    }

    _fallingFlobo->setFloboXY(_fallingX, _fallingY);
    _companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter)
        (*iter)->fallingsDidMoveLeft(_fallingFlobo, _companionFlobo);
}

void FloboLocalGame::moveRight()
{
    if (_endOfCycle) {
        return;
    }
    bool moved = false;
    if (((_fallingY<0)&&(_fallingX<FLOBOBAN_DIMX-1))||((getFloboCellAt(_fallingX+1, _fallingY) <= FLOBO_EMPTY)
        && (getFloboCellAt(getFallingCompanionX()+1, getFallingCompanionY()) <= FLOBO_EMPTY))) {
	moved = true;
        _fallingX++;
    }
    _fallingFlobo->setFloboXY(_fallingX, _fallingY);
    _companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter)
            (*iter)->fallingsDidMoveRight(_fallingFlobo, _companionFlobo);
}

void FloboLocalGame::rotate(bool left)
{
    if (_endOfCycle) {
        return;
    }
    unsigned char backupCompanion = _fallingCompanion;
    int newX, newY;
    bool moved = true;
    _fallingCompanion = (unsigned char)(_fallingCompanion + (left?3:1)) % 4;
    unsigned char newCompanion = _fallingCompanion;
    int newCompanionX = getFallingCompanionX();
    int newCompanionY = getFallingCompanionY();
    _fallingCompanion = backupCompanion;
    if (getFloboCellAt(newCompanionX, newCompanionY) > FLOBO_EMPTY) {
        newX = _fallingX + (_fallingX - newCompanionX);
        newY = _fallingY + (_fallingY - newCompanionY);
        if (getFloboCellAt(newX, newY) > FLOBO_EMPTY) {
            moved = false;
        }
        else {
            _fallingCompanion = newCompanion;
            _fallingX = newX;
            if (_fallingY != newY)
            {
                _semiMove = 0;
                _fallingY = newY;
            }
            _fallingFlobo->setFloboXY(_fallingX, _fallingY);
        }
        }
    else
        _fallingCompanion = newCompanion;

    _companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());

    if (moved)
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter)
            (*iter)->companionDidTurn(_companionFlobo, _fallingFlobo, !left);

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
    return _attachedRandom->getFloboStateForSequence(_sequenceNr);
}

FloboState FloboLocalGame::getNextCompanion()
{
    return _attachedRandom->getFloboStateForSequence(_sequenceNr+1);
}

void FloboLocalGame::increaseNeutralFlobos(int incr)
{
    _neutralFlobos += incr;
}

int FloboLocalGame::getNeutralFlobos() const
{
    return _neutralFlobos;
}



// Set the state of the flobo at the indicated coordinates (not recommanded)
void FloboLocalGame::setFloboCellAt(int X, int Y, FloboState value)
{
  if (floboCells[X + Y * FLOBOBAN_DIMX])
    floboCells[X + Y * FLOBOBAN_DIMX]->setFloboState(value);
};

// Set the flobo at the indicated coordinates
void FloboLocalGame::setFloboAt(int X, int Y, std::shared_ptr<Flobo> newFlobo)
{
    floboCells[X + Y * FLOBOBAN_DIMX] = newFlobo;
    if (newFlobo != nullptr)
        newFlobo->setFloboXY(X, Y);
}

int FloboLocalGame::getGameTotalNeutralFlobos() const
{
    return _nbFalled;
}

void FloboLocalGame::dropNeutrals()
{
    if (_neutralFlobos < 0) {
        _gameStat.points -= _gameLevel * _neutralFlobos * 1000;
    }
    int totalNeutral = _neutralFlobos;
    int idNeutral = 0;
    while (_neutralFlobos > 0)
    {
      int cycleNeutral;
      if (_neutralFlobos >= FLOBOBAN_DIMX)
        cycleNeutral = FLOBOBAN_DIMX;
      else
        cycleNeutral = _neutralFlobos;
      for (int i = 0 ; i < cycleNeutral ; i++)
      {
        int posX = fallingTable[(_nbFalled++) % FLOBOBAN_DIMX];
        int posY = getFallY(posX, 2);
        _neutralFlobos -= 1;
        if (getFloboCellAt(posX, posY) != FLOBO_EMPTY)
            continue;
        // Creating a new neutral flobo
        auto newNeutral = _attachedFactory->createFlobo(FLOBO_NEUTRAL);
        _floboVector.push_back(newNeutral);
        setFloboAt(posX, posY, newNeutral);
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter)
            (*iter)->gameDidAddNeutral(newNeutral, idNeutral++, totalNeutral);
      }
    }
    _neutralFlobos = 0;
  _phaseReady = 1;
}

bool FloboLocalGame::isPhaseReady(void)
{
  bool r = (_phaseReady == 2);
  if (r) _phaseReady = 0;
  return r;
}

void FloboLocalGame::addNeutralLayer()
{
    // Raise everything up
    for (int j = 1 ; j < FLOBOBAN_DIMY ; ++j) {
        for (int i = 0 ; i < FLOBOBAN_DIMX ; ++i) {
            auto currentFlobo = getFloboAt(i, j);
            if (currentFlobo != nullptr) {
                setFloboAt(i, j, nullptr);
                setFloboAt(i, j-1, currentFlobo);
                for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
                     iter != _listeners.end() ; ++iter)
                    (*iter)->floboDidFall(currentFlobo, i, j, 1);
            }
        }
    }
    for (int i = 0 ; i < FLOBOBAN_DIMX ; ++i) {
        auto newNeutral = _attachedFactory->createFlobo(FLOBO_NEUTRAL);
        _floboVector.push_back(newNeutral);
        setFloboAt(i, FLOBOBAN_DIMY-1, newNeutral);
        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
             iter != _listeners.end() ; ++iter)
            (*iter)->floboDidFall(newNeutral, i, FLOBOBAN_DIMY, 1);
    }
}

void FloboLocalGame::setFallingAtTop(bool gameConstruction)
{
    for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
         iter != _listeners.end() ; ++iter)
        (*iter)->gameDidEndCycle();

    if (!gameConstruction) {
        dropNeutrals();
        if (getFloboCellAt((FLOBOBAN_DIMX-1)/2, 1) != FLOBO_EMPTY) {
            _gameRunning = false;
            for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
                 iter != _listeners.end() ; ++iter)
                (*iter)->gameLost();
            return;
        }
    }

    // Creating the new falling flobo and its companion
    _fallingX = (FLOBOBAN_DIMX-1)/2;
    _fallingY = 1;
    _fallingCompanion = 2;
    auto fallingPtr = _attachedFactory->createFlobo(_attachedRandom->getFloboStateForSequence(_sequenceNr++));
    auto companionPtr = _attachedFactory->createFlobo(_attachedRandom->getFloboStateForSequence(_sequenceNr++));
    _fallingFlobo = fallingPtr;
    _companionFlobo = companionPtr;
    _fallingFlobo->setFloboXY(_fallingX, _fallingY);
    _companionFlobo->setFloboXY(getFallingCompanionX(), getFallingCompanionY());
    _floboVector.push_back(fallingPtr);
    _floboVector.push_back(companionPtr);

    _endOfCycle = false;
    _semiMove = 0;
    _phase = 0;
}

int FloboLocalGame::getFallingCompanionX() const
{
    if (_fallingCompanion == 1)
        return _fallingX - 1;
    if (_fallingCompanion == 3)
        return _fallingX + 1;
    return _fallingX;
}

int FloboLocalGame::getFallingCompanionY() const
{
    if (_fallingCompanion == 0)
        return _fallingY + 1;
    if (_fallingCompanion == 2)
        return _fallingY - 1;
    return _fallingY;
}

int FloboLocalGame::getFallY(int X, int Y) const
{
    int result = Y + 1;
    while (getFloboCellAt(X, result) == FLOBO_EMPTY)
        result++;
    return result - 1;
}

int FloboLocalGame::getColumnHeigth(int colNum) const
{
  int result = 0;
  for (int i = 0 ; i < FLOBOBAN_DIMY ; i++) {
    if (getFloboCellAt(colNum, i) > FLOBO_EMPTY)
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
      if ((Y+1<FLOBOBAN_DIMY)&&!marked[X][Y+1] && (getFloboCellAt(X,Y+1) == color)) {
          again = true;
          mx[nFound] = X;
          my[nFound] = Y+1;
          marked[X][Y+1] = 1;
          nFound++;
      }
      if ((X+1<FLOBOBAN_DIMX) && !marked[X+1][Y] && (getFloboCellAt(X+1,Y) == color)) {
          again = true;
          mx[nFound] = X+1;
          my[nFound] = Y;
          marked[X+1][Y] = 1;
          nFound++;
      }
      if ((X-1>=0) && !marked[X-1][Y] && (getFloboCellAt(X-1,Y) == color)) {
          again = true;
          mx[nFound] = X-1;
          my[nFound] = Y;
          marked[X-1][Y] = 1;
          nFound++;
      }
      if ((Y-1>=0) && !marked[X][Y-1] && (getFloboCellAt(X,Y-1) == color)) {
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

void FloboLocalGame::markFloboAt(int X, int Y, bool mark, bool includeNeutral)
{
    auto currentFlobo = getFloboAt(X, Y);
    if (currentFlobo->isMarked() == mark)
        return;
    FloboState currentFloboState = getFloboCellAt(X, Y);
    currentFlobo->setMark(mark);
    if (getFloboCellAt(X-1, Y) == currentFloboState)
        markFloboAt(X-1, Y, mark, includeNeutral);
    if (getFloboCellAt(X+1, Y) == currentFloboState)
        markFloboAt(X+1, Y, mark, includeNeutral);
    if (getFloboCellAt(X, Y-1) == currentFloboState)
        markFloboAt(X, Y-1, mark, includeNeutral);
    if (getFloboCellAt(X, Y+1) == currentFloboState)
        markFloboAt(X, Y+1, mark, includeNeutral);
    if (includeNeutral) {
        if (getFloboCellAt(X-1, Y) == FLOBO_NEUTRAL)
            getFloboAt(X-1, Y)->setMark(mark);
        if (getFloboCellAt(X+1, Y) == FLOBO_NEUTRAL)
            getFloboAt(X+1, Y)->setMark(mark);
        if (getFloboCellAt(X, Y-1) == FLOBO_NEUTRAL)
            getFloboAt(X, Y-1)->setMark(mark);
        if (getFloboCellAt(X, Y+1) == FLOBO_NEUTRAL)
            getFloboAt(X, Y+1)->setMark(mark);
    }
}

// delete the marked flobos and the neutral next to them
void FloboLocalGame::deleteMarkedFlobosAt(int X, int Y)
{
  removeFloboFromVector(_floboVector, getFloboAt(X, Y));
  if (getFloboAt(X,Y) == _companionFlobo) {
    _companionFlobo = nullptr;
  }
  else if (getFloboAt(X,Y) == _fallingFlobo) {
    _fallingFlobo = nullptr;
  }
  setFloboAt(X, Y, NULL);
    if (getFloboAt(X-1, Y)->isMarked())
        deleteMarkedFlobosAt(X-1, Y);
    if (getFloboAt(X+1, Y)->isMarked())
        deleteMarkedFlobosAt(X+1, Y);
    if (getFloboAt(X, Y-1)->isMarked())
        deleteMarkedFlobosAt(X, Y-1);
    if (getFloboAt(X, Y+1)->isMarked())
        deleteMarkedFlobosAt(X, Y+1);
    if (getFloboCellAt(X-1, Y) == FLOBO_NEUTRAL) {
        removeFloboFromVector(_floboVector, getFloboAt(X-1, Y));
        setFloboAt(X-1, Y, NULL);
    }
    if (getFloboCellAt(X+1, Y) == FLOBO_NEUTRAL) {
        removeFloboFromVector(_floboVector, getFloboAt(X+1, Y));
        setFloboAt(X+1, Y, NULL);
    }
    if (getFloboCellAt(X, Y-1) == FLOBO_NEUTRAL) {
        removeFloboFromVector(_floboVector, getFloboAt(X, Y-1));
        setFloboAt(X, Y-1, NULL);
    }
    if (getFloboCellAt(X, Y+1) == FLOBO_NEUTRAL) {
        removeFloboFromVector(_floboVector, getFloboAt(X, Y+1));
        setFloboAt(X, Y+1, NULL);
    }
}

int FloboLocalGame::removeFlobos()
{
    int globalRemovedFlobos = 0;
    /* First, we will mark all the flobos that need to be removed */
    for (int i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            FloboState currentFlobo = getFloboCellAt(i, j);
            if ((currentFlobo >= FLOBO_BLUE) && (currentFlobo <= FLOBO_YELLOW)) {
                int removedFlobos = 0;
                markFloboAt(i, j, true, false);
                for (int u = 0, v = _floboVector.size() ; u < v ; u++) {
                    if (_floboVector[u]->isMarked()) {
                        removedFlobos++;
                    }
                }
                //printf("Removed for %d, %d : %d\n", i, j, removedFlobos);
                if (removedFlobos >= 4) {
                    globalRemovedFlobos += removedFlobos;
                    deleteMarkedFlobosAt(i, j);
                }
                else
                    markFloboAt(i, j, false, false);
            }
        }
    }
    /* Next we make the other flobos fall */
    for (int i = 0 ; i < FLOBOBAN_DIMX ; i++) {
        int feltBelow = 0;
        for (int j = FLOBOBAN_DIMY - 1 ; j > 0 ; j--) {
            FloboState currentFloboState = getFloboCellAt(i, j);
            if ((currentFloboState >= FLOBO_BLUE) && (currentFloboState <= FLOBO_NEUTRAL)) {
                int newJ = getFallY(i, j);
                if (newJ != j) {
                    auto currentFlobo = getFloboAt(i, j);
                    setFloboAt(i, j, nullptr);
                    setFloboAt(i, newJ, currentFlobo);
                    for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
                         iter != _listeners.end() ; ++iter) {
                        (*iter)->floboDidFall(currentFlobo, i, j, feltBelow);
                        feltBelow++;
                    }
                }
            }
        }
    }
    return globalRemovedFlobos;
}


void FloboLocalGame::notifyReductions()
{
    std::vector<std::shared_ptr<Flobo>> removedFlobos;
    // Clearing every flobo's flag
    for (int i = 0, j = getFloboCount() ; i < j ; i++) {
        getFloboAtIndex(i)->unsetFlag();
    }
    // Search for groupped flobos
    int floboGroupNumber = 0;
    for (int j = 0 ; j < FLOBOBAN_DIMY ; j++) {
        for (int i = 0 ; i <= FLOBOBAN_DIMX ; i++) {
            auto floboToMark = getFloboAt(i, j);
            // If the flobo exists and is not flagged, then
            if ((floboToMark != nullptr) && (! floboToMark->getFlag())) {
                FloboState initialFloboState = floboToMark->getFloboState();
                // I really would have liked to skip this stupid test
                if ((initialFloboState >= FLOBO_BLUE) && (initialFloboState <= FLOBO_YELLOW)) {
                    markFloboAt(i, j, true, true);

                    // Collecting every marked flobo in a vector
                    removedFlobos.clear();
                    int removedCount = 0;
                    for (int u = 0, v = _floboVector.size() ; u < v ; u++) {
                        auto markedFlobo = _floboVector[u];
                        if (markedFlobo->isMarked()) {
                            markedFlobo->setFlag();
                            removedFlobos.push_back(markedFlobo);
                            if (markedFlobo->getFloboState() != FLOBO_NEUTRAL)
                                removedCount++;
                        }
                    }
                    // If there is more than 4 flobos in the group, let's notify it
                    if (removedCount >= 4) {
                        markFloboAt(i, j, false, true);
                        for (GameListenerPtrVector::iterator iter = _listeners.begin() ;
                             iter != _listeners.end() ; ++iter)
                            (*iter)->floboWillVanish(removedFlobos, floboGroupNumber++, _phase+1);
                    }
                    else {
                        markFloboAt(i, j, false, true);
                    }
                }
            }
        }
    }
}

void FloboLocalGame::cycleEnding()
{
    int score = removeFlobos();
    _gameStat.explode_count += score;

    if (score >= 4) {
#ifdef DESACTIVE
        audio_sound_play(sound_splash[phase>7?7:phase]);
#endif
        score -= 3;
        if (_phase > 0) {
            _neutralFlobos -= FLOBOBAN_DIMX;
            _gameStat.ghost_sent_count += FLOBOBAN_DIMX;
        }
        ++_gameStat.combo_count[_phase];
        _phase++;
    }

    _gameStat.points += _gameLevel * 100 + _gameLevel * (_phase>0?_phase-1:0) * 5000;

    _neutralFlobos -= score;
    _gameStat.ghost_sent_count += score;

    if (score == 0)
        setFallingAtTop();
}
