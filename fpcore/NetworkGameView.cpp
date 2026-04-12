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
#include "FPIGPDefs.h"
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
        int floboCount = attachedGame->getFloboCount();
        AdvancedBuffer<int> buffer(floboCount * 4);
        for (FloboDefaultIterator iter(attachedGame) ;
             ! iter.end() ; ++iter) {
            auto currentFlobo = iter.get();
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
void NetworkGameView::fallingsDidMoveLeft(std::shared_ptr<Flobo> _fallingFlobo, std::shared_ptr<Flobo> _companionFlobo)
{
    GameView::fallingsDidMoveLeft(_fallingFlobo, _companionFlobo);
    moveLeftBuffer.add(_fallingFlobo->getID());
    moveLeftBuffer.add(_fallingFlobo->getFloboState());
    moveLeftBuffer.add(_fallingFlobo->getFloboX());
    moveLeftBuffer.add(_fallingFlobo->getFloboY());
    moveLeftBuffer.add(_companionFlobo->getID());
    moveLeftBuffer.add(_companionFlobo->getFloboState());
    moveLeftBuffer.add(_companionFlobo->getFloboX());
    moveLeftBuffer.add(_companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidMoveRight(std::shared_ptr<Flobo> _fallingFlobo, std::shared_ptr<Flobo> _companionFlobo)
{
    GameView::fallingsDidMoveRight(_fallingFlobo, _companionFlobo);
    moveRightBuffer.add(_fallingFlobo->getID());
    moveRightBuffer.add(_fallingFlobo->getFloboState());
    moveRightBuffer.add(_fallingFlobo->getFloboX());
    moveRightBuffer.add(_fallingFlobo->getFloboY());
    moveRightBuffer.add(_companionFlobo->getID());
    moveRightBuffer.add(_companionFlobo->getFloboState());
    moveRightBuffer.add(_companionFlobo->getFloboX());
    moveRightBuffer.add(_companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidFallingStep(std::shared_ptr<Flobo> _fallingFlobo, std::shared_ptr<Flobo> _companionFlobo)
{
    GameView::fallingsDidFallingStep(_fallingFlobo, _companionFlobo);
    fallingStepBuffer.add(_fallingFlobo->getID());
    fallingStepBuffer.add(_fallingFlobo->getFloboState());
    fallingStepBuffer.add(_fallingFlobo->getFloboX());
    fallingStepBuffer.add(_fallingFlobo->getFloboY());
    fallingStepBuffer.add(_companionFlobo->getID());
    fallingStepBuffer.add(_companionFlobo->getFloboState());
    fallingStepBuffer.add(_companionFlobo->getFloboX());
    fallingStepBuffer.add(_companionFlobo->getFloboY());
}

void NetworkGameView::gameDidAddNeutral(std::shared_ptr<Flobo> neutralFlobo, int neutralIndex, int totalNeutral)
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

void NetworkGameView::companionDidTurn(std::shared_ptr<Flobo> _companionFlobo, std::shared_ptr<Flobo> _fallingFlobo, bool counterclockwise)
{
    GameView::companionDidTurn(_companionFlobo, _fallingFlobo, counterclockwise);
    compTurnBuffer.add(_fallingFlobo->getID());
    compTurnBuffer.add(_fallingFlobo->getFloboState());
    compTurnBuffer.add(_fallingFlobo->getFloboX());
    compTurnBuffer.add(_fallingFlobo->getFloboY());
    compTurnBuffer.add(_companionFlobo->getID());
    compTurnBuffer.add(_companionFlobo->getFloboState());
    compTurnBuffer.add(_companionFlobo->getFloboX());
    compTurnBuffer.add(_companionFlobo->getFloboY());
    compTurnBuffer.add(counterclockwise);
}

void NetworkGameView::floboDidFall(std::shared_ptr<Flobo> flobo, int originX, int originY, int nFalledBelow)
{
    GameView::floboDidFall(flobo, originX, originY, nFalledBelow);
    didFallBuffer.add(flobo->getID());
    didFallBuffer.add(flobo->getFloboState());
    didFallBuffer.add(flobo->getFloboX());
    didFallBuffer.add(flobo->getFloboY());
    didFallBuffer.add(originX);
    didFallBuffer.add(originY);
    didFallBuffer.add(nFalledBelow);
}

void NetworkGameView::floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int _phase)
{
    GameView::floboWillVanish(floboGroup, groupNum, _phase);
    willVanishBuffer.add(_phase);
    willVanishBuffer.add(groupNum);
    willVanishBuffer.add(floboGroup.size());
    for (int i = 0 ; i < floboGroup.size() ; i++)
    {
        Flobo *currentFlobo = floboGroup[i].get();
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
    message->addString  (FPNetMessage::NAME,   p1name.c_str());
    PlayerGameStat &_gameStat = attachedGame->getGameStat();
    message->addInt(FPNetMessage::SCORE, _gameStat.points);
    message->addInt(FPNetMessage::TOTAL_SCORE, _gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        std::string messageName = std::string(FPNetMessage::COMBO_COUNT) + std::to_string(i);
        message->addInt(messageName, _gameStat.combo_count[i]);
    }
    message->addInt(FPNetMessage::EXPLODE_COUNT, _gameStat.explode_count);
    message->addInt(FPNetMessage::DROP_COUNT, _gameStat.drop_count);
    message->addInt(FPNetMessage::GHOST_SENT_COUNT, _gameStat.ghost_sent_count);
    message->addFloat(FPNetMessage::TIME_LEFT, _gameStat.time_left);
    message->addBool(FPNetMessage::IS_DEAD, _gameStat.is_dead);
    message->addBool(FPNetMessage::IS_WINNER, _gameStat.is_winner);

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
    std::unique_ptr<Message> message (igpbox->createMessage());
    message->addInt   ("CMD",   FLOBO_IGP_GAME_OVER);
    message->addInt   ("WINNER", winner);
    message->addInt   ("GAMEID", gameId);
    message->addString("NAME1",  p1name.c_str());
    message->addString("NAME2",  p2name.c_str());
    PlayerGameStat &_gameStat = attachedGame->getGameStat();
    message->addInt("SCORE", _gameStat.points);
    message->addInt("TOTAL_SCORE", _gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        std::string messageName = std::string("COMBO_COUNT") + std::to_string(i);
        message->addInt(messageName, _gameStat.combo_count[i]);
    }
    message->addInt("EXPLODE_COUNT", _gameStat.explode_count);
    message->addInt("DROP_COUNT", _gameStat.drop_count);
    message->addInt("GHOST_SENT_COUNT", _gameStat.ghost_sent_count);
    message->addFloat("TIME_LEFT", _gameStat.time_left);
    message->addBool("IS_DEAD", _gameStat.is_dead);
    message->addBool("IS_WINNER", _gameStat.is_winner);

    message->addBoolProperty("RELIABLE", true);
    message->send();
    // delete message; unique_ptr will delete it! (even if exception is thrown, which is nice of him)
    // SaveIgpBound restore bound peer.
}

