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

#include "EventPlayer.h"

using namespace event_manager;

EventPlayer::EventPlayer(GameView &view,
						     int downEvent, int leftEvent, int rightEvent,
						     int turnLeftEvent, int turnRightEvent)
  : GamePlayer(view), _downEvent(downEvent), _leftEvent(leftEvent), _rightEvent(rightEvent),
    _turnLeftEvent(turnLeftEvent), _turnRightEvent(turnRightEvent),
    _fpKey_Down(0), _fpKey_Left(0), _fpKey_Right(0), _fpKey_TurnLeft(0), _fpKey_TurnRight(0),
    _fpKey_Repeat(7), _fpKey_Delay(5)
{
}

void EventPlayer::eventOccured(GameControlEvent *event)
{
    int curGameEvent = event->gameEvent;
    if (event->isUp) {
        if (curGameEvent == _downEvent) {
	    _fpKey_Down = 0;
	}
	else if (curGameEvent == _leftEvent) {
	    _fpKey_Left = 0;
	}
	else if (curGameEvent == _rightEvent) {
	    _fpKey_Right = 0;
	}
	else if (curGameEvent == _turnLeftEvent) {
	    _fpKey_TurnLeft = 0;
	}
	else if (curGameEvent == _turnRightEvent) {
	    _fpKey_TurnRight = 0;
	}
    }
    else {
        if (curGameEvent == _downEvent) {
	    _fpKey_Down++;
	}
	else if (curGameEvent == _leftEvent) {
	    _targetView.moveLeft();
	    _fpKey_Left++;
	}
	else if (curGameEvent == _rightEvent) {
	    _targetView.moveRight();
	    _fpKey_Right++;
	}
	else if (curGameEvent == _turnLeftEvent) {
	    _targetView.rotateLeft();
	    _fpKey_TurnLeft++;
	}
	else if (curGameEvent == _turnRightEvent) {
	    _targetView.rotateRight();
	    _fpKey_TurnRight++;
	}
    }
}

void EventPlayer::cycle()
{
    // Key repetition
    if (_fpKey_Down) {
    if (_attachedGame->isEndOfCycle())
        _fpKey_Down = 0;
    else
        _targetView.cycleGame();
    }
    if (keyShouldRepeat(_fpKey_Left))
        _targetView.moveLeft();
    if (keyShouldRepeat(_fpKey_Right))
        _targetView.moveRight();
    if (keyShouldRepeat(_fpKey_TurnLeft)) {
        if (_attachedGame->isEndOfCycle())
	    _fpKey_TurnLeft = 0;
	_targetView.rotateLeft();
    }
    if (keyShouldRepeat(_fpKey_TurnRight)) {
        if (_attachedGame->isEndOfCycle())
	    _fpKey_TurnRight = 0;
	_targetView.rotateRight();
    }
}

bool EventPlayer::keyShouldRepeat(int &key)
{
    if (key == 0) return false;
    key++;
    return ((key - _fpKey_Delay) > 0) && ((key - _fpKey_Delay) % _fpKey_Repeat == 0);
}


CombinedEventPlayer::CombinedEventPlayer(GameView &view)
    : GamePlayer(view),
      _player1controller(view,
                        kPlayer1Down,
                        kPlayer1Left,
                        kPlayer1Right,
                        kPlayer1TurnLeft,
                        kPlayer1TurnRight),
      _player2controller(view,
                        kPlayer2Down,
                        kPlayer2Left,
                        kPlayer2Right,
                        kPlayer2TurnLeft,
                        kPlayer2TurnRight)
{
}

void CombinedEventPlayer::eventOccured(GameControlEvent *event)
{
    _player1controller.eventOccured(event);
    _player2controller.eventOccured(event);
}

void CombinedEventPlayer::cycle()
{
    _player1controller.cycle();
    _player2controller.cycle();
}

