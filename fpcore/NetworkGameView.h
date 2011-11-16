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

#ifndef _PUYONETWORKVIEW
#define _PUYONETWORKVIEW

#include "FPNetMessageDef.h"
#include "GameView.h"

#include "NetworkDefinitions.h"

using namespace ios_fc;

class NetworkGameView : public GameView {
  public:
    NetworkGameView(FloboGameFactory *attachedFloboGameFactory, MessageBox *mbox, int gameId)
        : GameView(attachedFloboGameFactory),
          mbox(mbox), gameId(gameId), badFlobos(0), lastFullMessage(-1) {}

    NetworkGameView(FloboGameFactory *attachedFloboGameFactory,
            int playerId,
		    FloboSetTheme *attachedFloboThemeSet,
            LevelTheme *attachedLevelTheme,
		    MessageBox *mbox, int gameId)
        : GameView(attachedFloboGameFactory, playerId, attachedFloboThemeSet, attachedLevelTheme),
          mbox(mbox), gameId(gameId), badFlobos(0), lastFullMessage(-1) {}

    void cycleGame();

    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();

    // GameListener methods
    virtual void fallingsDidMoveLeft(Flobo *fallingFlobo, Flobo *companionFlobo);
	virtual void fallingsDidMoveRight(Flobo *fallingFlobo, Flobo *companionFlobo);
	virtual void fallingsDidFallingStep(Flobo *fallingFlobo, Flobo *companionFlobo);
    void gameDidAddNeutral(Flobo *neutralFlobo, int neutralIndex, int totalNeutral);
    void gameDidEndCycle();
    void companionDidTurn(Flobo *companionFlobo, Flobo *fallingFlobo, bool counterclockwise);
    void floboDidFall(Flobo *flobo, int originX, int originY, int nFalledBelow);
    void floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase);
    virtual void gameWin();
    virtual void gameLost();

    void sendStateMessage(bool sendFullMessage = false);

protected:
    void sendEndOfGameMessage(int messageType);

    MessageBox *mbox;
    int gameId;
    Message *createStateMessage(bool sendFullMessage);
    AdvancedBuffer<int> neutralsBuffer;
    AdvancedBuffer<int> moveLeftBuffer;
    AdvancedBuffer<int> moveRightBuffer;
    AdvancedBuffer<int> fallingStepBuffer;
    AdvancedBuffer<int> compTurnBuffer;
    AdvancedBuffer<int> didFallBuffer;
    AdvancedBuffer<int> willVanishBuffer;
    int badFlobos;
    double lastFullMessage;
};

class InternetGameView : public NetworkGameView {
    public:
        InternetGameView(FloboGameFactory *attachedFloboGameFactory, MessageBox *mbox, int gameId,
                                FPServerIGPMessageBox *igpbox)
          : NetworkGameView(attachedFloboGameFactory, mbox, gameId)
          , igpbox(igpbox)
        {}

        InternetGameView(FloboGameFactory *attachedFloboGameFactory,
                                int playerId,
                                FloboSetTheme *attachedFloboThemeSet,
                                LevelTheme *attachedLevelTheme,
                                MessageBox *mbox, int gameId,
                                FPServerIGPMessageBox *igpbox)
          : NetworkGameView(attachedFloboGameFactory,
                             playerId,
                             attachedFloboThemeSet,
                             attachedLevelTheme,
                             mbox, gameId),
          igpbox(igpbox) {}
        virtual void gameWin();
        virtual void gameLost();
    private:
        void sendGameResultToServer(int winner);
        FPServerIGPMessageBox *igpbox;
};

#endif // _PUYONETWORKVIEW
