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
#include "GTLog.h"

using namespace PuyoMessage;

PuyoNetworkGame::~PuyoNetworkGame()
{
  msgBox.removeListener(this);
}

PuyoNetworkGame::PuyoNetworkGame(PuyoFactory *attachedFactory, MessageBox &msgBox, int gameId)
  : PuyoGame(attachedFactory), nextFalling(PUYO_BLUE), nextCompanion(PUYO_BLUE),
    msgBox(msgBox), gameId(gameId), gameRunning(true), comboPhase(0)
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
                    delegate->gameLost();
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

void PuyoNetworkGame::synchronizePuyo(Buffer<int> buffer) {
    int puyoID    = buffer[0];
    int puyoState = buffer[1]; GTCheckInterval(puyoState, 0, 20, "puyoState is invalid");
    int posX      = buffer[2]; GTCheckInterval(posX, 0, PUYODIMX, "Puyo X is invalid");
    int posY      = buffer[3]; GTCheckInterval(posY, 0, PUYODIMY, "Puyo Y is invalid");
    PuyoPuyo *puyo = findPuyo(puyoID);
    if (puyo == NULL) {
        puyo = attachedFactory->createPuyo((PuyoState)puyoState);
        puyo->setID(puyoID);
        puyoVector.add(puyo);
    }
    else {
        puyo->setPuyoState((PuyoState)puyoState);
    }
    setPuyoAt(puyo->getPuyoX(), puyo->getPuyoY(), NULL);
    setPuyoAt(posX, posY, puyo);
    puyo->setFlag(); // TODO: return puyo and to this in synchromessage
}

