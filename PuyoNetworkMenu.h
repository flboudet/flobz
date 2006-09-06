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
#include "PuyoCommander.h"
#include "ios_httpdocument.h"

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
    LANGameMenu(PuyoRealMainScreen * mainScreen);
    void build();
private:
    Text lanTitle;
    EditFieldWithLabel playerNameLabel, portNumLabel;
    PuyoPopMenuAction cancelAction;
    Button startButton, cancelButton;
};

class InternetGameMenu : public PuyoScreen {
public:
    InternetGameMenu();
    void build();
    void setSelectedServer(const String &s, int port);
    virtual void idle(double currentTime);
    
private:
    PuyoHttpServerList servers;
    VBox *serverSelectionPanel;
    ListWidget *serverListPanel;
    HBox menu;
    PuyoStoryWidget story;
    SliderContainer container;
    EditField playerName;
    EditField serverName, serverPort;
    int portNum;
    Button *updating;
};

class NetworkInternetAction : public Action {
public:
    NetworkInternetAction(InternetGameMenu **menuToCreate) : menuToCreate(menuToCreate) {}
    void action();
private:
    InternetGameMenu **menuToCreate;
};

class NetworkGameMenu : public PuyoMainScreenMenu {
public:
    NetworkGameMenu(PuyoRealMainScreen * mainScreen);
    void build();
private:
    PuyoLocalizedDictionary locale;
    LANGameMenu  lanGameMenu;
    InternetGameMenu *internetGameMenu;
    NetworkInternetAction internetAction;
    PuyoPushMenuAction lanAction;
    PuyoPopMenuAction mainScreenPopAction;
    Text networkTitleText;
    Button lanGameButton, internetGameButton, cancelButton;
};

#endif // _PUYONETWORKMENU

