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

extern char *p1name;

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
        
    // creation du message
    Message *message = mbox->createMessage();

    message->addInt     (PuyoMessage::TYPE,   kGameState);
    message->addString  (PuyoMessage::NAME,   p1name);
    message->addBool    (PuyoMessage::PAUSED, paused);
    message->addIntArray(PuyoMessage::PUYOS,  buffer);
    message->addInt     (PuyoMessage::SCORE,  attachedGame->getPoints());
    message->addInt     (PuyoMessage::NEXT_F, attachedGame->getNextFalling());
    message->addInt     (PuyoMessage::NEXT_C, attachedGame->getNextCompanion());
    
    message->addIntArray(PuyoMessage::ADD_NEUTRALS,  neutralsBuffer);
    message->addBool    (PuyoMessage::DID_END_CYCLE, didEndCycle);
    
    // clear des infos ayant ete envoyee
    neutralsBuffer.clear();
    didEndCycle = false;
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
void PuyoNetworkView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex)
{
    PuyoView::gameDidAddNeutral(neutralPuyo, neutralIndex);
    neutralsBuffer.add(neutralPuyo->getID());
    neutralsBuffer.add(neutralIndex);
}

void PuyoNetworkView::gameDidEndCycle()
{
    PuyoView::gameDidEndCycle();
    didEndCycle = true;
}

void PuyoNetworkView::companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise)
{
    PuyoView::companionDidTurn(companionPuyo, companionVector, counterclockwise);
}

void PuyoNetworkView::puyoDidFall(PuyoPuyo *puyo, int originX, int originY)
{
    PuyoView::puyoDidFall(puyo, originX, originY);
}

void PuyoNetworkView::puyoWillVanish(IosVector &puyoGroup, int groupNum, int phase)
{
    PuyoView::puyoWillVanish(puyoGroup, groupNum, phase);
}

void PuyoNetworkView::gameLost()
{
    PuyoView::gameLost();
    Message *message = mbox->createMessage();
    message->addInt     ("TYPE",   kGameOver);
    message->addString  ("NAME",   p1name);
    message->addInt     ("SCORE",  attachedGame->getPoints());
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
}

