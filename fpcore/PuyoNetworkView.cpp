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

    // Create the message
    Message *message = mbox->createMessage();

    // TODO: Send some of those only in full messages
    message->addInt     (PuyoMessage::GAMEID, gameId);
    message->addInt     (PuyoMessage::TYPE,   PuyoMessage::kGameState);
    message->addInt     (PuyoMessage::SCORE,  attachedGame->getGameStat().points);
    message->addInt     (PuyoMessage::NEXT_F, attachedGame->getNextFalling());
    message->addInt     (PuyoMessage::NEXT_C, attachedGame->getNextCompanion());
    message->addInt     (PuyoMessage::SEMI_MOVE, attachedGame->getSemiMove());
    message->addInt     (PuyoMessage::CURRENT_NEUTRALS, attachedGame->getNeutralFlobos());

    if (sendFullMessage) {
        int puyoCount = attachedGame->getFloboCount();
        AdvancedBuffer<int> buffer(puyoCount * 4);
        for (int i = 0 ; i < puyoCount ; i++) {
            Flobo *currentPuyo = attachedGame->getFloboAtIndex(i);
            buffer.add(currentPuyo->getID());
            buffer.add(currentPuyo->getFloboState());
            buffer.add(currentPuyo->getFloboX());
            buffer.add(currentPuyo->getFloboY());
        }
        message->addIntArray(PuyoMessage::PUYOS, buffer);
    }

    // TODO: Add those only if not empty
    if (neutralsBuffer.size() > 0)
        message->addIntArray(PuyoMessage::ADD_NEUTRALS,  neutralsBuffer);
    if (moveLeftBuffer.size() > 0)
        message->addIntArray(PuyoMessage::MV_L,moveLeftBuffer);
    if (moveRightBuffer.size() > 0)
        message->addIntArray(PuyoMessage::MV_R,moveRightBuffer);
    if (fallingStepBuffer.size() > 0)
        message->addIntArray(PuyoMessage::MV_D,fallingStepBuffer);
    if (compTurnBuffer.size() > 0)
        message->addIntArray(PuyoMessage::COMPANION_TURN,compTurnBuffer);
    if (didFallBuffer.size() > 0)
        message->addIntArray(PuyoMessage::DID_FALL, didFallBuffer);
    if (willVanishBuffer.size() > 0)
        message->addIntArray(PuyoMessage::WILL_VANISH,   willVanishBuffer);

    message->addInt     (PuyoMessage::NUMBER_BAD_PUYOS, badPuyos);

    // Clear the buffers after they have been sent
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

// GameListener methods
void PuyoNetworkView::fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    PuyoView::fallingsDidMoveLeft(fallingFlobo, companionFlobo);
    moveLeftBuffer.add(fallingFlobo->getID());
    moveLeftBuffer.add(fallingFlobo->getFloboState());
    moveLeftBuffer.add(fallingFlobo->getFloboX());
    moveLeftBuffer.add(fallingFlobo->getFloboY());
    moveLeftBuffer.add(companionFlobo->getID());
    moveLeftBuffer.add(companionFlobo->getFloboState());
    moveLeftBuffer.add(companionFlobo->getFloboX());
    moveLeftBuffer.add(companionFlobo->getFloboY());
}

void PuyoNetworkView::fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    PuyoView::fallingsDidMoveRight(fallingFlobo, companionFlobo);
    moveRightBuffer.add(fallingFlobo->getID());
    moveRightBuffer.add(fallingFlobo->getFloboState());
    moveRightBuffer.add(fallingFlobo->getFloboX());
    moveRightBuffer.add(fallingFlobo->getFloboY());
    moveRightBuffer.add(companionFlobo->getID());
    moveRightBuffer.add(companionFlobo->getFloboState());
    moveRightBuffer.add(companionFlobo->getFloboX());
    moveRightBuffer.add(companionFlobo->getFloboY());
}

void PuyoNetworkView::fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    PuyoView::fallingsDidFallingStep(fallingFlobo, companionFlobo);
    fallingStepBuffer.add(fallingFlobo->getID());
    fallingStepBuffer.add(fallingFlobo->getFloboState());
    fallingStepBuffer.add(fallingFlobo->getFloboX());
    fallingStepBuffer.add(fallingFlobo->getFloboY());
    fallingStepBuffer.add(companionFlobo->getID());
    fallingStepBuffer.add(companionFlobo->getFloboState());
    fallingStepBuffer.add(companionFlobo->getFloboX());
    fallingStepBuffer.add(companionFlobo->getFloboY());
}

void PuyoNetworkView::gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral)
{
    PuyoView::gameDidAddNeutral(neutralFlobo, neutralIndex, totalNeutral);
    neutralsBuffer.add(neutralFlobo->getID());
    neutralsBuffer.add(neutralFlobo->getFloboState());
    neutralsBuffer.add(neutralFlobo->getFloboX());
    neutralsBuffer.add(neutralFlobo->getFloboY());
    neutralsBuffer.add(neutralIndex);
    neutralsBuffer.add(totalNeutral);
}

void PuyoNetworkView::gameDidEndCycle()
{
    PuyoView::gameDidEndCycle();
    if (attachedGame->getNeutralFlobos() < 0)
        badPuyos -= attachedGame->getNeutralFlobos();
    sendStateMessage(true);
}

void PuyoNetworkView::companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise)
{
    PuyoView::companionDidTurn(companionFlobo, fallingFlobo, counterclockwise);
    compTurnBuffer.add(fallingFlobo->getID());
    compTurnBuffer.add(fallingFlobo->getFloboState());
    compTurnBuffer.add(fallingFlobo->getFloboX());
    compTurnBuffer.add(fallingFlobo->getFloboY());
    compTurnBuffer.add(companionFlobo->getID());
    compTurnBuffer.add(companionFlobo->getFloboState());
    compTurnBuffer.add(companionFlobo->getFloboX());
    compTurnBuffer.add(companionFlobo->getFloboY());
    compTurnBuffer.add(counterclockwise);
}

void PuyoNetworkView::floboDidFall(Flobo *puyo, int originX, int originY, int nFalledBelow)
{
    PuyoView::floboDidFall(puyo, originX, originY, nFalledBelow);
    didFallBuffer.add(puyo->getID());
    didFallBuffer.add(puyo->getFloboState());
    didFallBuffer.add(puyo->getFloboX());
    didFallBuffer.add(puyo->getFloboY());
    didFallBuffer.add(originX);
    didFallBuffer.add(originY);
    didFallBuffer.add(nFalledBelow);
}

void PuyoNetworkView::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    PuyoView::floboWillVanish(floboGroup, groupNum, phase);
    willVanishBuffer.add(phase);
    willVanishBuffer.add(groupNum);
    willVanishBuffer.add(floboGroup.size());
    for (int i = 0 ; i < floboGroup.size() ; i++)
    {
        Flobo *currentPuyo = floboGroup[i];
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
        FPServerIGPMessageBox *igpbox;
        int saveBound;
        SaveIgpBound(FPServerIGPMessageBox *igpbox) : igpbox(igpbox), saveBound(igpbox->getBound()) {}
        ~SaveIgpBound() { igpbox->bind(saveBound); }
};

void PuyoInternetNetworkView::sendGameResultToServer(int winner)
{
    if (igpbox == NULL) return;
    SaveIgpBound saveIgpBound(igpbox);
    igpbox->bind(1);
    std::auto_ptr<Message> message (igpbox->createMessage());
    message->addInt   ("CMD",   FLOBO_IGP_GAME_OVER);
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

