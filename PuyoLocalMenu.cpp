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

const char *LocalGameMenu::getDefaultPlayerName()
{
    return PuyoGame::getDefaultPlayerName(0);
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
  PuyoGame::setDefaultPlayerName(0,playerName);
  return playerName;
}

const char *Local2PlayersGameMenu::getDefaultPlayer1Name()
{
  return PuyoGame::getDefaultPlayerName(1);
}

const char *Local2PlayersGameMenu::getDefaultPlayer2Name()
{
  return PuyoGame::getDefaultPlayerName(2);
}

Local2PlayersGameMenu::Local2PlayersGameMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), screenTitle("Choose Game Level"), editPlayer1Name("Player 1 Name:", getDefaultPlayer1Name()),
      editPlayer2Name("Player 2 Name:", getDefaultPlayer2Name()),
      easyAction(EASY, gameWidgetFactory, this), mediumAction(MEDIUM, gameWidgetFactory, this), hardAction(HARD, gameWidgetFactory, this), popAction(mainScreen),
      easy("Easy", &easyAction), medium("Medium", &mediumAction), hard("Hard", &hardAction), back("Cancel", &popAction)
{}

void Local2PlayersGameMenu::build()
{
    add(&editPlayer1Name);
    add(&editPlayer2Name);
    add(&screenTitle);
    add(&easy);
    add(&medium);
    add(&hard);
    add(&back);
}

String Local2PlayersGameMenu::getPlayer1Name() const
{
  String playerName = editPlayer1Name.getEditField()->getValue();
  PuyoGame::setDefaultPlayerName(1,playerName);
  return playerName;
}

String Local2PlayersGameMenu::getPlayer2Name() const
{
  String playerName = editPlayer2Name.getEditField()->getValue();
  PuyoGame::setDefaultPlayerName(2,playerName);
  return playerName;
}
