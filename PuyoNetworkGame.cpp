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

#include "PuyoNetworkGame.h"
#include "PuyoMessageDef.h"
#include <stdlib.h>
#include <stdio.h>

using namespace PuyoMessage;

PuyoNetworkGame::~PuyoNetworkGame()
{
  msgBox.removeListener(this);
}

PuyoNetworkGame::PuyoNetworkGame(PuyoFactory *attachedFactory, MessageBox &msgBox) : PuyoGame(attachedFactory), msgBox(msgBox)
{
    fakePuyo = attachedFactory->createPuyo(PUYO_FALLINGRED);
    msgBox.addListener(this);
    semiMove = 0;
    neutralPuyos = 0;
    sentBadPuyos = 0;
    for (int i = 0 ; i < PUYODIMX ; i++) {
		for (int j = 0 ; j <= PUYODIMY ; j++) {
            setPuyoAt(i, j, NULL);
		}
	}
}

void PuyoNetworkGame::onMessage(Message &message)
{
    int msgType = message.getInt(TYPE);
    switch (msgType) {
        case kGameState:
            synchronizeState(message);
            break;
        default:
            break;
    }
}

void PuyoNetworkGame::synchronizeState(Message &message)
{
    points = message.getInt(SCORE);
    nextFalling = (PuyoState)(message.getInt(NEXT_F));
    nextCompanion = (PuyoState)(message.getInt(NEXT_C));
    semiMove = message.getInt(SEMI_MOVE);
    
    Buffer<int> puyos = message.getIntArray(PUYOS);
    int i = 0;
    while (i < puyos.size()) {
        int currentPuyoID = puyos[i];
        PuyoPuyo *currentPuyo = findPuyo(currentPuyoID);
        if (currentPuyo == NULL) {
            currentPuyo = attachedFactory->createPuyo((PuyoState)(puyos[i+1]));
            currentPuyo->setID(currentPuyoID);
            puyoVector.addElement(currentPuyo);
        }
        else {
            currentPuyo->setPuyoState((PuyoState)(puyos[i+1]));
        }
        currentPuyo->setFlag();
        setPuyoAt(currentPuyo->getPuyoX(), currentPuyo->getPuyoY(), NULL);
        setPuyoAt(puyos[i+2], puyos[i+3], currentPuyo);
        i += 4;
    }
    
    for (int i = puyoVector.getSize() - 1 ; i >= 0 ; i--) {
        PuyoPuyo *currentPuyo = (PuyoPuyo *)(puyoVector.getElementAt(i));
        if (currentPuyo->getFlag()) {
            currentPuyo->unsetFlag();
        }
        else {
            setPuyoAt(currentPuyo->getPuyoX(), currentPuyo->getPuyoY(), NULL);
            puyoVector.removeElementAt(i);
            attachedFactory->deletePuyo(currentPuyo);
        }
    }
    
    // Notifications
    Buffer<int> addNeutrals= message.getIntArray(ADD_NEUTRALS);
    if (addNeutrals.size() > 0) {
       if (delegate != NULL) {
            for (int i = 0, j = addNeutrals.size() ; i < j ; i += 2) {
                delegate->gameDidAddNeutral(findPuyo(addNeutrals[i]), addNeutrals[i+1]);
            }
        }
    }
    
    Buffer<int> turnBuffer= message.getIntArray(COMPANION_TURN);
    if (turnBuffer.size() > 0) {
        if (delegate != NULL) {
            for (int i = 0, j = turnBuffer.size() ; i < j ; i += 3) {
                delegate->companionDidTurn(findPuyo(turnBuffer[i]), turnBuffer[i+1], turnBuffer[i+2]);
            }
        }
    }
    
    Buffer<int> didFall= message.getIntArray(DID_FALL);
    if (didFall.size() > 0) {
       if (delegate != NULL) {
            for (int i = 0, j = didFall.size() ; i < j ; i += 3) {
                delegate->puyoDidFall(findPuyo(didFall[i]), didFall[i+1], didFall[i+2]);
            }
        }
    }
    
    int badPuyos = message.getInt(NUMBER_BAD_PUYOS);
    if (badPuyos > sentBadPuyos) {
        neutralPuyos = sentBadPuyos - badPuyos;
        if (delegate != NULL) {
            delegate->gameDidEndCycle();
        }
        neutralPuyos = 0;
        sentBadPuyos = badPuyos;
    }
}

PuyoPuyo *PuyoNetworkGame::findPuyo(int puyoID)
{
    for (int i = 0, j = puyoVector.getSize() ; i < j ; i++) {
        PuyoPuyo *currentPuyo = (PuyoPuyo *)(puyoVector.getElementAt(i));
        if (currentPuyo->getID() == puyoID)
            return currentPuyo;
    }
    return NULL;
}

void PuyoNetworkGame::cycle()
{
}

PuyoPuyo *PuyoNetworkGame::getPuyoAt(int X, int Y) const
{
    if ((X >= PUYODIMX) || (Y >= PUYODIMY) || (X < 0) || (Y < 0))
		return NULL;
    return puyoCells[X + Y * PUYODIMX];
}

void PuyoNetworkGame::setPuyoAt(int X, int Y, PuyoPuyo *newPuyo)
{
    puyoCells[X + Y * PUYODIMX] = newPuyo;
    if (newPuyo != NULL)
        newPuyo->setPuyoXY(X, Y);
}

// List access to the PuyoPuyo objects
int PuyoNetworkGame::getPuyoCount() const
{
    return puyoVector.getSize();
}

PuyoPuyo *PuyoNetworkGame::getPuyoAtIndex(int index) const
{
    return (PuyoPuyo *)(puyoVector.getElementAt(index));
}
  
PuyoState PuyoNetworkGame::getNextFalling()
{
    return nextFalling;
}

PuyoState PuyoNetworkGame::getNextCompanion()
{
    return nextCompanion;
}
  
PuyoState PuyoNetworkGame::getCompanionState() const
{
    return PUYO_FALLINGRED;
}

PuyoState PuyoNetworkGame::getFallingState() const
{
    return PUYO_FALLINGRED;
}

int PuyoNetworkGame::getFallingX() const
{
    return 1;
}

int PuyoNetworkGame::getFallingY() const
{
    return 1;
}

int PuyoNetworkGame::getFallingCompanionDir() const
{
    return 1;
}

PuyoPuyo *PuyoNetworkGame::getFallingPuyo() const
{
    return fakePuyo;
}

void PuyoNetworkGame::increaseNeutralPuyos(int incr)
{
}

int PuyoNetworkGame::getNeutralPuyos() const
{
    return neutralPuyos;
}

void PuyoNetworkGame::dropNeutrals()
{
}

bool PuyoNetworkGame::isGameRunning() const
{
    return true;
}

bool PuyoNetworkGame::isEndOfCycle() const
{
    return false;
}

int PuyoNetworkGame::getColumnHeigth(int colNum) const
{
    return 0;
}

int PuyoNetworkGame::getMaxColumnHeight() const
{
    return 0;
}

int PuyoNetworkGame::getSamePuyoAround(int X, int Y, PuyoState color)
{
    return 0;
}

int PuyoNetworkGame::getSemiMove() const
{
    return semiMove;
}

