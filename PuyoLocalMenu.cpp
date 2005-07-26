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

#include "PuyoLocalMenu.h"
#include "AnimatedPuyoTheme.h"
#include "PuyoScreenTransition.h"
#include "preferences.h"

const char *LocalGameMenu::getDefaultPlayerName()
{
    static char playerName[256];

    char * defaultName = getenv("USER");
    if (defaultName == NULL)
      defaultName = "Player";
    if (!(defaultName[0]>=32))
      defaultName = "Player";
    if ((defaultName[0]>='a') && (defaultName[0]<='z'))
      defaultName[0] += 'A' - 'a';
    
    GetStrPreference("Player Name", playerName, defaultName);
    return playerName;
}

LocalGameMenu::LocalGameMenu(PuyoRealMainScreen *mainScreen)
    : editPlayerName("Player Name:", getDefaultPlayerName()), PuyoMainScreenMenu(mainScreen),
      easyAction(EASY, this), mediumAction(MEDIUM, this), hardAction(HARD, this), popAction(mainScreen)
{}

void LocalGameMenu::build() {
    add(&editPlayerName);
    add(new Text("Choose Game Level"));
    add(new Button("Easy", &easyAction));
    add(new Button("Medium", &mediumAction));
    add(new Button("Hard", &hardAction));
    add(new Button("Cancel", &popAction));
}

String LocalGameMenu::getPlayerName() const
{
    String playerName = editPlayerName.getEditField()->getValue();
    SetStrPreference("Player Name", playerName);
    return playerName;
}

void Local2PlayersGameMenu::build()
{
    add(new EditFieldWithLabel("Player Name:", "flobo"));
    add(new Text("Choose Game Level"));
    add(new Button("Easy", &easyAction));
    add(new Button("Medium", &mediumAction));
    add(new Button("Hard", &hardAction));
    add(new Button("Cancel", &popAction));
}


