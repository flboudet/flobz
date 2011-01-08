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

#include "PuyoStarter.h"
#include "PuyoView.h"
#include "PuyoNetworkView.h"
#include "PuyoNetworkGame.h"

#include "audio.h"

using namespace ios_fc;
using namespace event_manager;



TwoPlayerGameWidget::TwoPlayerGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, Action *gameOverAction)
: attachedPuyoThemeSet(puyoThemeSet),
    attachedRandom(5), attachedGameFactory(&attachedRandom),
    areaA(&attachedGameFactory, 0, &attachedPuyoThemeSet, &levelTheme),
    areaB(&attachedGameFactory, 1, &attachedPuyoThemeSet, &levelTheme),
    controllerA(areaA, kPlayer1Down, kPlayer1Left, kPlayer1Right,
            kPlayer1TurnLeft, kPlayer1TurnRight),
    controllerB(areaB, kPlayer2Down, kPlayer2Left, kPlayer2Right,
            kPlayer2TurnLeft, kPlayer2TurnRight)
{
    initWithGUI(areaA, areaB, controllerA, controllerB, levelTheme, gameOverAction);
}


