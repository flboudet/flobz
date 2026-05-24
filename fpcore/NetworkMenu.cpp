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

#include "NetworkMenu.h"
#include "LanGameCenter.h"
#include "NetCenterMenu.h"
#include "PlayerNameUtils.h"

using namespace ios_fc;


class PushLanNetCenterMenuAction : public Action
{
public:
    PushLanNetCenterMenuAction(MainScreen * mainScreen, EditField *serverPort, EditField *userName)
        : _mainScreen(mainScreen), _serverPort(serverPort), _userName(userName) {}

    void action()
    {
        LanGameCenter *gameCenter = new LanGameCenter(atoi(_serverPort->getValue().c_str()), _userName->getValue());
        NetCenterMenu *newNetCenterMenu =
	  new NetCenterMenu(_mainScreen, gameCenter,
			    theCommander->getLocalizedString("LAN Game Center"));
        newNetCenterMenu->build();
        _mainScreen->pushMenu(newNetCenterMenu, true);
    }
private:
    MainScreen * _mainScreen;
    EditField * _serverPort;
    EditField * _userName;
};


LANGameMenu::LANGameMenu(MainScreen * mainScreen)
  : MainScreenMenu(mainScreen),
    _titleFrame(theCommander->getSeparatorFramePicture()),
    _lanTitle(theCommander->getLocalizedString("LAN Game")),
    _playerNameLabel(theCommander->getLocalizedString("Player name:"),
                    PlayerNameUtils::getDefaultPlayerName(-1).c_str(), PlayerNameUtils::getDefaultPlayerKey(-1).c_str(),
                    theCommander->getPreferencesManager(),
                    theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    _portNumLabel(theCommander->getLocalizedString("Port number:"), "6581", NULL,
                 theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    _cancelAction(mainScreen),
    _startButton(theCommander->getLocalizedString("Start!"),
                new PushLanNetCenterMenuAction(mainScreen, &(_portNumLabel.getEditField()),
					       &(_playerNameLabel.getEditField()))),
    _cancelButton(theCommander->getLocalizedString("Back"), &_cancelAction)
{
}

void LANGameMenu::build() {
    setPolicy(USE_MIN_SIZE);
    _titleFrame.add(&_lanTitle);
    _titleFrame.setPreferedSize(Vec3(0, 20));
    add(&_titleFrame);
    _buttonsBox.add(&_startButton);
    _buttonsBox.add(&_playerNameLabel);
    _buttonsBox.add(&_portNumLabel);
    _buttonsBox.add(&_cancelButton);
    add(&_buttonsBox);
}

void NetworkInternetAction::action()
{
    if (*_menuToCreate == NULL) {
        *_menuToCreate = new InternetGameMenu(_mainScreen);
        (*_menuToCreate)->build();
    }
    _mainScreen->pushMenu(*_menuToCreate, true);
}


NetworkGameMenu::NetworkGameMenu(MainScreen * mainScreen)
    : MainScreenMenu(mainScreen),
      _locale(theCommander->getDataPathManager(), "locale", "main"),
      _lanGameMenu(mainScreen),
      _internetGameMenu(NULL), _internetAction(mainScreen, &_internetGameMenu),
      _lanAction(&_lanGameMenu, mainScreen), _mainScreenPopAction(mainScreen),
      _titleFrame(theCommander->getSeparatorFramePicture()),
      _networkTitleText(_locale.getLocalizedString("Network Game")),
      _lanGameButton(_locale.getLocalizedString("Local Area Network Game"), &_lanAction),
      _internetGameButton(_locale.getLocalizedString("Internet Game"), &_internetAction),
      _cancelButton(_locale.getLocalizedString("Back"), &_mainScreenPopAction)
{}

void NetworkGameMenu::build() {
  setPolicy(USE_MIN_SIZE);
  _lanGameMenu.build();
  _titleFrame.add(&_networkTitleText);
  _titleFrame.setPreferedSize(Vec3(0, 20));
  add(&_titleFrame);
  _buttonsBox.add(&_lanGameButton);
  _buttonsBox.add(&_internetGameButton);
  _buttonsBox.add(&_cancelButton);
  add(&_buttonsBox);
}
