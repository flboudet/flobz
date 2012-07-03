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

#ifndef _NETWORKGAME_H_
#define _NETWORKGAME_H_

#include <map>
#include "FloboGame.h"
#include "ios_messagebox.h"

using namespace ios_fc;

class NetworkGame : public FloboGame, MessageListener {
public:
    NetworkGame(FloboFactory *attachedFactory, MessageBox *msgBox, int gameId = 0);
    ~NetworkGame();

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
    void setFloboAt(int X, int Y, Flobo *newFlobo);
    void synchronizeFlobo(Buffer<int> buffer);
    Flobo *findFlobo(int floboID);
    Flobo *floboCells[FLOBOBAN_DIMX * (FLOBOBAN_DIMY+1)];
    FloboState nextFalling, nextCompanion;
    Flobo *fakeFlobo;
    MessageBox *msgBox;
    int gameId;
    AdvancedBuffer<Flobo *> floboVector;
    std::map<int, Flobo *> m_floboMap;
    int semiMove;
    int neutralFlobos, sentBadFlobos;
    bool gameRunning;
    int comboPhase;
};


#endif // _NETWORKGAME_H_
