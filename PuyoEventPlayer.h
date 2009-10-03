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

#ifndef _PUYO_EVENT_PLAYER_H
#define _PUYO_EVENT_PLAYER_H

#include "PuyoPlayer.h"

/**
 * ThePuyoEventPlayer class implements a PuyoPlayer controlled by
 * classic input devices events, ie SDL joystics and keyboard
 */
class PuyoEventPlayer : public PuyoPlayer {
public:
    PuyoEventPlayer(PuyoView &view, int downEvent, int leftEvent, int rightEvent, int turnLeftEvent, int turnRightEvent);
    void eventOccured(event_manager::GameControlEvent *event);
    void cycle();
private:
    bool keyShouldRepeat(int &key);
    const int downEvent, leftEvent, rightEvent, turnLeftEvent, turnRightEvent;
    int fpKey_Down, fpKey_Left, fpKey_Right, fpKey_TurnLeft, fpKey_TurnRight;
    int fpKey_Repeat, fpKey_Delay;
};

/**
 * The PuyoCombinedEventPlayer class implements a PuyoPlayer
 * controlled by the combination of two PuyoEventPlayers.
 * The purpose is to have a "union" of the Player 1 and Player 2
 * controllers, so that any joystick or keyboard for either
 * player 1 or player 2 can be used when playing a solo game
 */
class PuyoCombinedEventPlayer : public PuyoPlayer {
public:
    PuyoCombinedEventPlayer(PuyoView &view);
    void eventOccured(event_manager::GameControlEvent *event);
    void cycle();
private:
    PuyoEventPlayer player1controller;
    PuyoEventPlayer player2controller;
};

#endif // _PUYO_EVENT_PLAYER_H

