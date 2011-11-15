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

#include "NetworkGameView.h"
#include "PuyoIgpDefs.h"
#include "GTLog.h"
#include <memory>

Message *NetworkGameView::createStateMessage(bool sendFullMessage)
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
    message->addInt     (FPNetMessage::GAMEID, gameId);
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameState);
    message->addInt     (FPNetMessage::SCORE,  attachedGame->getGameStat().points);
    message->addInt     (FPNetMessage::NEXT_F, attachedGame->getNextFalling());
    message->addInt     (FPNetMessage::NEXT_C, attachedGame->getNextCompanion());
    message->addInt     (FPNetMessage::SEMI_MOVE, attachedGame->getSemiMove());
    message->addInt     (FPNetMessage::CURRENT_NEUTRALS, attachedGame->getNeutralFlobos());

    if (sendFullMessage) {
        int puyoCount = attachedGame->getFloboCount();
        AdvancedBuffer<int> buffer(puyoCount * 4);
        for (int i = 0 ; i < puyoCount ; i++) {
            Flobo *currentFlobo = attachedGame->getFloboAtIndex(i);
            buffer.add(currentFlobo->getID());
            buffer.add(currentFlobo->getFloboState());
            buffer.add(currentFlobo->getFloboX());
            buffer.add(currentFlobo->getFloboY());
        }
        message->addIntArray(FPNetMessage::FLOBOS, buffer);
    }

    // TODO: Add those only if not empty
    if (neutralsBuffer.size() > 0)
        message->addIntArray(FPNetMessage::ADD_NEUTRALS,  neutralsBuffer);
    if (moveLeftBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_L,moveLeftBuffer);
    if (moveRightBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_R,moveRightBuffer);
    if (fallingStepBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_D,fallingStepBuffer);
    if (compTurnBuffer.size() > 0)
        message->addIntArray(FPNetMessage::COMPANION_TURN,compTurnBuffer);
    if (didFallBuffer.size() > 0)
        message->addIntArray(FPNetMessage::DID_FALL, didFallBuffer);
    if (willVanishBuffer.size() > 0)
        message->addIntArray(FPNetMessage::WILL_VANISH,   willVanishBuffer);

    message->addInt     (FPNetMessage::NUMBER_BAD_FLOBOS, badFlobos);

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

void NetworkGameView::sendStateMessage(bool sendFullMessage)
{
    //double initial = ios_fc::getTimeMs();
    Message *message = createStateMessage(sendFullMessage);
    message->send();
    delete message;
    //cout << "Time taken:" << ios_fc::getTimeMs() - initial << endl;
}


void NetworkGameView::cycleGame()
{
    GameView::cycleGame();
    sendStateMessage();
}

void NetworkGameView::moveLeft()
{
    GameView::moveLeft();
    sendStateMessage();
}

void NetworkGameView::moveRight()
{
    GameView::moveRight();
    sendStateMessage();
}

void NetworkGameView::rotateLeft()
{
    GameView::rotateLeft();
    sendStateMessage();
}

void NetworkGameView::rotateRight()
{
    GameView::rotateRight();
    sendStateMessage();
}

// GameListener methods
void NetworkGameView::fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    GameView::fallingsDidMoveLeft(fallingFlobo, companionFlobo);
    moveLeftBuffer.add(fallingFlobo->getID());
    moveLeftBuffer.add(fallingFlobo->getFloboState());
    moveLeftBuffer.add(fallingFlobo->getFloboX());
    moveLeftBuffer.add(fallingFlobo->getFloboY());
    moveLeftBuffer.add(companionFlobo->getID());
    moveLeftBuffer.add(companionFlobo->getFloboState());
    moveLeftBuffer.add(companionFlobo->getFloboX());
    moveLeftBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    GameView::fallingsDidMoveRight(fallingFlobo, companionFlobo);
    moveRightBuffer.add(fallingFlobo->getID());
    moveRightBuffer.add(fallingFlobo->getFloboState());
    moveRightBuffer.add(fallingFlobo->getFloboX());
    moveRightBuffer.add(fallingFlobo->getFloboY());
    moveRightBuffer.add(companionFlobo->getID());
    moveRightBuffer.add(companionFlobo->getFloboState());
    moveRightBuffer.add(companionFlobo->getFloboX());
    moveRightBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo)
{
    GameView::fallingsDidFallingStep(fallingFlobo, companionFlobo);
    fallingStepBuffer.add(fallingFlobo->getID());
    fallingStepBuffer.add(fallingFlobo->getFloboState());
    fallingStepBuffer.add(fallingFlobo->getFloboX());
    fallingStepBuffer.add(fallingFlobo->getFloboY());
    fallingStepBuffer.add(companionFlobo->getID());
    fallingStepBuffer.add(companionFlobo->getFloboState());
    fallingStepBuffer.add(companionFlobo->getFloboX());
    fallingStepBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral)
{
    GameView::gameDidAddNeutral(neutralFlobo, neutralIndex, totalNeutral);
    neutralsBuffer.add(neutralFlobo->getID());
    neutralsBuffer.add(neutralFlobo->getFloboState());
    neutralsBuffer.add(neutralFlobo->getFloboX());
    neutralsBuffer.add(neutralFlobo->getFloboY());
    neutralsBuffer.add(neutralIndex);
    neutralsBuffer.add(totalNeutral);
}

