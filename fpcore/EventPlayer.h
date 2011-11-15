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

#ifndef _FLOBO_EVENT_PLAYER_H
#define _FLOBO_EVENT_PLAYER_H

#include "GamePlayer.h"

/**
 * TheEventPlayer class implements a GamePlayer controlled by
 * classic input devices events, ie SDL joystics and keyboard
 */
class EventPlayer : public GamePlayer {
public:
    EventPlayer(GameView &view, int downEvent, int leftEvent, int rightEvent, int turnLeftEvent, int turnRightEvent);
    void eventOccured(event_manager::GameControlEvent *event);
    void cycle();
private:
    bool keyShouldRepeat(int &key);
    const int downEvent, leftEvent, rightEvent, turnLeftEvent, turnRightEvent;
    int fpKey_Down, fpKey_Left, fpKey_Right, fpKey_TurnLeft, fpKey_TurnRight;
    int fpKey_Repeat, fpKey_Delay;
};

/**
 * The CombinedEventPlayer class implements a GamePlayer
 * controlled by the combination of two EventPlayers.
 * The purpose is to have a "union" of the Player 1 and Player 2
 * controllers, so that any joystick or keyboard for either
 * player 1 or player 2 can be used when playing a solo game
 */
class CombinedEventPlayer : public GamePlayer {
public:
    CombinedEventPlayer(GameView &view);
    void eventOccured(event_manager::GameControlEvent *event);
    void cycle();
private:
    EventPlayer player1controller;
    EventPlayer player2controller;
};

#endif // _FLOBO_EVENT_PLAYER_H

