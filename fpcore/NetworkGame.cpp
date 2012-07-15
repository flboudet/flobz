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

#include "NetworkGame.h"
#include "FPNetMessageDef.h"
#include <stdlib.h>
#include <stdio.h>
#include "GTLog.h"

using namespace FPNetMessage;

NetworkGame::~NetworkGame()
{
  msgBox->removeListener(this);
}

NetworkGame::NetworkGame(FloboFactory *attachedFactory, MessageBox *msgBox, int gameId)
  : FloboGame(attachedFactory), nextFalling(FLOBO_BLUE), nextCompanion(FLOBO_BLUE),
    msgBox(msgBox), gameId(gameId), gameRunning(true), comboPhase(0)
{
    fakeFlobo = attachedFactory->createFlobo(FLOBO_FALLINGRED);
    msgBox->addListener(this);
    semiMove = 0;
    neutralFlobos = 0;
    sentBadFlobos = 0;
    for (int i = 0 ; i < FLOBOBAN_DIMX ; i++) {
		for (int j = 0 ; j <= FLOBOBAN_DIMY ; j++) {
            setFloboAt(i, j, NULL);
		}
	}
}

void NetworkGame::onMessage(Message &message)
{
    try {
        if (!message.hasInt(GAMEID))
            return;
        // Check if the message matches the gameid
        int gid = message.getInt(GAMEID);
        if (gid != gameId)
            return;
        int msgType = message.getInt(TYPE);
        switch (msgType) {
            case kGameState:
                synchronizeState(message);
                break;
            case kGameOverLost:
            case kGameOverWon:
                gameStat.points = message.getInt(SCORE);
                GTCheckInterval(gameStat.points, 0, 999999999, "points is invalid");
                gameStat.total_points = message.getInt(TOTAL_SCORE);
                GTCheckInterval(gameStat.total_points, 0, 999999999, "total_points is invalid");
                for (int i = 0 ; i < 24 ; i++) {
                    String messageName = String(COMBO_COUNT) + i;
                    gameStat.combo_count[i] = message.getInt(messageName);
                    GTCheckInterval(gameStat.combo_count[i], 0, 999, "combo_count is invalid");
                }
                gameStat.explode_count = message.getInt(EXPLODE_COUNT);
                GTCheckInterval(gameStat.explode_count, 0, 999999, "explode_count is invalid");
                gameStat.drop_count = message.getInt(DROP_COUNT);
                GTCheckInterval(gameStat.drop_count, 0, 999999, "drop_count is invalid");
                gameStat.ghost_sent_count = message.getInt(GHOST_SENT_COUNT);
                GTCheckInterval(gameStat.ghost_sent_count, 0, 9999999, "ghost_sent_count is invalid");
                gameStat.time_left = message.getFloat(TIME_LEFT);
                gameStat.is_dead = message.getBool(IS_DEAD);
                gameStat.is_winner = message.getBool(IS_WINNER);
                gameRunning = false;
                if (msgType == kGameOverLost)
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->gameLost();
                break;
            default:
                GTLogTrace("Invalid message type");
                break;
        }
    }
    catch (Exception e) {
        GTLogTrace("Invalid Message...");
    }
}

void NetworkGame::synchronizeFlobo(Buffer<int> buffer) {
    int floboID    = buffer[0];
    int floboState = buffer[1]; GTCheckInterval(floboState, 0, 20, "floboState is invalid");
    int posX       = buffer[2]; GTCheckInterval(posX, 0, FLOBOBAN_DIMX, "Flobo X is invalid");
    int posY       = buffer[3]; GTCheckInterval(posY, 0, FLOBOBAN_DIMY, "Flobo Y is invalid");
    Flobo *flobo = findFlobo(floboID);
    if (flobo == NULL) {
        flobo = attachedFactory->createFlobo((FloboState)floboState);
        flobo->setID(floboID);
        m_floboList.push_back(flobo);
        m_floboMap[floboID] = flobo;
    }
    else {
        flobo->setFloboState((FloboState)floboState);
    }
    setFloboAt(flobo->getFloboX(), flobo->getFloboY(), NULL);
    setFloboAt(posX, posY, flobo);
    flobo->setFlag(); // TODO: return flobo and to this in synchromessage
}