void NetworkGameView::gameDidEndCycle()
{
    GameView::gameDidEndCycle();
    if (attachedGame->getNeutralFlobos() < 0)
        badFlobos -= attachedGame->getNeutralFlobos();
    sendStateMessage(true);
}

void NetworkGameView::companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise)
{
    GameView::companionDidTurn(companionFlobo, fallingFlobo, counterclockwise);
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

void NetworkGameView::floboDidFall(Flobo *puyo, int originX, int originY, int nFalledBelow)
{
    GameView::floboDidFall(puyo, originX, originY, nFalledBelow);
    didFallBuffer.add(puyo->getID());
    didFallBuffer.add(puyo->getFloboState());
    didFallBuffer.add(puyo->getFloboX());
    didFallBuffer.add(puyo->getFloboY());
    didFallBuffer.add(originX);
    didFallBuffer.add(originY);
    didFallBuffer.add(nFalledBelow);
}

void NetworkGameView::floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase)
{
    GameView::floboWillVanish(floboGroup, groupNum, phase);
    willVanishBuffer.add(phase);
    willVanishBuffer.add(groupNum);
    willVanishBuffer.add(floboGroup.size());
    for (int i = 0 ; i < floboGroup.size() ; i++)
    {
        Flobo *currentFlobo = floboGroup[i];
        willVanishBuffer.add(currentFlobo->getID());
    }
}

void NetworkGameView::gameWin()
{
    GameView::gameWin();
    sendEndOfGameMessage(FPNetMessage::kGameOverWon);
}

void NetworkGameView::gameLost()
{
    GameView::gameLost();
    sendEndOfGameMessage(FPNetMessage::kGameOverLost);
}

void NetworkGameView::sendEndOfGameMessage(int messageType)
{
    Message *message = mbox->createMessage();
    message->addInt     (FPNetMessage::GAMEID, gameId);
    message->addInt     (FPNetMessage::TYPE,   messageType);
    message->addString  (FPNetMessage::NAME,   p1name);
    PlayerGameStat &gameStat = attachedGame->getGameStat();
    message->addInt(FPNetMessage::SCORE, gameStat.points);
    message->addInt(FPNetMessage::TOTAL_SCORE, gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        String messageName = String(FPNetMessage::COMBO_COUNT) + i;
        message->addInt(messageName, gameStat.combo_count[i]);
    }
    message->addInt(FPNetMessage::EXPLODE_COUNT, gameStat.explode_count);
    message->addInt(FPNetMessage::DROP_COUNT, gameStat.drop_count);
    message->addInt(FPNetMessage::GHOST_SENT_COUNT, gameStat.ghost_sent_count);
    message->addFloat(FPNetMessage::TIME_LEFT, gameStat.time_left);
    message->addBool(FPNetMessage::IS_DEAD, gameStat.is_dead);
    message->addBool(FPNetMessage::IS_WINNER, gameStat.is_winner);

    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
}


void InternetGameView::gameWin()
{
    NetworkGameView::gameWin();
    sendGameResultToServer(1);
}

void InternetGameView::gameLost()
{
    NetworkGameView::gameLost();
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

void InternetGameView::sendGameResultToServer(int winner)
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

