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

using namespace PuyoMessage;

PuyoNetworkGame::PuyoNetworkGame(PuyoFactory *attachedFactory, MessageBox &msgBox) : PuyoGame(attachedFactory), msgBox(msgBox)
{
    fakePuyo = attachedFactory->createPuyo(PUYO_FALLINGRED);
    msgBox.addListener(*this);
}

void PuyoNetworkGame::onMessage(Message &message)
{
     printf("msg\n");
    int msgType = message.getInt(TYPE);
    switch (msgType) {
        case kGameState:
            synchronizeState(message);
            break;
        default:
            break;
    }
}

void PuyoNetworkGame::synchronizeState(Message &message)
{
    points = message.getInt("SCORE");
}

void PuyoNetworkGame::cycle()
{
}

PuyoPuyo *PuyoNetworkGame::getPuyoAt(int X, int Y) const
{
    return NULL;
}
  
// List access to the PuyoPuyo objects
int PuyoNetworkGame::getPuyoCount() const
{
    return 0;
}

PuyoPuyo *PuyoNetworkGame::getPuyoAtIndex(int index) const
{
    return fakePuyo;
}
  
PuyoState PuyoNetworkGame::getNextFalling()
{
    return PUYO_FALLINGRED;
}

PuyoState PuyoNetworkGame::getNextCompanion()
{
    return PUYO_FALLINGRED;
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
    return 0;
}

void PuyoNetworkGame::dropNeutrals()
{
}

bool PuyoNetworkGame::isGameRunning() const
{
    return true;
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
    return 0;
}

