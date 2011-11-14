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

#include <map>
#include "FloboGame.h"
#include "ios_messagebox.h"

using namespace ios_fc;

class PuyoNetworkGame : public FloboGame, MessageListener {
public:
    PuyoNetworkGame(FloboFactory *attachedFactory, MessageBox &msgBox, int gameId = 0);
    ~PuyoNetworkGame();

    virtual void onMessage(Message &);

    virtual void cycle();

    virtual Flobo *getFloboAt(int X, int Y) const;

    // List access to the Flobo objects
    virtual int getFloboCount() const;
    virtual Flobo *getFloboAtIndex(int index) const;

    virtual FloboState getNextFalling();
    virtual FloboState getNextCompanion();

    virtual FloboState getCompanionState() const;
    virtual FloboState getFallingState() const;
    virtual int getFallingX() const;
    virtual int getFallingY() const;

    virtual int getFallingCompanionDir() const;
    virtual Flobo *getFallingFlobo() const;

    virtual void increaseNeutralFlobos(int incr);
    virtual int getNeutralFlobos() const;
    virtual void dropNeutrals();
    virtual bool isGameRunning() const;
    virtual bool isEndOfCycle() const;
    virtual int getColumnHeigth(int colNum) const;
    virtual int getMaxColumnHeight() const;
    virtual int getSameFloboAround(int X, int Y, FloboState color);
    virtual int getSemiMove() const;
    virtual int  getComboPhase() const { return comboPhase; }
private:
    void synchronizeState(Message &message);
    void setPuyoAt(int X, int Y, Flobo *newPuyo);
    void synchronizePuyo(Buffer<int> buffer);
    Flobo *findPuyo(int floboID);
    Flobo *puyoCells[FLOBOBAN_DIMX * (FLOBOBAN_DIMY+1)];
    FloboState nextFalling, nextCompanion;
    Flobo *fakePuyo;
    MessageBox &msgBox;
    int gameId;
    AdvancedBuffer<Flobo *> puyoVector;
    std::map<int, Flobo *> m_puyoMap;
    int semiMove;
    int neutralFlobos, sentBadPuyos;
    bool gameRunning;
    int comboPhase;
};


#endif // PUYONETWORKGAME_H
