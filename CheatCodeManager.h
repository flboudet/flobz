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

#ifndef _PUYO_CHEAT_CODE_MANAGER_H
#define _PUYO_CHEAT_CODE_MANAGER_H

#include "gameui.h"
#include "GameControls.h"

class CheatCodeManager : public gameui::Widget {
public:
    CheatCodeManager(String cheatCode, gameui::Action *cheatAction)
      : cheatCode(cheatCode), cheatAction(cheatAction), cheatCodeLength(cheatCode.length()), currentPosition(0) {}
    void eventOccured(GameControlEvent *event);
private:
    String cheatCode;
    gameui::Action *cheatAction;
    int cheatCodeLength;
    int currentPosition;
};

#endif // _PUYO_CHEAT_CODE_MANAGER_H