void NetworkGame::synchronizeState(Message &message)
{
    gameStat.points = message.getInt(SCORE);
    GTCheckInterval(gameStat.points, 0, 999999999, "points is invalid");
    nextFalling = (FloboState)(message.getInt(NEXT_F));
    GTCheckInterval(nextFalling, 0, 20, "nextFalling is invalid");
    nextCompanion = (FloboState)(message.getInt(NEXT_C));
    GTCheckInterval(nextCompanion, 0, 20, "nextCompanion is invalid");
    semiMove = message.getInt(SEMI_MOVE);

    if (message.hasIntArray(FLOBOS)) {
        Buffer<int> flobos = message.getIntArray(FLOBOS);
        for (int i = 0; i+3 < flobos.size(); i += 4)
            synchronizeFlobo(flobos+i);

        // Remove the flobos that were not flagged.
        for (FloboPtrList::iterator iter  = m_floboList.begin() ;
             iter != m_floboList.end() ; ++iter) {
            Flobo *currentFlobo = *iter;
            if (currentFlobo->getFlag()) {
                currentFlobo->unsetFlag();
            }
            else {
                setFloboAt(currentFlobo->getFloboX(), currentFlobo->getFloboY(), NULL);
                FloboPtrList::iterator removed = iter--;
                m_floboList.erase(removed);
                m_floboMap.erase(currentFlobo->getID());
                attachedFactory->deleteFlobo(currentFlobo);
            }
        }
    }

    // Notifications
    if (message.hasIntArray(ADD_NEUTRALS)) {
        Buffer<int> addNeutrals= message.getIntArray(ADD_NEUTRALS);
        if (addNeutrals.size() > 0) {
            for (int i = 0, j = addNeutrals.size() ; i+5 < j ; i += 6) {
                synchronizeFlobo(addNeutrals+i);
                Flobo *neutral = findFlobo(addNeutrals[i]);
                if (neutral != NULL)
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->gameDidAddNeutral(neutral, addNeutrals[i+4], addNeutrals[i+5]);
            }
        }
    }
    if (message.hasIntArray(MV_L)) {
        Buffer<int> moveLeftBuffer= message.getIntArray(MV_L);
        if (moveLeftBuffer.size() > 0) {
            for (int i = 0, j = moveLeftBuffer.size() ; i+7 < j ; i += 8) {
                synchronizeFlobo(moveLeftBuffer+i);
                synchronizeFlobo(moveLeftBuffer+i+4);
                Flobo *falling = findFlobo(moveLeftBuffer[i]);
                Flobo *companion = findFlobo(moveLeftBuffer[i+4]);
                if ((falling != NULL) && (companion != NULL)) {
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->fallingsDidMoveLeft(falling, companion);
                }
            }
        }
    }
    if (message.hasIntArray(MV_R)) {
        Buffer<int> moveRightBuffer= message.getIntArray(MV_R);
        if (moveRightBuffer.size() > 0) {
            for (int i = 0, j = moveRightBuffer.size() ; i+7 < j ; i += 8) {
                synchronizeFlobo(moveRightBuffer+i);
                synchronizeFlobo(moveRightBuffer+i+4);
                Flobo *falling = findFlobo(moveRightBuffer[i]);
                Flobo *companion = findFlobo(moveRightBuffer[i+4]);
                if ((falling != NULL) && (companion != NULL)) {
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->fallingsDidMoveRight(falling, companion);
                }
            }
        }
    }
    if (message.hasIntArray(MV_D)) {
        Buffer<int> fallingStepBuffer= message.getIntArray(MV_D);
        if (fallingStepBuffer.size() > 0) {
            for (int i = 0, j = fallingStepBuffer.size() ; i+7 < j ; i += 8) {
                synchronizeFlobo(fallingStepBuffer+i);
                synchronizeFlobo(fallingStepBuffer+i+4);
                Flobo *fallingFlobo = findFlobo(fallingStepBuffer[i]);
                Flobo *companionFlobo = findFlobo(fallingStepBuffer[i+4]);
                if ((fallingFlobo != NULL) && (companionFlobo != NULL))
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->fallingsDidFallingStep(fallingFlobo, companionFlobo);
            }
        }
    }
    if (message.hasIntArray(COMPANION_TURN)) {
        Buffer<int> turnBuffer= message.getIntArray(COMPANION_TURN);
        if (turnBuffer.size() > 0) {
            for (int i = 0, j = turnBuffer.size() ; i+8 < j ; i += 9) {
                synchronizeFlobo(turnBuffer+i);
                synchronizeFlobo(turnBuffer+i+4);
                Flobo *fallingFlobo = findFlobo(turnBuffer[i]);
                Flobo *companionFlobo = findFlobo(turnBuffer[i+4]);
                if ((fallingFlobo != NULL) && (companionFlobo != NULL))
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->companionDidTurn(companionFlobo, fallingFlobo, turnBuffer[i+8]);
            }
        }
    }
    if (message.hasIntArray(DID_FALL)) {
        Buffer<int> didFall= message.getIntArray(DID_FALL);
        if (didFall.size() > 0) {
            for (int i = 0, j = didFall.size() ; i+6 < j ; i += 7) {
                synchronizeFlobo(didFall+i);
                Flobo *didFallFlobo = findFlobo(didFall[i]);
                if (didFallFlobo != NULL)
                    for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                         iter != m_listeners.end() ; ++iter)
                        (*iter)->floboDidFall(didFallFlobo, didFall[i+4], didFall[i+5], didFall[i+6]);
            }
        }
    }
    if (message.hasIntArray(WILL_VANISH)) {
        Buffer<int> willVanish= message.getIntArray(WILL_VANISH);
        if (willVanish.size() > 0) {
            int i = 0;
            AdvancedBuffer<Flobo *> temporaryGroup;
            while (i < willVanish.size()) {
                temporaryGroup.clear();
                int numPhase = willVanish[i++];
                int groupNumber = willVanish[i++];
                int numberOfFlobosInGroup = willVanish[i++];
                for (int index = 0 ; index < numberOfFlobosInGroup ; index++) {
                    Flobo *vanishedFlobo = findFlobo(willVanish[i + index]);
                    if (vanishedFlobo != NULL)
                        temporaryGroup.add(vanishedFlobo);
                }
                for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
                     iter != m_listeners.end() ; ++iter)
                (*iter)->floboWillVanish(temporaryGroup, groupNumber, numPhase);
                i += numberOfFlobosInGroup;
            }
        }
    }
    int badFlobos = message.getInt(NUMBER_BAD_FLOBOS);
    if (badFlobos > sentBadFlobos) {
        neutralFlobos = sentBadFlobos - badFlobos;
        for (GameListenerPtrVector::iterator iter = m_listeners.begin() ;
             iter != m_listeners.end() ; ++iter) {
            (*iter)->gameDidEndCycle();
        }
        neutralFlobos = 0;
        sentBadFlobos = badFlobos;
    }

    neutralFlobos = message.getInt(CURRENT_NEUTRALS);
}

