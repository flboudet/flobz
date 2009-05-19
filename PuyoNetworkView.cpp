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

#include "PuyoNetworkView.h"
#include "PuyoIgpDefs.h"
#include <memory>

extern char *p1name;
extern char *p2name;

Message *PuyoNetworkView::createStateMessage(bool paused)
{
    // preparation des infos */
    int puyoCount = attachedGame->getPuyoCount();
    AdvancedBuffer<int> buffer(puyoCount * 4);

    for (int i = 0 ; i < puyoCount ; i++) {
        PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
        buffer.add(currentPuyo->getID());
        buffer.add(currentPuyo->getPuyoState());
        buffer.add(currentPuyo->getPuyoX());
        buffer.add(currentPuyo->getPuyoY());
    }
    neutralsBuffer.flush();
    moveLeftBuffer.flush();
    moveRightBuffer.flush();
    fallingStepBuffer.flush();
    compTurnBuffer.flush();
    didFallBuffer.flush();
    willVanishBuffer.flush();

    // creation du message
    Message *message = mbox->createMessage();

    message->addInt     (PuyoMessage::GAMEID, gameId);
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameState);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->addBool    (PuyoMessage::PAUSED, paused);
    message->addIntArray(PuyoMessage::PUYOS,  buffer);
    message->addInt     (PuyoMessage::SCORE,  attachedGame->getGameStat().points);
    message->addInt     (PuyoMessage::NEXT_F, attachedGame->getNextFalling());
    message->addInt     (PuyoMessage::NEXT_C, attachedGame->getNextCompanion());
    message->addInt     (PuyoMessage::SEMI_MOVE, attachedGame->getSemiMove());
    message->addInt     (PuyoMessage::CURRENT_NEUTRALS, attachedGame->getNeutralPuyos());

    message->addIntArray(PuyoMessage::ADD_NEUTRALS,  neutralsBuffer);
    message->addIntArray(PuyoMessage::MV_L,moveLeftBuffer);
    message->addIntArray(PuyoMessage::MV_R,moveRightBuffer);
    message->addIntArray(PuyoMessage::MV_D,fallingStepBuffer);
    message->addIntArray(PuyoMessage::COMPANION_TURN,compTurnBuffer);
    message->addIntArray(PuyoMessage::DID_FALL,      didFallBuffer);
    message->addIntArray(PuyoMessage::WILL_VANISH,   willVanishBuffer);
    message->addInt     (PuyoMessage::NUMBER_BAD_PUYOS, badPuyos);

    // clear des infos ayant ete envoyee
    neutralsBuffer.clear();
    moveLeftBuffer.clear();
    moveRightBuffer.clear();
    fallingStepBuffer.clear();
    compTurnBuffer.clear();
    didFallBuffer.clear();
    willVanishBuffer.clear();
    return message;
}

void PuyoNetworkView::sendStateMessage(bool paused)
{
    Message *message = createStateMessage(paused);
    message->send();
    delete message;
}


void PuyoNetworkView::cycleGame()
{
    PuyoView::cycleGame();
    sendStateMessage();
}

void PuyoNetworkView::moveLeft()
{
    PuyoView::moveLeft();
    sendStateMessage();
}

void PuyoNetworkView::moveRight()
{
    PuyoView::moveRight();
    sendStateMessage();
}

void PuyoNetworkView::rotateLeft()
{
    PuyoView::rotateLeft();
    sendStateMessage();
}

void PuyoNetworkView::rotateRight()
{
    PuyoView::rotateRight();
    sendStateMessage();
}

// PuyoDelegate methods
void PuyoNetworkView::fallingsDidMoveLeft(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    PuyoView::fallingsDidMoveLeft(fallingPuyo, companionPuyo);
    moveLeftBuffer.add(fallingPuyo->getID());
    moveLeftBuffer.add(companionPuyo->getID());
}

void PuyoNetworkView::fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    PuyoView::fallingsDidMoveRight(fallingPuyo, companionPuyo);
    moveRightBuffer.add(fallingPuyo->getID());
    moveRightBuffer.add(companionPuyo->getID());
}

void PuyoNetworkView::fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    PuyoView::fallingsDidFallingStep(fallingPuyo, companionPuyo);
    fallingStepBuffer.add(fallingPuyo->getID());
    fallingStepBuffer.add(companionPuyo->getID());
}

void PuyoNetworkView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex)
{
    PuyoView::gameDidAddNeutral(neutralPuyo, neutralIndex);
    neutralsBuffer.add(neutralPuyo->getID());
    neutralsBuffer.add(neutralIndex);
}

void PuyoNetworkView::gameDidEndCycle()
{
    PuyoView::gameDidEndCycle();
    if (attachedGame->getNeutralPuyos() < 0)
        badPuyos -= attachedGame->getNeutralPuyos();
}

