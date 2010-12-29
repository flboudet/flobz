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
#include "GTLog.h"
#include <memory>

Message *PuyoNetworkView::createStateMessage(bool sendFullMessage)
{
    // preparation des infos */

    neutralsBuffer.flush(); // TODO: Voir ce que deviennent ces flush
    moveLeftBuffer.flush();
    moveRightBuffer.flush();
    fallingStepBuffer.flush();
    compTurnBuffer.flush();
    didFallBuffer.flush();
    willVanishBuffer.flush();

    // creation du message
    Message *message = mbox->createMessage();

    // TODO: Send some of those only in full messages
    message->addInt     (PuyoMessage::GAMEID, gameId);
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameState);
    message->addString  (PuyoMessage::NAME,   p1name);
    // message->addBool    (PuyoMessage::PAUSED, false); // paused);
    message->addInt     (PuyoMessage::SCORE,  attachedGame->getGameStat().points);
    message->addInt     (PuyoMessage::NEXT_F, attachedGame->getNextFalling());
    message->addInt     (PuyoMessage::NEXT_C, attachedGame->getNextCompanion());
    message->addInt     (PuyoMessage::SEMI_MOVE, attachedGame->getSemiMove());
    message->addInt     (PuyoMessage::CURRENT_NEUTRALS, attachedGame->getNeutralPuyos());

    if (sendFullMessage) {
        int puyoCount = attachedGame->getPuyoCount();
        AdvancedBuffer<int> buffer(puyoCount * 4);
        for (int i = 0 ; i < puyoCount ; i++) {
            PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
            buffer.add(currentPuyo->getID());
            buffer.add(currentPuyo->getPuyoState());
            buffer.add(currentPuyo->getPuyoX());
            buffer.add(currentPuyo->getPuyoY());
        }
        message->addIntArray(PuyoMessage::PUYOS, buffer);
    }

    // TODO: Add those only if not empty
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

void PuyoNetworkView::sendStateMessage(bool sendFullMessage)
{
    //double initial = ios_fc::getTimeMs();
    Message *message = createStateMessage(sendFullMessage);
    message->send();
    delete message;
    //cout << "Time taken:" << ios_fc::getTimeMs() - initial << endl;
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
    moveLeftBuffer.add(fallingPuyo->getPuyoState());
    moveLeftBuffer.add(fallingPuyo->getPuyoX());
    moveLeftBuffer.add(fallingPuyo->getPuyoY());
    moveLeftBuffer.add(companionPuyo->getID());
    moveLeftBuffer.add(companionPuyo->getPuyoState());
    moveLeftBuffer.add(companionPuyo->getPuyoX());
    moveLeftBuffer.add(companionPuyo->getPuyoY());
}

void PuyoNetworkView::fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    PuyoView::fallingsDidMoveRight(fallingPuyo, companionPuyo);
    moveRightBuffer.add(fallingPuyo->getID());
    moveRightBuffer.add(fallingPuyo->getPuyoState());
    moveRightBuffer.add(fallingPuyo->getPuyoX());
    moveRightBuffer.add(fallingPuyo->getPuyoY());
    moveRightBuffer.add(companionPuyo->getID());
    moveRightBuffer.add(companionPuyo->getPuyoState());
    moveRightBuffer.add(companionPuyo->getPuyoX());
    moveRightBuffer.add(companionPuyo->getPuyoY());
}

void PuyoNetworkView::fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo)
{
    PuyoView::fallingsDidFallingStep(fallingPuyo, companionPuyo);
    fallingStepBuffer.add(fallingPuyo->getID());
    fallingStepBuffer.add(fallingPuyo->getPuyoState());
    fallingStepBuffer.add(fallingPuyo->getPuyoX());
    fallingStepBuffer.add(fallingPuyo->getPuyoY());
    fallingStepBuffer.add(companionPuyo->getID());
    fallingStepBuffer.add(companionPuyo->getPuyoState());
    fallingStepBuffer.add(companionPuyo->getPuyoX());
    fallingStepBuffer.add(companionPuyo->getPuyoY());
}

