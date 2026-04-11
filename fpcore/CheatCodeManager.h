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

#ifndef _FLOBO_CHEAT_CODE_MANAGER_H
#define _FLOBO_CHEAT_CODE_MANAGER_H

#include <string>
#include "gameui.h"
#include "GameControls.h"

class CheatCodeManager : public gameui::Widget {
public:
    CheatCodeManager(const std::string &cheatCode, gameui::Action *cheatAction)
      : _cheatCode(cheatCode), _cheatAction(cheatAction), _cheatCodeLength(cheatCode.length()), _currentPosition(0) {}
    void eventOccured(event_manager::GameControlEvent *event);
private:
    std::string _cheatCode;
    gameui::Action *_cheatAction;
    int _cheatCodeLength;
    int _currentPosition;
};

#endif // _FLOBO_CHEAT_CODE_MANAGER_H

