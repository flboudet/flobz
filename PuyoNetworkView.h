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


#include "PuyoView.h"
#include "ios_messagebox.h"
using namespace ios_fc;

class PuyoNetworkView : public PuyoView {
  public:
    PuyoNetworkView(PuyoGameFactory *attachedPuyoGameFactory, int xOffset, int yOffset, int nXOffset, int nYOffset, MessageBox *mbox)
        :PuyoView(attachedPuyoGameFactory, xOffset, yOffset, nXOffset, nYOffset), mbox(mbox) {}
        
    void cycleGame();
    
    void moveLeft();
    void moveRight();
    void rotateLeft();
    void rotateRight();

    // PuyoDelegate methods
    void gameDidAddNeutral(PuyoPuyo *neutralPuyo, int neutralIndex);
    void gameDidEndCycle();
    void companionDidTurn(PuyoPuyo *companionPuyo, int companionVector, bool counterclockwise);
    void puyoDidFall(PuyoPuyo *puyo, int originX, int originY);
    void puyoWillVanish(IosVector &puyoGroup, int groupNum, int phase);
    void gameLost();
private:
    MessageBox *mbox;
    Message *createStateMessage(bool paused = false) const;
};

#endif // _PUYONETWORKVIEW