void PuyoNetworkView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex)
{
    PuyoView::gameDidAddNeutral(neutralPuyo, neutralIndex);
    neutralsBuffer.add(neutralPuyo->getID());
    neutralsBuffer.add(neutralPuyo->getPuyoState());
    neutralsBuffer.add(neutralPuyo->getPuyoX());
    neutralsBuffer.add(neutralPuyo->getPuyoY());
    neutralsBuffer.add(neutralIndex);
}

void PuyoNetworkView::gameDidEndCycle()
{
    PuyoView::gameDidEndCycle();
    if (attachedGame->getNeutralPuyos() < 0)
        badPuyos -= attachedGame->getNeutralPuyos();
    sendStateMessage(true);
}

void PuyoNetworkView::companionDidTurn(PuyoPuyo *companionPuyo, PuyoPuyo *fallingPuyo, bool counterclockwise)
{
    PuyoView::companionDidTurn(companionPuyo, fallingPuyo, counterclockwise);
    compTurnBuffer.add(fallingPuyo->getID());
    compTurnBuffer.add(fallingPuyo->getPuyoState());
    compTurnBuffer.add(fallingPuyo->getPuyoX());
    compTurnBuffer.add(fallingPuyo->getPuyoY());
    compTurnBuffer.add(companionPuyo->getID());
    compTurnBuffer.add(companionPuyo->getPuyoState());
    compTurnBuffer.add(companionPuyo->getPuyoX());
    compTurnBuffer.add(companionPuyo->getPuyoY());
    compTurnBuffer.add(counterclockwise);
}

void PuyoNetworkView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow)
{
    PuyoView::puyoDidFall(puyo, originX, originY, nFalledBelow);
    didFallBuffer.add(puyo->getID());
    didFallBuffer.add(puyo->getPuyoState());
    didFallBuffer.add(puyo->getPuyoX());
    didFallBuffer.add(puyo->getPuyoY());
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

// Save and restore IgpMessageBox bound peer.
class SaveIgpBound {
    public:
        IgpMessageBox *igpbox;
        int saveBound;
        SaveIgpBound(IgpMessageBox *igpbox) : igpbox(igpbox), saveBound(igpbox->getBound()) {}
        ~SaveIgpBound() { igpbox->bind(saveBound); }
};

void PuyoInternetNetworkView::sendGameResultToServer(int winner)
{
    if (igpbox == NULL) return;
    SaveIgpBound saveIgpBound(igpbox);
    igpbox->bind(1);
    std::auto_ptr<Message> message (igpbox->createMessage());
    message->addInt   ("CMD",   PUYO_IGP_GAME_OVER);
    message->addInt   ("WINNER", winner);
    message->addInt   ("GAMEID", gameId);
    message->addString("NAME1",  p1name);
    message->addString("NAME2",  p2name);
    PlayerGameStat &gameStat = attachedGame->getGameStat();
    message->addInt("SCORE", gameStat.points);
    message->addInt("TOTAL_SCORE", gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        String messageName = String("COMBO_COUNT") + i;
        message->addInt(messageName, gameStat.combo_count[i]);
    }
    message->addInt("EXPLODE_COUNT", gameStat.explode_count);
    message->addInt("DROP_COUNT", gameStat.drop_count);
    message->addInt("GHOST_SENT_COUNT", gameStat.ghost_sent_count);
    message->addFloat("TIME_LEFT", gameStat.time_left);
    message->addBool("IS_DEAD", gameStat.is_dead);
    message->addBool("IS_WINNER", gameStat.is_winner);

    message->addBoolProperty("RELIABLE", true);
    message->send();
    // delete message; auto_ptr will delete it! (even if exception is thrown, which is nice of him)
    // SaveIgpBound restore bound peer.
}

