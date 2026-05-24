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

#include "LocalMenu.h"
#include "ScreenTransition.h"
#include "PlayerNameUtils.h"

class NoNameAction : public Action {
    public:
        NoNameAction(LocalizedDictionary &locale, EditField &field)
          : _locale(locale), _field(field)
        {}


    virtual void action() {
            std::string s = _field.getValue();
            if (s == "") {
                s = _locale.getLocalizedString("NoName");
                _field.setValue(s);
            }
        }
    private:
        LocalizedDictionary _locale;
        EditField &_field;
};

SoloGameMenu::SoloGameMenu(MainScreen *mainScreen)
  : MainScreenMenu(mainScreen)
{
    _editPlayerName.reset(new EditFieldWithLabel(
                               theCommander->getLocalizedString("Player:"),
                               PlayerNameUtils::getDefaultPlayerName(0).c_str(),
                               PlayerNameUtils::getDefaultPlayerKey(0).c_str(),
                               theCommander->getPreferencesManager(),
                               theCommander->getEditFieldFramePicture(),
                               theCommander->getEditFieldOverFramePicture(), 150));
    _screenTitleFrame.reset(new Frame(theCommander->getSeparatorFramePicture()));
    _screenTitle.reset(new Text(theCommander->getLocalizedString("Choose Game Level")));
    _easyAction.reset(new SoloModeStarterAction(EASY, this));
    _mediumAction.reset(new SoloModeStarterAction(MEDIUM, this));
    _hardAction.reset(new SoloModeStarterAction(HARD,   this));
    _popAction.reset(new PopMainScreenMenuAction(mainScreen));
    _easy.reset(new   Button(theCommander->getLocalizedString("Beginner"), _easyAction.get()));
    _medium.reset(new Button(theCommander->getLocalizedString("Normal"), _mediumAction.get()));
    _hard.reset(new   Button(theCommander->getLocalizedString("Expert"), _hardAction.get()));
    _back.reset(new   Button(theCommander->getLocalizedString("Back"), _popAction.get()));

    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame->add(_screenTitle.get());
    add(_screenTitleFrame.get());
    _screenTitleFrame->setPreferedSize(Vec3(0, 20));
    _buttonsBox.add(_easy.get());
    _buttonsBox.add(_medium.get());
    _buttonsBox.add(_hard.get());
    _buttonsBox.add(_editPlayerName.get());
    _buttonsBox.add(_back.get());
    add(&_buttonsBox);
}

std::string SoloGameMenu::getPlayerName(int playerNumber) const
{
    return _editPlayerName->getEditField().getValue();
}

LocalGameMenu::LocalGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      _locale(theCommander->getDataPathManager(), "locale", "main"),
      _editPlayerName(_locale.getLocalizedString("Player:"),
		     PlayerNameUtils::getDefaultPlayerName(0).c_str(),
		     PlayerNameUtils::getDefaultPlayerKey(0).c_str(),
             theCommander->getPreferencesManager(),
		     theCommander->getEditFieldFramePicture(),
		     theCommander->getEditFieldOverFramePicture(), 150),
      _screenTitleFrame(theCommander->getSeparatorFramePicture()),
      _screenTitle(_locale.getLocalizedString("Choose Game Level")),
      _easyAction(EASY, this),
      _mediumAction(MEDIUM, this),
      _hardAction(HARD,   this),
      _popAction(mainScreen),
      _easy(_locale.getLocalizedString("Beginner"), &_easyAction),
      _medium(_locale.getLocalizedString("Normal"), &_mediumAction),
      _hard(_locale.getLocalizedString("Expert"), &_hardAction),
      _back(_locale.getLocalizedString("Back"), &_popAction)
{
    EditField &editName = _editPlayerName.getEditField();
    editName.setAction(ON_ACTION, new NoNameAction(_locale, editName));
}

void LocalGameMenu::build() {
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.add(&_screenTitle);
    add(&_screenTitleFrame);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _buttonsBox.add(&_easy);
    _buttonsBox.add(&_medium);
    _buttonsBox.add(&_hard);
    _buttonsBox.add(&_editPlayerName);
    _buttonsBox.add(&_back);
    add(&_buttonsBox);
}

std::string LocalGameMenu::getPlayerName(int playerNumber) const
{
  return _editPlayerName.getEditField().getValue();
}

Local2PlayersGameMenu::Local2PlayersGameMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      _locale(theCommander->getDataPathManager(), "locale", "main"),
      _editPlayer1Name(_locale.getLocalizedString("Player 1:"),
		      PlayerNameUtils::getDefaultPlayerName(1).c_str(),
		      PlayerNameUtils::getDefaultPlayerKey(1).c_str(),
              theCommander->getPreferencesManager(),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      _editPlayer2Name(_locale.getLocalizedString("Player 2:"),
		      PlayerNameUtils::getDefaultPlayerName(2).c_str(),
		      PlayerNameUtils::getDefaultPlayerKey(2).c_str(),
              theCommander->getPreferencesManager(),
		      theCommander->getEditFieldFramePicture(),
		      theCommander->getEditFieldOverFramePicture(), 150),
      _screenTitleFrame(theCommander->getSeparatorFramePicture()),
      _screenTitle(_locale.getLocalizedString("Choose Game Level")),
      _easyAction(EASY, &_gameWidgetFactory, this, 3),
      _mediumAction(MEDIUM, &_gameWidgetFactory, this),
      _hardAction(HARD, &_gameWidgetFactory, this), _popAction(mainScreen),
      _easy(_locale.getLocalizedString("Easy"), &_easyAction),
      _medium(_locale.getLocalizedString("Medium"), &_mediumAction),
      _hard(_locale.getLocalizedString("Hard"), &_hardAction),
      _back(_locale.getLocalizedString("Back"), &_popAction)
{}

void Local2PlayersGameMenu::build()
{
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.add(&_screenTitle);
    add(&_screenTitleFrame);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _buttonsBox.add(&_easy);
    _buttonsBox.add(&_medium);
    _buttonsBox.add(&_hard);
    _buttonsBox.add(&_editPlayer1Name);
    _buttonsBox.add(&_editPlayer2Name);
    _buttonsBox.add(&_back);
    add(&_buttonsBox);
}

std::string Local2PlayersGameMenu::getPlayerName(int playerNumber) const
{
  switch (playerNumber) {
  case 0:  {
    return _editPlayer1Name.getEditField().getValue();
  }
  case 1:
  default: {
    return _editPlayer2Name.getEditField().getValue();
  }
  }
}
