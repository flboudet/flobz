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
#include "ScreenTransition.h"

class NoNameAction : public Action {
    public:
        NoNameAction(PuyoLocalizedDictionary &locale, EditField &field)
          : locale(locale), field(field)
        {}

        virtual void action() {
            String s = field.getValue();
            if (s == "") {
                s = locale.getLocalizedString("NoName");
                field.setValue(s);
            }
        }
    private:
        PuyoLocalizedDictionary locale;
        EditField &field;
};

LocalGameMenu::LocalGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      locale(theCommander->getDataPathManager(), "locale", "main"),
      editPlayerName(locale.getLocalizedString("Player:"),
		     PuyoGame::getDefaultPlayerName(0),
		     PuyoGame::getDefaultPlayerKey(0),
		     theCommander->getEditFieldFramePicture(),
		     theCommander->getEditFieldOverFramePicture(), 150),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      screenTitle(locale.getLocalizedString("Choose Game Level")),
      easyAction(mainScreen,   EASY,   this),
      mediumAction(mainScreen, MEDIUM, this),
      hardAction(mainScreen,   HARD,   this),
      popAction(mainScreen),
      easy(locale.getLocalizedString("Beginner"), &easyAction),
      medium(locale.getLocalizedString("Normal"), &mediumAction),
      hard(locale.getLocalizedString("Expert"), &hardAction),
      back(locale.getLocalizedString("Back"), &popAction)
{
    EditField &editName = editPlayerName.getEditField();
    editName.setAction(ON_START, new NoNameAction(locale, editName));
}

void LocalGameMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.add(&screenTitle);
    add(&screenTitleFrame);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    buttonsBox.add(&easy);
    buttonsBox.add(&medium);
    buttonsBox.add(&hard);
    buttonsBox.add(&editPlayerName);
    buttonsBox.add(&back);
    add(&buttonsBox);
}

String LocalGameMenu::getPlayerName() const
{
  String playerName = editPlayerName.getEditField().getValue();
  return playerName;
}

Local2PlayersGameMenu::Local2PlayersGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      locale(theCommander->getDataPathManager(), "locale", "main"),
      editPlayer1Name(locale.getLocalizedString("Player 1:"),
		      PuyoGame::getDefaultPlayerName(1),
		      PuyoGame::getDefaultPlayerKey(1),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      editPlayer2Name(locale.getLocalizedString("Player 2:"),
		      PuyoGame::getDefaultPlayerName(2),
		      PuyoGame::getDefaultPlayerKey(2),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      screenTitle(locale.getLocalizedString("Choose Game Level")),
      easyAction(EASY, gameWidgetFactory, this),
      mediumAction(MEDIUM, gameWidgetFactory, this),
      hardAction(HARD, gameWidgetFactory, this), popAction(mainScreen),
      easy(locale.getLocalizedString("Easy"), &easyAction),
      medium(locale.getLocalizedString("Medium"), &mediumAction),
      hard(locale.getLocalizedString("Hard"), &hardAction),
      back(locale.getLocalizedString("Back"), &popAction)
{}

void Local2PlayersGameMenu::build()
{
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.add(&screenTitle);
    add(&screenTitleFrame);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    buttonsBox.add(&easy);
    buttonsBox.add(&medium);
    buttonsBox.add(&hard);
    buttonsBox.add(&editPlayer1Name);
    buttonsBox.add(&editPlayer2Name);
    buttonsBox.add(&back);
    add(&buttonsBox);
}

String Local2PlayersGameMenu::getPlayer1Name() const
{
  String playerName = editPlayer1Name.getEditField().getValue();
  return playerName;
}

String Local2PlayersGameMenu::getPlayer2Name() const
{
  String playerName = editPlayer2Name.getEditField().getValue();
  return playerName;
}
