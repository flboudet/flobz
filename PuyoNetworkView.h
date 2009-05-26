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

#ifndef _PUYONETWORKVIEW
#define _PUYONETWORKVIEW

#include "PuyoMessageDef.h"
#include "PuyoView.h"

#include "ios_messagebox.h"
#include "ios_igpmessagebox.h"
using namespace ios_fc;

class PuyoNetworkView : public PuyoView {
  public:
    PuyoNetworkView(PuyoGameFactory *attachedPuyoGameFactory, MessageBox *mbox, int gameId)
        : PuyoView(attachedPuyoGameFactory),
          mbox(mbox), gameId(gameId), badPuyos(0) {}

    PuyoNetworkView(PuyoGameFactory *attachedPuyoGameFactory,
		    AnimatedPuyoSetTheme *attachedPuyoThemeSet,
            PuyoLevelTheme *attachedLevelTheme,
		    int xOffset, int yOffset,
                    int nXOffset, int nYOffset, MessageBox *mbox, int gameId)
        : PuyoView(attachedPuyoGameFactory, attachedPuyoThemeSet, attachedLevelTheme,
		   xOffset, yOffset, nXOffset, nYOffset),
          mbox(mbox), gameId(gameId), badPuyos(0) {}

    void cycleGame();

    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();

    // PuyoDelegate methods
    virtual void fallingsDidMoveLeft(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
	virtual void fallingsDidMoveRight(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
	virtual void fallingsDidFallingStep(PuyoPuyo *fallingPuyo, PuyoPuyo *companionPuyo);
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, PuyoPuyo *fallingPuyo, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY, int nFalledBelow);
    void puyoWillVanish(AdvancedBuffer<PuyoPuyo *> &puyoGroup, int groupNum, int phase);
    virtual void gameWin();
    virtual void gameLost();

    void sendStateMessage(bool paused = false);

protected:
    void sendEndOfGameMessage(int messageType);

    MessageBox *mbox;
    int gameId;
    Message *createStateMessage(bool paused);
    AdvancedBuffer<int> neutralsBuffer;
    AdvancedBuffer<int> moveLeftBuffer;
    AdvancedBuffer<int> moveRightBuffer;
    AdvancedBuffer<int> fallingStepBuffer;
    AdvancedBuffer<int> compTurnBuffer;
    AdvancedBuffer<int> didFallBuffer;
    AdvancedBuffer<int> willVanishBuffer;
    int badPuyos;
};

class PuyoInternetNetworkView : public PuyoNetworkView {
    public:
        PuyoInternetNetworkView(PuyoGameFactory *attachedPuyoGameFactory, MessageBox *mbox, int gameId,
                                IgpMessageBox *igpbox)
          : PuyoNetworkView(attachedPuyoGameFactory, mbox, gameId)
          , igpbox(igpbox)
        {}

        PuyoInternetNetworkView(PuyoGameFactory *attachedPuyoGameFactory,
                                AnimatedPuyoSetTheme *attachedPuyoThemeSet,
                                PuyoLevelTheme *attachedLevelTheme,
                                int xOffset, int yOffset,
                                int nXOffset, int nYOffset, MessageBox *mbox, int gameId,
                                IgpMessageBox *igpbox)
          : PuyoNetworkView(attachedPuyoGameFactory,
                             attachedPuyoThemeSet,
                             attachedLevelTheme,
                             xOffset, yOffset,
                             nXOffset, nYOffset, mbox, gameId),
          igpbox(igpbox) {}
        virtual void gameWin();
        virtual void gameLost();
    private:
        void sendGameResultToServer(int winner);
        IgpMessageBox *igpbox;
};

#endif // _PUYONETWORKVIEW
