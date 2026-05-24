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

    _neutralsBuffer.flush(); // TODO: Voir ce que deviennent ces flush
    _moveLeftBuffer.flush();
    _moveRightBuffer.flush();
    _fallingStepBuffer.flush();
    _compTurnBuffer.flush();
    _didFallBuffer.flush();
    _willVanishBuffer.flush();

    // Create the message
    Message *message = _mbox->createMessage();

    // TODO: Send some of those only in full messages
    message->addInt     (FPNetMessage::GAMEID, _gameId);
    message->addInt     (FPNetMessage::TYPE,   FPNetMessage::kGameState);
    message->addInt     (FPNetMessage::SCORE,  _attachedGame->getGameStat().points);
    message->addInt     (FPNetMessage::NEXT_F, _attachedGame->getNextFalling());
    message->addInt     (FPNetMessage::NEXT_C, _attachedGame->getNextCompanion());
    message->addInt     (FPNetMessage::SEMI_MOVE, _attachedGame->getSemiMove());
    message->addInt     (FPNetMessage::CURRENT_NEUTRALS, _attachedGame->getNeutralFlobos());

    if (sendFullMessage) {
        int floboCount = _attachedGame->getFloboCount();
        AdvancedBuffer<int> buffer(floboCount * 4);
        for (FloboDefaultIterator iter(_attachedGame) ;
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
    if (_neutralsBuffer.size() > 0)
        message->addIntArray(FPNetMessage::ADD_NEUTRALS,  _neutralsBuffer);
    if (_moveLeftBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_L,_moveLeftBuffer);
    if (_moveRightBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_R,_moveRightBuffer);
    if (_fallingStepBuffer.size() > 0)
        message->addIntArray(FPNetMessage::MV_D,_fallingStepBuffer);
    if (_compTurnBuffer.size() > 0)
        message->addIntArray(FPNetMessage::COMPANION_TURN,_compTurnBuffer);
    if (_didFallBuffer.size() > 0)
        message->addIntArray(FPNetMessage::DID_FALL, _didFallBuffer);
    if (_willVanishBuffer.size() > 0)
        message->addIntArray(FPNetMessage::WILL_VANISH,   _willVanishBuffer);

    message->addInt     (FPNetMessage::NUMBER_BAD_FLOBOS, _badFlobos);

    // Clear the buffers after they have been sent
    _neutralsBuffer.clear();
    _moveLeftBuffer.clear();
    _moveRightBuffer.clear();
    _fallingStepBuffer.clear();
    _compTurnBuffer.clear();
    _didFallBuffer.clear();
    _willVanishBuffer.clear();
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
void NetworkGameView::fallingsDidMoveLeft(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    GameView::fallingsDidMoveLeft(fallingFlobo, companionFlobo);
    _moveLeftBuffer.add(fallingFlobo->getID());
    _moveLeftBuffer.add(fallingFlobo->getFloboState());
    _moveLeftBuffer.add(fallingFlobo->getFloboX());
    _moveLeftBuffer.add(fallingFlobo->getFloboY());
    _moveLeftBuffer.add(companionFlobo->getID());
    _moveLeftBuffer.add(companionFlobo->getFloboState());
    _moveLeftBuffer.add(companionFlobo->getFloboX());
    _moveLeftBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidMoveRight(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    GameView::fallingsDidMoveRight(fallingFlobo, companionFlobo);
    _moveRightBuffer.add(fallingFlobo->getID());
    _moveRightBuffer.add(fallingFlobo->getFloboState());
    _moveRightBuffer.add(fallingFlobo->getFloboX());
    _moveRightBuffer.add(fallingFlobo->getFloboY());
    _moveRightBuffer.add(companionFlobo->getID());
    _moveRightBuffer.add(companionFlobo->getFloboState());
    _moveRightBuffer.add(companionFlobo->getFloboX());
    _moveRightBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::fallingsDidFallingStep(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo)
{
    GameView::fallingsDidFallingStep(fallingFlobo, companionFlobo);
    _fallingStepBuffer.add(fallingFlobo->getID());
    _fallingStepBuffer.add(fallingFlobo->getFloboState());
    _fallingStepBuffer.add(fallingFlobo->getFloboX());
    _fallingStepBuffer.add(fallingFlobo->getFloboY());
    _fallingStepBuffer.add(companionFlobo->getID());
    _fallingStepBuffer.add(companionFlobo->getFloboState());
    _fallingStepBuffer.add(companionFlobo->getFloboX());
    _fallingStepBuffer.add(companionFlobo->getFloboY());
}

void NetworkGameView::gameDidAddNeutral(std::shared_ptr<Flobo> neutralFlobo, int neutralIndex, int totalNeutral)
{
    GameView::gameDidAddNeutral(neutralFlobo, neutralIndex, totalNeutral);
    _neutralsBuffer.add(neutralFlobo->getID());
    _neutralsBuffer.add(neutralFlobo->getFloboState());
    _neutralsBuffer.add(neutralFlobo->getFloboX());
    _neutralsBuffer.add(neutralFlobo->getFloboY());
    _neutralsBuffer.add(neutralIndex);
    _neutralsBuffer.add(totalNeutral);
}

void NetworkGameView::gameDidEndCycle()
{
    GameView::gameDidEndCycle();
    if (_attachedGame->getNeutralFlobos() < 0)
        _badFlobos -= _attachedGame->getNeutralFlobos();
    sendStateMessage(true);
}

void NetworkGameView::companionDidTurn(std::shared_ptr<Flobo> companionFlobo, std::shared_ptr<Flobo> fallingFlobo, bool counterclockwise)
{
    GameView::companionDidTurn(companionFlobo, fallingFlobo, counterclockwise);
    _compTurnBuffer.add(fallingFlobo->getID());
    _compTurnBuffer.add(fallingFlobo->getFloboState());
    _compTurnBuffer.add(fallingFlobo->getFloboX());
    _compTurnBuffer.add(fallingFlobo->getFloboY());
    _compTurnBuffer.add(companionFlobo->getID());
    _compTurnBuffer.add(companionFlobo->getFloboState());
    _compTurnBuffer.add(companionFlobo->getFloboX());
    _compTurnBuffer.add(companionFlobo->getFloboY());
    _compTurnBuffer.add(counterclockwise);
}

void NetworkGameView::floboDidFall(std::shared_ptr<Flobo> flobo, int originX, int originY, int nFalledBelow)
{
    GameView::floboDidFall(flobo, originX, originY, nFalledBelow);
    _didFallBuffer.add(flobo->getID());
    _didFallBuffer.add(flobo->getFloboState());
    _didFallBuffer.add(flobo->getFloboX());
    _didFallBuffer.add(flobo->getFloboY());
    _didFallBuffer.add(originX);
    _didFallBuffer.add(originY);
    _didFallBuffer.add(nFalledBelow);
}

void NetworkGameView::floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase)
{
    GameView::floboWillVanish(floboGroup, groupNum, phase);
    _willVanishBuffer.add(phase);
    _willVanishBuffer.add(groupNum);
    _willVanishBuffer.add(floboGroup.size());
    for (int i = 0 ; i < floboGroup.size() ; i++)
    {
        Flobo *currentFlobo = floboGroup[i].get();
        _willVanishBuffer.add(currentFlobo->getID());
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
    Message *message = _mbox->createMessage();
    message->addInt     (FPNetMessage::GAMEID, _gameId);
    message->addInt     (FPNetMessage::TYPE,   messageType);
    message->addString  (FPNetMessage::NAME,   _p1name.c_str());
    PlayerGameStat &gameStat = _attachedGame->getGameStat();
    message->addInt(FPNetMessage::SCORE, gameStat.points);
    message->addInt(FPNetMessage::TOTAL_SCORE, gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        std::string messageName = std::string(FPNetMessage::COMBO_COUNT) + std::to_string(i);
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
    if (_igpbox == NULL) return;
    SaveIgpBound saveIgpBound(_igpbox);
    _igpbox->bind(1);
    std::unique_ptr<Message> message (_igpbox->createMessage());
    message->addInt   ("CMD",   FLOBO_IGP_GAME_OVER);
    message->addInt   ("WINNER", winner);
    message->addInt   ("GAMEID", _gameId);
    message->addString("NAME1",  _p1name.c_str());
    message->addString("NAME2",  _p2name.c_str());
    PlayerGameStat &gameStat = _attachedGame->getGameStat();
    message->addInt("SCORE", gameStat.points);
    message->addInt("TOTAL_SCORE", gameStat.total_points);
    for (int i = 0 ; i < 24 ; i++) {
        std::string messageName = std::string("COMBO_COUNT") + std::to_string(i);
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
    // delete message; unique_ptr will delete it! (even if exception is thrown, which is nice of him)
    // SaveIgpBound restore bound peer.
}