void PuyoNetworkGame::synchronizeState(Message &message)
{
    gameStat.points = message.getInt(SCORE);
    GTCheckInterval(gameStat.points, 0, 999999999, "points is invalid");
    nextFalling = (PuyoState)(message.getInt(NEXT_F));
    GTCheckInterval(nextFalling, 0, 20, "nextFalling is invalid");
    nextCompanion = (PuyoState)(message.getInt(NEXT_C));
    GTCheckInterval(nextCompanion, 0, 20, "nextCompanion is invalid");
    semiMove = message.getInt(SEMI_MOVE);

    if (message.hasIntArray(PUYOS)) {
        Buffer<int> puyos = message.getIntArray(PUYOS);
        for (int i = 0; i+3 < puyos.size(); i += 4)
            synchronizePuyo(puyos+i);

        // Remove the puyos that were not flagged.
        for (int i = puyoVector.size() - 1 ; i >= 0 ; i--) {
            PuyoPuyo *currentPuyo = puyoVector[i];
            if (currentPuyo->getFlag()) {
                currentPuyo->unsetFlag();
            }
            else {
                setPuyoAt(currentPuyo->getPuyoX(), currentPuyo->getPuyoY(), NULL);
                puyoVector.removeAt(i);
                attachedFactory->deletePuyo(currentPuyo);
            }
        }
    }

    // Notifications
    Buffer<int> addNeutrals= message.getIntArray(ADD_NEUTRALS);
    if (addNeutrals.size() > 0) {
       if (delegate != NULL) {
            for (int i = 0, j = addNeutrals.size() ; i+1 < j ; i += 2) {
                PuyoPuyo *neutral = findPuyo(addNeutrals[i]);
                if (neutral != NULL)
                    delegate->gameDidAddNeutral(neutral, addNeutrals[i+1]);
            }
        }
    }

    Buffer<int> moveLeftBuffer= message.getIntArray(MV_L);
    if (moveLeftBuffer.size() > 0) {
        if (delegate != NULL) {
            for (int i = 0, j = moveLeftBuffer.size() ; i+7 < j ; i += 8) {
                PuyoPuyo *falling = findPuyo(moveLeftBuffer[i]);
                PuyoPuyo *companion = findPuyo(moveLeftBuffer[i+4]);
                if ((falling != NULL) && (companion != NULL)) {
                    synchronizePuyo(moveLeftBuffer+i);
                    synchronizePuyo(moveLeftBuffer+i+4);
                    delegate->fallingsDidMoveLeft(falling, companion);
                }
            }
        }
    }

    Buffer<int> moveRightBuffer= message.getIntArray(MV_R);
    if (moveRightBuffer.size() > 0) {
        if (delegate != NULL) {
            for (int i = 0, j = moveRightBuffer.size() ; i+7 < j ; i += 8) {
                PuyoPuyo *falling = findPuyo(moveRightBuffer[i]);
                PuyoPuyo *companion = findPuyo(moveRightBuffer[i+4]);
                if ((falling != NULL) && (companion != NULL)) {
                    synchronizePuyo(moveRightBuffer+i);
                    synchronizePuyo(moveRightBuffer+i+4);
                    delegate->fallingsDidMoveRight(falling, companion);
                }
            }
        }
    }

    Buffer<int> fallingStepBuffer= message.getIntArray(MV_D);
    if (fallingStepBuffer.size() > 0) {
        if (delegate != NULL) {
            for (int i = 0, j = fallingStepBuffer.size() ; i+1 < j ; i += 2) {
                PuyoPuyo *fallingPuyo = findPuyo(fallingStepBuffer[i]);
                PuyoPuyo *companionPuyo = findPuyo(fallingStepBuffer[i+1]);
                if ((fallingPuyo != NULL) && (companionPuyo != NULL))
                    delegate->fallingsDidFallingStep(fallingPuyo, companionPuyo);
            }
        }
    }

    Buffer<int> turnBuffer= message.getIntArray(COMPANION_TURN);
    if (turnBuffer.size() > 0) {
        if (delegate != NULL) {
            for (int i = 0, j = turnBuffer.size() ; i+2 < j ; i += 3) {
                PuyoPuyo *companionPuyo = findPuyo(turnBuffer[i]);
                PuyoPuyo *fallingPuyo = findPuyo(turnBuffer[i+1]);
                if ((fallingPuyo != NULL) && (companionPuyo != NULL))
                    delegate->companionDidTurn(companionPuyo, fallingPuyo, turnBuffer[i+2]);
            }
        }
    }

    Buffer<int> didFall= message.getIntArray(DID_FALL);
    if (didFall.size() > 0) {
       if (delegate != NULL) {
            for (int i = 0, j = didFall.size() ; i+3 < j ; i += 4) {
                PuyoPuyo *didFallPuyo = findPuyo(didFall[i]);
                if (didFallPuyo != NULL)
                    delegate->puyoDidFall(didFallPuyo, didFall[i+1], didFall[i+2], didFall[i+3]);
            }
        }
    }

    Buffer<int> willVanish= message.getIntArray(WILL_VANISH);
    if (willVanish.size() > 0) {
       if (delegate != NULL) {
            int i = 0;
            AdvancedBuffer<PuyoPuyo *> temporaryGroup;
            while (i < willVanish.size()) {
                temporaryGroup.clear();
                int numPhase = willVanish[i++];
                int groupNumber = willVanish[i++];
                int numberOfPuyosInGroup = willVanish[i++];
                for (int index = 0 ; index < numberOfPuyosInGroup ; index++) {
                    PuyoPuyo *vanishedPuyo = findPuyo(willVanish[i + index]);
                    if (vanishedPuyo != NULL)
                        temporaryGroup.add(vanishedPuyo);
                }
                delegate->puyoWillVanish(temporaryGroup, groupNumber, numPhase);
                i += numberOfPuyosInGroup;
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

    neutralPuyos = message.getInt(CURRENT_NEUTRALS);
}

PuyoPuyo *PuyoNetworkGame::findPuyo(int puyoID)
{
    for (int i = 0, j = puyoVector.size() ; i < j ; i++) {
        PuyoPuyo *currentPuyo = puyoVector[i];
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
    return puyoVector.size();
}

PuyoPuyo *PuyoNetworkGame::getPuyoAtIndex(int index) const
{
    return puyoVector[index];
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
    return gameRunning;
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

