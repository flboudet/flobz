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

void PuyoNetworkView::cycleGame() 
{
    PuyoView::cycleGame();
    Message *currentMessage = mbox->createMessage();
    currentMessage->addInt("TYPE", kGameState);
    currentMessage->addBool("PAUSE", false);
    currentMessage->addString("NAME", p1name);
    
    int puyoCount = attachedGame->getPuyoCount();
    AdvancedBuffer<int> buffer(puyoCount * 4);
    
    for (int i = 0 ; i < puyoCount ; i++) {
        PuyoPuyo *currentPuyo = attachedGame->getPuyoAtIndex(i);
        buffer.add(currentPuyo->getID());
        buffer.add(currentPuyo->getPuyoState());
        buffer.add(currentPuyo->getPuyoX());
        buffer.add(currentPuyo->getPuyoY());
    }
    currentMessage->addIntArray("PUYOS", buffer);
    
    currentMessage->send();
    delete currentMessage;
    //  int getPuyoCount() const;
    //  PuyoPuyo *getPuyoAtIndex(int index) const;
}
    
void PuyoNetworkView::moveLeft()
{
    PuyoView::moveLeft();
}

void PuyoNetworkView::moveRight()
{
    PuyoView::moveRight();
}

void PuyoNetworkView::rotateLeft()
{
    PuyoView::rotateLeft();
}

void PuyoNetworkView::rotateRight()
{
    PuyoView::rotateRight();
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

