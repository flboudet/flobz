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

#ifndef PUYONETWORKGAME_H
#define PUYONETWORKGAME_H

#include "PuyoGame.h"
#include "ios_messagebox.h"

using namespace ios_fc;

class PuyoNetworkGame : public PuyoGame, MessageListener {
public:
    PuyoNetworkGame(PuyoFactory *attachedFactory, MessageBox &msgBox);
    ~PuyoNetworkGame();
    
    virtual void onMessage(Message &);
    
    virtual void cycle();
    
    virtual PuyoPuyo *getPuyoAt(int X, int Y) const;
    
    // List access to the PuyoPuyo objects
    virtual int getPuyoCount() const;
    virtual PuyoPuyo *getPuyoAtIndex(int index) const;
    
    virtual PuyoState getNextFalling();
    virtual PuyoState getNextCompanion();
    
    virtual PuyoState getCompanionState() const;
    virtual PuyoState getFallingState() const;
    virtual int getFallingX() const;
    virtual int getFallingY() const;
    
    virtual int getFallingCompanionDir() const;
    virtual PuyoPuyo *getFallingPuyo() const;
    
    virtual void increaseNeutralPuyos(int incr);
    virtual int getNeutralPuyos() const;
    virtual void dropNeutrals();
    virtual bool isGameRunning() const;
    virtual bool isEndOfCycle() const;
    virtual int getColumnHeigth(int colNum) const;
    virtual int getMaxColumnHeight() const;
    virtual int getSamePuyoAround(int X, int Y, PuyoState color);
    virtual int getSemiMove() const;
private:
    void synchronizeState(Message &message);
    void setPuyoAt(int X, int Y, PuyoPuyo *newPuyo);
    PuyoPuyo *findPuyo(int puyoID);
    PuyoPuyo *puyoCells[PUYODIMX * (PUYODIMY+1)];
    PuyoState nextFalling, nextCompanion;
    PuyoPuyo *fakePuyo;
    MessageBox &msgBox;
    AdvancedBuffer<PuyoPuyo *> puyoVector;
    int semiMove;
    int neutralPuyos, sentBadPuyos;
};


#endif // PUYONETWORKGAME_H