inline Flobo *NetworkGame::findFlobo(int floboID)
{
    std::map<int, Flobo *>::iterator found = m_floboMap.find(floboID);
    if (found == m_floboMap.end())
        return NULL;
    return found->second;
}

void NetworkGame::cycle()
{
}

Flobo *NetworkGame::getFloboAt(int X, int Y) const
{
    if ((X >= FLOBOBAN_DIMX) || (Y >= FLOBOBAN_DIMY) || (X < 0) || (Y < 0))
		return NULL;
    return floboCells[X + Y * FLOBOBAN_DIMX];
}

void NetworkGame::setFloboAt(int X, int Y, Flobo *newFlobo)
{
    floboCells[X + Y * FLOBOBAN_DIMX] = newFlobo;
    if (newFlobo != NULL)
        newFlobo->setFloboXY(X, Y);
}

// List access to the Flobo objects
int NetworkGame::getFloboCount() const
{
    return m_floboMap.size();
}

#ifdef DISABLED
Flobo *NetworkGame::getFloboAtIndex(int index) const
{
    return m_floboVector[index];
}
#endif

FloboState NetworkGame::getNextFalling()
{
    return nextFalling;
}

FloboState NetworkGame::getNextCompanion()
{
    return nextCompanion;
}

FloboState NetworkGame::getCompanionState() const
{
    return FLOBO_FALLINGRED;
}

FloboState NetworkGame::getFallingState() const
{
    return FLOBO_FALLINGRED;
}

int NetworkGame::getFallingX() const
{
    return 1;
}

int NetworkGame::getFallingY() const
{
    return 1;
}

int NetworkGame::getFallingCompanionDir() const
{
    return 1;
}

Flobo *NetworkGame::getFallingFlobo() const
{
    return fakeFlobo;
}

void NetworkGame::increaseNeutralFlobos(int incr)
{
}

int NetworkGame::getNeutralFlobos() const
{
    return neutralFlobos;
}

void NetworkGame::dropNeutrals()
{
}

bool NetworkGame::isGameRunning() const
{
    return gameRunning;
}

bool NetworkGame::isEndOfCycle() const
{
    return false;
}

int NetworkGame::getColumnHeigth(int colNum) const
{
    return 0;
}

int NetworkGame::getMaxColumnHeight() const
{
    return 0;
}

int NetworkGame::getSameFloboAround(int X, int Y, FloboState color)
{
    return 0;
}

int NetworkGame::getSemiMove() const
{
    return semiMove;
}