void PuyoNetworkView::companionDidTurn(PuyoPuyo *companionPuyo, PuyoPuyo *fallingPuyo, bool counterclockwise)
{
    PuyoView::companionDidTurn(companionPuyo, fallingPuyo, counterclockwise);
    compTurnBuffer.add(companionPuyo->getID());
    compTurnBuffer.add(fallingPuyo->getID());
    compTurnBuffer.add(counterclockwise);
}

void PuyoNetworkView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow)
{
    PuyoView::puyoDidFall(puyo, originX, originY, nFalledBelow);
    didFallBuffer.add(puyo->getID());
    didFallBuffer.add(originX);
    didFallBuffer.add(originY);
    didFallBuffer.add(nFalledBelow);
}

void PuyoNetworkView::puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase)
{
    PuyoView::puyoWillVanish(puyoGroup, groupNum, phase);
    willVanishBuffer.add(phase);
    willVanishBuffer.add(groupNum);
    willVanishBuffer.add(puyoGroup.size());
    for (int i = 0 ; i < puyoGroup.size() ; i++)
    {
        PuyoPuyo *currentPuyo = puyoGroup[i];
        willVanishBuffer.add(currentPuyo->getID());
    }
}

void PuyoNetworkView::gameWin()
{
    PuyoView::gameWin();
    sendEndOfGameMessage(PuyoMessage::kGameOverWon);
}

void PuyoNetworkView::gameLost()
{
    PuyoView::gameLost();
    sendEndOfGameMessage(PuyoMessage::kGameOverLost);
}

void PuyoNetworkView::sendEndOfGameMessage(int messageType)
{
    Message *message = mbox->createMessage();
    message->addInt     (PuyoMessage::GAMEID, gameId);
    message->addInt     (PuyoMessage::TYPE,   messageType);
    message->addString  (PuyoMessage::NAME,   p1name);
    PlayerGameStat &gameStat = attachedGame->getGameStat();
    message->addInt(PuyoMessage::SCORE, gameStat.points);
    message->addInt(PuyoMessage::TOTAL_SCORE, gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        String messageName = String(PuyoMessage::COMBO_COUNT) + i;
        message->addInt(messageName, gameStat.combo_count[i]);
    }
    message->addInt(PuyoMessage::EXPLODE_COUNT, gameStat.explode_count);
    message->addInt(PuyoMessage::DROP_COUNT, gameStat.drop_count);
    message->addInt(PuyoMessage::GHOST_SENT_COUNT, gameStat.ghost_sent_count);
    message->addFloat(PuyoMessage::TIME_LEFT, gameStat.time_left);
    message->addBool(PuyoMessage::IS_DEAD, gameStat.is_dead);
    message->addBool(PuyoMessage::IS_WINNER, gameStat.is_winner);

    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
}


void PuyoInternetNetworkView::gameWin()
{
    PuyoNetworkView::gameWin();
    sendGameResultToServer(1);
}

void PuyoInternetNetworkView::gameLost()
{
    PuyoNetworkView::gameLost();
    sendGameResultToServer(2);
}

void PuyoInternetNetworkView::sendGameResultToServer(int winner)
{
    return; // Buggy for now... disabled TODO
    if (igpbox == NULL) return;
    igpbox->bind(1);
    std::auto_ptr<Message> message (igpbox->createMessage());
    message->addInt   ("CMD",   PUYO_IGP_GAME_OVER);
    message->addInt   ("WINNER", winner);
    message->addInt   (PuyoMessage::GAMEID, gameId);
    message->addString(PuyoMessage::NAME1,  p1name);
    message->addString(PuyoMessage::NAME2,  p2name);
    PlayerGameStat &gameStat = attachedGame->getGameStat();
    message->addInt(PuyoMessage::SCORE, gameStat.points);
    message->addInt(PuyoMessage::TOTAL_SCORE, gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        String messageName = String(PuyoMessage::COMBO_COUNT) + i;
        message->addInt(messageName, gameStat.combo_count[i]);
    }
    message->addInt(PuyoMessage::EXPLODE_COUNT, gameStat.explode_count);
    message->addInt(PuyoMessage::DROP_COUNT, gameStat.drop_count);
    message->addInt(PuyoMessage::GHOST_SENT_COUNT, gameStat.ghost_sent_count);
    message->addFloat(PuyoMessage::TIME_LEFT, gameStat.time_left);
    message->addBool(PuyoMessage::IS_DEAD, gameStat.is_dead);
    message->addBool(PuyoMessage::IS_WINNER, gameStat.is_winner);

    message->addBoolProperty("RELIABLE", true);
    message->send();
    // delete message; auto_ptr will delete it! (even if exception is thrown, which is nice of him)
}
