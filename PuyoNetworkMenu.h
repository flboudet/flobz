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

#ifndef _PUYONETWORKMENU
#define _PUYONETWORKMENU

#include "gameui.h"
#include "Frame.h"
#include "FramedButton.h"
#include "FramedEditField.h"
#include "ListView.h"
#include "PuyoCommander.h"
#include "ios_httpdocument.h"


class PushNetCenterMenuAction : public Action
{
public:
    PushNetCenterMenuAction(PuyoMainScreen * mainScreen, Text *serverName, Text *serverPort, Text *userName) : mainScreen(mainScreen), serverName(serverName), serverPort(serverPort), userName(userName) {}
    virtual void action();
private:
    PuyoMainScreen * mainScreen;
    Text *serverName;
    Text *serverPort;
    Text *userName;
};

class PuyoHttpServerList {
public:
    PuyoHttpServerList();
    String getServerNameAtIndex(int index) const;
    int getServerPortAtIndex(int index) const;
    int getNumServer() const;

    void fetchServersInfo();
    bool listHasChanged();
    int fetchingNewData();

private:
  class PuyoHttpServer;
	HttpDocument *doc;
  AdvancedBuffer<PuyoHttpServer *> servers;
  AdvancedBuffer<PuyoHttpServer *> metaservers;
	int fetching;
	bool firstTime;
};

class LANGameMenu : public PuyoMainScreenMenu {
public:
    LANGameMenu(PuyoMainScreen * mainScreen);
    void build();
private:
    Frame titleFrame;
    Text lanTitle;
    VBox buttonsBox;
    EditFieldWithLabel playerNameLabel, portNumLabel;
    PuyoPopMenuAction cancelAction;
    Button startButton, cancelButton;
};

class InternetGameMenu : public PuyoMainScreenMenu, public IdleComponent {
public:
    InternetGameMenu(PuyoMainScreen * mainScreen);
    virtual ~InternetGameMenu();
    void build();
    void setSelectedServer(const String &s, int port);
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
private:
    IIM_Surface *upArrow, *downArrow;
    PuyoHttpServerList servers;
    Frame serverSelectionPanel;
    ListView serverListPanel;
    Text serverListText;
    FramedButton updating;
    Frame rightPanel;
    Separator separator1_1, separator1_2, separator1_3,  separator10_1, separator10_2;
    Text internetGameText, nicknameText, serverText, portText;
    HBox hbox, menu;
    SliderContainer container;
    FramedEditField playerName;
    FramedEditField serverName, serverPort;
    int portNum;
    PushNetCenterMenuAction pushNetCenter;
    PuyoPopMenuAction backAction;
    FramedButton joinButton, backButton;
};

class NetworkInternetAction : public Action {
public:
    NetworkInternetAction(PuyoMainScreen * mainScreen, InternetGameMenu **menuToCreate) : mainScreen(mainScreen), menuToCreate(menuToCreate) {}
    void action();
private:
    PuyoMainScreen * mainScreen;
    InternetGameMenu **menuToCreate;
};

class NetworkGameMenu : public PuyoMainScreenMenu {
public:
    NetworkGameMenu(PuyoMainScreen * mainScreen);
    void build();
private:
    PuyoLocalizedDictionary locale;
    LANGameMenu  lanGameMenu;
    InternetGameMenu *internetGameMenu;
    NetworkInternetAction internetAction;
    PuyoPushMenuAction lanAction;
    PuyoPopMenuAction mainScreenPopAction;
    Frame titleFrame;
    Text networkTitleText;
    VBox buttonsBox;
    Button lanGameButton, internetGameButton, cancelButton;
};

#endif // _PUYONETWORKMENU

