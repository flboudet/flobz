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
        : mainScreen(mainScreen), serverPort(serverPort), userName(userName) {}

    void action()
    {
        LanGameCenter *gameCenter = new LanGameCenter(atoi(serverPort->getValue()), userName->getValue());
        NetCenterMenu *newNetCenterMenu =
	  new NetCenterMenu(mainScreen, gameCenter,
			    theCommander->getLocalizedString("LAN Game Center"));
        newNetCenterMenu->build();
        mainScreen->pushMenu(newNetCenterMenu, true);
    }
private:
    MainScreen * mainScreen;
    EditField *serverPort;
    EditField *userName;
};


LANGameMenu::LANGameMenu(MainScreen * mainScreen)
  : MainScreenMenu(mainScreen),
    titleFrame(theCommander->getSeparatorFramePicture()),
    lanTitle(theCommander->getLocalizedString("LAN Game")),
    playerNameLabel(theCommander->getLocalizedString("Player name:"),
                    PlayerNameUtils::getDefaultPlayerName(-1).c_str(), PlayerNameUtils::getDefaultPlayerKey(-1).c_str(),
                    theCommander->getPreferencesManager(),
                    theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    portNumLabel(theCommander->getLocalizedString("Port number:"), "6581", NULL,
                 theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    cancelAction(mainScreen),
    startButton(theCommander->getLocalizedString("Start!"),
                new PushLanNetCenterMenuAction(mainScreen, &(portNumLabel.getEditField()),
					       &(playerNameLabel.getEditField()))),
    cancelButton(theCommander->getLocalizedString("Back"), &cancelAction)
{
}

void LANGameMenu::build() {
    setPolicy(USE_MIN_SIZE);
    titleFrame.add(&lanTitle);
    titleFrame.setPreferedSize(Vec3(0, 20));
    add(&titleFrame);
    buttonsBox.add(&startButton);
    buttonsBox.add(&playerNameLabel);
    buttonsBox.add(&portNumLabel);
    buttonsBox.add(&cancelButton);
    add(&buttonsBox);
}

void NetworkInternetAction::action()
{
    if (*menuToCreate == NULL) {
        *menuToCreate = new InternetGameMenu(mainScreen);
        (*menuToCreate)->build();
    }
    mainScreen->pushMenu(*menuToCreate, true);
}


NetworkGameMenu::NetworkGameMenu(MainScreen * mainScreen)
    : MainScreenMenu(mainScreen),
      locale(theCommander->getDataPathManager(), "locale", "main"),
      lanGameMenu(mainScreen),
      internetGameMenu(NULL), internetAction(mainScreen, &internetGameMenu),
      lanAction(&lanGameMenu, mainScreen), mainScreenPopAction(mainScreen),
      titleFrame(theCommander->getSeparatorFramePicture()),
      networkTitleText(locale.getLocalizedString("Network Game")),
      lanGameButton(locale.getLocalizedString("Local Area Network Game"), &lanAction),
      internetGameButton(locale.getLocalizedString("Internet Game"), &internetAction),
      cancelButton(locale.getLocalizedString("Back"), &mainScreenPopAction)
{}

void NetworkGameMenu::build() {
  setPolicy(USE_MIN_SIZE);
  lanGameMenu.build();
  titleFrame.add(&networkTitleText);
  titleFrame.setPreferedSize(Vec3(0, 20));
  add(&titleFrame);
  buttonsBox.add(&lanGameButton);
  buttonsBox.add(&internetGameButton);
  buttonsBox.add(&cancelButton);
  add(&buttonsBox);
}
