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
    : editPlayerName("Player Name:", getDefaultPlayerName()), screenTitle("Choose Game Level"), PuyoMainScreenMenu(mainScreen),
      easyAction(EASY, this), mediumAction(MEDIUM, this), hardAction(HARD, this), popAction(mainScreen),
      easy("Easy", &easyAction), medium("Medium", &mediumAction), hard("Hard", &hardAction), back("Cancel", &popAction)
{}

void LocalGameMenu::build() {
    add(&editPlayerName);
    add(&screenTitle);
    add(&easy);
    add(&medium);
    add(&hard);
    add(&back);
}

String LocalGameMenu::getPlayerName() const
{
    String playerName = editPlayerName.getEditField()->getValue();
    SetStrPreference("Player Name", playerName);
    return playerName;
}

const char *Local2PlayersGameMenu::getDefaultPlayer1Name()
{
    static char player1Name[256];
    GetStrPreference("Player1 Name", player1Name, "Player 1");
    return player1Name;
}

const char *Local2PlayersGameMenu::getDefaultPlayer2Name()
{
    static char player2Name[256];
    GetStrPreference("Player2 Name", player2Name, "Player 2");
    return player2Name;
}

Local2PlayersGameMenu::Local2PlayersGameMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), editPlayer1Name("Player 1 Name:", getDefaultPlayer1Name()),
      editPlayer2Name("Player 2 Name:", getDefaultPlayer2Name()),
      easyAction(EASY, this), mediumAction(MEDIUM, this), hardAction(HARD, this), popAction(mainScreen)
{}

void Local2PlayersGameMenu::build()
{
    add(&editPlayer1Name);
    add(&editPlayer2Name);
    add(new Text("Choose Game Level"));
    add(new Button("Easy", &easyAction));
    add(new Button("Medium", &mediumAction));
    add(new Button("Hard", &hardAction));
    add(new Button("Cancel", &popAction));
}

String Local2PlayersGameMenu::getPlayer1Name() const
{
    String playerName = editPlayer1Name.getEditField()->getValue();
    SetStrPreference("Player1 Name", playerName);
    return playerName;
}

String Local2PlayersGameMenu::getPlayer2Name() const
{
    String playerName = editPlayer2Name.getEditField()->getValue();
    SetStrPreference("Player2 Name", playerName);
    return playerName;
}
