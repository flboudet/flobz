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

enum {
    kGameState = 0
};

extern char *p1name;

Message *PuyoNetworkView::createStateMessage(bool paused) const
{
    Message *message = mbox->createMessage();

    int puyoCount = attachedGame->getPuyoCount();
    AdvancedBuffer<int> buffer(puyoCount * 4);

    for (int i = 0 ; i < puyoCount ; i++) {
        PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
        buffer.add(currentPuyo->getID());
        buffer.add(currentPuyo->getPuyoState());
        buffer.add(currentPuyo->getPuyoX());
        buffer.add(currentPuyo->getPuyoY());
    }
    
    message->addInt     ("TYPE",  kGameState);
    message->addString  ("NAME",  p1name);
    message->addBool    ("PAUSE", paused);
    message->addIntArray("PUYOS", buffer);
    return message;
}

void PuyoNetworkView::cycleGame() 
{
    PuyoView::cycleGame();
    Message *message = createStateMessage();
    message->send();
    delete message;
}

void PuyoNetworkView::moveLeft()
{
    PuyoView::moveLeft();
    Message *message = createStateMessage();
    message->send();
    delete message;
}

void PuyoNetworkView::moveRight()
{
    PuyoView::moveRight();
    Message *message = createStateMessage();
    message->send();
    delete message;
}

void PuyoNetworkView::rotateLeft()
{
    PuyoView::rotateLeft();
    Message *message = createStateMessage();
    message->send();
    delete message;
}

void PuyoNetworkView::rotateRight()
{
    PuyoView::rotateRight();
    Message *message = createStateMessage();
    message->send();
    delete message;
}

// PuyoDelegate methods
void PuyoNetworkView::gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex)
{
    PuyoView::gameDidAddNeutral(neutralPuyo, neutralIndex);
}

void PuyoNetworkView::gameDidEndCycle()
{
    PuyoView::gameDidEndCycle();
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
}

