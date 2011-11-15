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

#ifndef PUYOPLAYER_H
#define PUYOPLAYER_H

#include "GameView.h"
#include "FloboGame.h"
#include "GameControls.h"

class GamePlayer {
public:
    GamePlayer(GameView &view) : targetView(view), attachedGame(view.getAttachedGame()) {}
    virtual void eventOccured(event_manager::GameControlEvent *event) {}
    virtual void cycle() {}
    virtual ~GamePlayer() {}
protected:
    GameView &targetView;
    FloboGame *attachedGame;
};

class GameNullPlayer : public GamePlayer {
public:
    GameNullPlayer(GameView &view) : GamePlayer(view) {}
};

#endif
