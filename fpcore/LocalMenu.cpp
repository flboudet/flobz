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

#include "LocalMenu.h"
#include "ScreenTransition.h"
#include "PlayerNameUtils.h"

class NoNameAction : public Action {
    public:
        NoNameAction(LocalizedDictionary &locale, EditField &field)
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
        LocalizedDictionary locale;
        EditField &field;
};

SoloGameMenu::SoloGameMenu(MainScreen *mainScreen)
  : MainScreenMenu(mainScreen)
{
    m_editPlayerName.reset(new EditFieldWithLabel(
                               theCommander->getLocalizedString("Player:"),
                               PlayerNameUtils::getDefaultPlayerName(0).c_str(),
                               PlayerNameUtils::getDefaultPlayerKey(0).c_str(),
                               theCommander->getPreferencesManager(),
                               theCommander->getEditFieldFramePicture(),
                               theCommander->getEditFieldOverFramePicture(), 150));
    m_screenTitleFrame.reset(new Frame(theCommander->getSeparatorFramePicture()));
    m_screenTitle.reset(new Text(theCommander->getLocalizedString("Choose Game Level")));
    m_easyAction.reset(new SoloModeStarterAction(EASY, this));
    m_mediumAction.reset(new SoloModeStarterAction(MEDIUM, this));
    m_hardAction.reset(new SoloModeStarterAction(HARD,   this));
    m_popAction.reset(new PopMainScreenMenuAction(mainScreen));
    m_easy.reset(new   Button(theCommander->getLocalizedString("Beginner"), m_easyAction.get()));
    m_medium.reset(new Button(theCommander->getLocalizedString("Normal"), m_mediumAction.get()));
    m_hard.reset(new   Button(theCommander->getLocalizedString("Expert"), m_hardAction.get()));
    m_back.reset(new   Button(theCommander->getLocalizedString("Back"), m_popAction.get()));

    setPolicy(USE_MIN_SIZE);
    m_screenTitleFrame->add(m_screenTitle.get());
    add(m_screenTitleFrame.get());
    m_screenTitleFrame->setPreferedSize(Vec3(0, 20));
    m_buttonsBox.add(m_easy.get());
    m_buttonsBox.add(m_medium.get());
    m_buttonsBox.add(m_hard.get());
    m_buttonsBox.add(m_editPlayerName.get());
    m_buttonsBox.add(m_back.get());
    add(&m_buttonsBox);
}

String SoloGameMenu::getPlayerName(int playerNumber) const
{
    String playerName = "toto";//editPlayerName.getEditField().getValue();
    return playerName;
}

LocalGameMenu::LocalGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      locale(theCommander->getDataPathManager(), "locale", "main"),
      editPlayerName(locale.getLocalizedString("Player:"),
		     PlayerNameUtils::getDefaultPlayerName(0).c_str(),
		     PlayerNameUtils::getDefaultPlayerKey(0).c_str(),
             theCommander->getPreferencesManager(),
		     theCommander->getEditFieldFramePicture(),
		     theCommander->getEditFieldOverFramePicture(), 150),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      screenTitle(locale.getLocalizedString("Choose Game Level")),
      easyAction(EASY, this),
      mediumAction(MEDIUM, this),
      hardAction(HARD,   this),
      popAction(mainScreen),
      easy(locale.getLocalizedString("Beginner"), &easyAction),
      medium(locale.getLocalizedString("Normal"), &mediumAction),
      hard(locale.getLocalizedString("Expert"), &hardAction),
      back(locale.getLocalizedString("Back"), &popAction)
{
    EditField &editName = editPlayerName.getEditField();
    editName.setAction(ON_ACTION, new NoNameAction(locale, editName));
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

String LocalGameMenu::getPlayerName(int playerNumber) const
{
  String playerName = editPlayerName.getEditField().getValue();
  return playerName;
}

Local2PlayersGameMenu::Local2PlayersGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      locale(theCommander->getDataPathManager(), "locale", "main"),
      editPlayer1Name(locale.getLocalizedString("Player 1:"),
		      PlayerNameUtils::getDefaultPlayerName(1).c_str(),
		      PlayerNameUtils::getDefaultPlayerKey(1).c_str(),
              theCommander->getPreferencesManager(),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      editPlayer2Name(locale.getLocalizedString("Player 2:"),
		      PlayerNameUtils::getDefaultPlayerName(2).c_str(),
		      PlayerNameUtils::getDefaultPlayerKey(2).c_str(),
              theCommander->getPreferencesManager(),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      screenTitle(locale.getLocalizedString("Choose Game Level")),
      easyAction(EASY, &gameWidgetFactory, this),
      mediumAction(MEDIUM, &gameWidgetFactory, this),
      hardAction(HARD, &gameWidgetFactory, this), popAction(mainScreen),
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

String Local2PlayersGameMenu::getPlayerName(int playerNumber) const
{
  switch (playerNumber) {
  case 0:  {
    String player0Name = editPlayer1Name.getEditField().getValue();
    return player0Name;
  }
  case 1:
  default: {
    String player1Name = editPlayer2Name.getEditField().getValue();
    return player1Name;
  }
  }
}
