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
  : GamePlayer(view), downEvent(downEvent), leftEvent(leftEvent), rightEvent(rightEvent),
    turnLeftEvent(turnLeftEvent), turnRightEvent(turnRightEvent),
    fpKey_Down(0), fpKey_Left(0), fpKey_Right(0), fpKey_TurnLeft(0), fpKey_TurnRight(0),
    fpKey_Repeat(7), fpKey_Delay(5)
{
}

void EventPlayer::eventOccured(GameControlEvent *event)
{
    int curGameEvent = event->gameEvent;
    if (event->isUp) {
        if (curGameEvent == downEvent) {
	    fpKey_Down = 0;
	}
	else if (curGameEvent == leftEvent) {
	    fpKey_Left = 0;
	}
	else if (curGameEvent == rightEvent) {
	    fpKey_Right = 0;
	}
	else if (curGameEvent == turnLeftEvent) {
	    fpKey_TurnLeft = 0;
	}
	else if (curGameEvent == turnRightEvent) {
	    fpKey_TurnRight = 0;
	}
    }
    else {
        if (curGameEvent == downEvent) {
	    fpKey_Down++;
	}
	else if (curGameEvent == leftEvent) {
	    targetView.moveLeft();
	    fpKey_Left++;
	}
	else if (curGameEvent == rightEvent) {
	    targetView.moveRight();
	    fpKey_Right++;
	}
	else if (curGameEvent == turnLeftEvent) {
	    targetView.rotateLeft();
	    fpKey_TurnLeft++;
	}
	else if (curGameEvent == turnRightEvent) {
	    targetView.rotateRight();
	    fpKey_TurnRight++;
	}
    }
}

void EventPlayer::cycle()
{
    // Key repetition
    if (fpKey_Down) {
    if (attachedGame->isEndOfCycle())
        fpKey_Down = 0;
    else
        targetView.cycleGame();
    }
    if (keyShouldRepeat(fpKey_Left))
        targetView.moveLeft();
    if (keyShouldRepeat(fpKey_Right))
        targetView.moveRight();
    if (keyShouldRepeat(fpKey_TurnLeft)) {
        if (attachedGame->isEndOfCycle())
	    fpKey_TurnLeft = 0;
	targetView.rotateLeft();
    }
    if (keyShouldRepeat(fpKey_TurnRight)) {
        if (attachedGame->isEndOfCycle())
	    fpKey_TurnRight = 0;
	targetView.rotateRight();
    }
}

bool EventPlayer::keyShouldRepeat(int &key)
{
    if (key == 0) return false;
    key++;
    return ((key - fpKey_Delay) > 0) && ((key - fpKey_Delay) % fpKey_Repeat == 0);
}


CombinedEventPlayer::CombinedEventPlayer(GameView &view)
    : GamePlayer(view),
      player1controller(view,
                        kPlayer1Down,
                        kPlayer1Left,
                        kPlayer1Right,
                        kPlayer1TurnLeft,
                        kPlayer1TurnRight),
      player2controller(view,
                        kPlayer2Down,
                        kPlayer2Left,
                        kPlayer2Right,
                        kPlayer2TurnLeft,
                        kPlayer2TurnRight)
{
}

void CombinedEventPlayer::eventOccured(GameControlEvent *event)
{
    player1controller.eventOccured(event);
    player2controller.eventOccured(event);
}

void CombinedEventPlayer::cycle()
{
    player1controller.cycle();
    player2controller.cycle();
}

