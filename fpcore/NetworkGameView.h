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
          _mbox(mbox), _gameId(gameId), _badFlobos(0), _lastFullMessage(-1) {}

    NetworkGameView(FloboGameFactory *attachedFloboGameFactory,
            int playerId,
		    FloboSetTheme *attachedFloboThemeSet,
            LevelTheme *attachedLevelTheme,
		    MessageBox *mbox, int gameId)
        : GameView(attachedFloboGameFactory, playerId, attachedFloboThemeSet, attachedLevelTheme),
          _mbox(mbox), _gameId(gameId), _badFlobos(0), _lastFullMessage(-1) {}

    void cycleGame();

    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();

    // GameListener methods
    virtual void fallingsDidMoveLeft(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo);
	virtual void fallingsDidMoveRight(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo);
	virtual void fallingsDidFallingStep(std::shared_ptr<Flobo> fallingFlobo, std::shared_ptr<Flobo> companionFlobo);
    void gameDidAddNeutral(std::shared_ptr<Flobo> neutralFlobo, int neutralIndex, int totalNeutral);
    void gameDidEndCycle();
    void companionDidTurn(std::shared_ptr<Flobo> companionFlobo, std::shared_ptr<Flobo> fallingFlobo, bool counterclockwise);
    void floboDidFall(std::shared_ptr<Flobo> flobo, int originX, int originY, int nFalledBelow);
    void floboWillVanish(std::vector<std::shared_ptr<Flobo>> &floboGroup, int groupNum, int phase);
    virtual void gameWin();
    virtual void gameLost();

    void sendStateMessage(bool sendFullMessage = false);

protected:
    void sendEndOfGameMessage(int messageType);

    MessageBox *_mbox;
    int _gameId;
    Message *createStateMessage(bool sendFullMessage);
    AdvancedBuffer<int> _neutralsBuffer;
    AdvancedBuffer<int> _moveLeftBuffer;
    AdvancedBuffer<int> _moveRightBuffer;
    AdvancedBuffer<int> _fallingStepBuffer;
    AdvancedBuffer<int> _compTurnBuffer;
    AdvancedBuffer<int> _didFallBuffer;
    AdvancedBuffer<int> _willVanishBuffer;
    int _badFlobos;
    double _lastFullMessage;
};

class InternetGameView : public NetworkGameView {
    public:
        InternetGameView(FloboGameFactory *attachedFloboGameFactory, MessageBox *mbox, int gameId,
                                FPServerIGPMessageBox *igpbox)
          : NetworkGameView(attachedFloboGameFactory, mbox, gameId)
          , _igpbox(igpbox)
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
          _igpbox(igpbox) {}
        virtual void gameWin();
        virtual void gameLost();
    private:
        void sendGameResultToServer(int winner);
        FPServerIGPMessageBox *_igpbox;
};

#endif // _PUYONETWORKVIEW
