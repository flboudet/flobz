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

#include "PuyoNetworkMenu.h"
#include "PuyoInternetGameCenter.h"
#include "PuyoLanGameCenter.h"
#include "preferences.h"
#include "ios_socket.h"
#include "ios_standardmessage.h"
#include "PuyoNetCenterMenu.h"
#include "audio.h"

extern IIM_Surface *menuBG_wide; // I know what you think..

static const char * kInternetServerKey = "Menu.Internet.Server.";
static const char * kInternetServerPortKey = "Menu.Internet.Server.Port.";
static const char * kInternetServerNumberKey = "Menu.Internet.Server.Number.";
static const char * kInternetMetaServerKey = "Menu.Internet.MetaServer.";
static const char * kInternetMetaServerPathKey = "Menu.Internet.MetaServer.Path.";
static const char * kInternetMetaServerPortKey = "Menu.Internet.MetaServer.Port.";
static const char * kInternetMetaServerNumberKey = "Menu.Internet.MetaServer.Number.";

static const char * kInternetCurrentServerKey = "Menu.Internet.CurrentServer";
static const char * kInternetCurrentServerDefaultValue = "durandal.homeunix.com";
static const char * kInternetCurrentServerPortKey = "Menu.Internet.CurrentServer.Port";
static const char * kInternetCurrentServerPortDefaultValue = "4567";


using namespace ios_fc;


class PushLanNetCenterMenuAction : public Action
{
public:
    PushLanNetCenterMenuAction(EditField *serverPort, EditField *userName) : serverPort(serverPort), userName(userName) {}
    
    void action()
    {
        PuyoLanGameCenter *gameCenter = new PuyoLanGameCenter(atoi(serverPort->getValue()), userName->getValue());
        NetCenterMenu *newNetCenterMenu = new NetCenterMenu(gameCenter);
        newNetCenterMenu->setAutoRelease(true);
        newNetCenterMenu->build();
        (GameUIDefaults::SCREEN_STACK)->push(newNetCenterMenu);
    }
private:
    EditField *serverPort;
    EditField *userName;
};

#include "PuyoGame.h"

LANGameMenu::LANGameMenu(PuyoRealMainScreen * mainScreen)
  : PuyoMainScreenMenu(mainScreen), lanTitle("LAN Game"),
    playerNameLabel("Player name:", PuyoGame::getDefaultPlayerName(-1), PuyoGame::getDefaultPlayerKey(-1)),
    portNumLabel("Port number:", "6581"), cancelAction(mainScreen),
    startButton("Start!", new PushLanNetCenterMenuAction(portNumLabel.getEditField(), playerNameLabel.getEditField())),
    cancelButton("Cancel", &cancelAction)
{
}

void LANGameMenu::build() {
    add(&lanTitle);
    add(&startButton);
    add(&playerNameLabel);
    add(&portNumLabel);
    add(&cancelButton);
}

class PuyoHttpServerList::PuyoHttpServer {
public:
    PuyoHttpServer(String hostName, int portNum, String path) : hostName(hostName), portNum(portNum), hostPath(path) {}
    String hostName;
    String hostPath;
    int portNum;
};

PuyoHttpServerList::PuyoHttpServerList() : doc(NULL), firstTime(true)
{
  char servname[256];
  char servpath[1024];
  int nbserv;
  
  // Making a meta-server list from prefs
  fetching = 0;
  nbserv = GetIntPreference(kInternetMetaServerNumberKey,1);
  for (int i = 1; i <= nbserv; i++)
  {
    char tmp[11];
    snprintf(tmp,sizeof(tmp),"%d",i);
    GetStrPreference (String(kInternetMetaServerKey)+String(tmp), servname, i==1?"www.ios-software.com":"Error", 256);
    GetStrPreference (String(kInternetMetaServerPathKey)+String(tmp), servpath, i==1?"/flobopuyo/fpservers":"/fpservers", 1024);
    metaservers.add(
        new PuyoHttpServer(servname,
                GetIntPreference (String(kInternetMetaServerPortKey)+String(tmp), 80), String(servpath)));
    fetching++;
  }
  fetchServersInfo();

  // Making a server list from prefs
  nbserv = GetIntPreference(kInternetServerNumberKey,1);
  for (int i = 1; i <= nbserv; i++)
  {
    char tmp[11];
    snprintf(tmp,sizeof(tmp),"%d",i);
    GetStrPreference (String(kInternetServerKey)+String(tmp), servname, i==1?kInternetCurrentServerDefaultValue:"Error", 256);
    servers.add(new PuyoHttpServer(servname, GetIntPreference (String(kInternetServerPortKey)+String(tmp), atoi(kInternetCurrentServerPortDefaultValue)), String("")));
  }
}

void PuyoHttpServerList::fetchServersInfo()
{
  if (fetching > 0) {

    PuyoHttpServer * curServer = metaservers[metaservers.size()-fetching];
  
    try {
      doc = new HttpDocument(curServer->hostName, curServer->hostPath, curServer->portNum);
    } catch (Exception e) {
      //e.printMessage();
      doc = NULL;
	    fetching--;
	    if (fetching > 0) fetchServersInfo();
	    else fetching = -1;
    }
  }  
}

bool PuyoHttpServerList::listHasChanged()
{
  if (firstTime)
  {
    firstTime = false;
    return true;
  }  
	if ((fetching == 0) || (doc == NULL)) return false;

  AdvancedBuffer<PuyoHttpServer *> newservers, newmetaservers;
  
  try {
		if (!doc->documentIsReady()) return false;
		else {
      StandardMessage msg(doc->getDocumentContent());
      int nbServers = msg.getInt("NBSERV");
      for (int i = 0 ; i < nbServers ; i++) {
        char tmpStr[256];
        sprintf(tmpStr, "SERVNAME%.2d", i);
        String serverName = msg.getString(tmpStr);
        sprintf(tmpStr, "PORTNUM%.2d", i);
        int portNum = msg.getInt(tmpStr);
        newservers.add(new PuyoHttpServer(serverName, portNum, String("")));
      }
      nbServers = msg.getInt("NBMETASERV");
      for (int i = 0 ; i < nbServers ; i++) {
        char tmpStr[256];
        sprintf(tmpStr, "METANAME%.2d", i);
        String serverName = msg.getString(tmpStr);
        sprintf(tmpStr, "METAPATH%.2d", i);
        String serverPath = msg.getString(tmpStr);
        sprintf(tmpStr, "METAPORT%.2d", i);
        int portNum = msg.getInt(tmpStr);
        newmetaservers.add(new PuyoHttpServer(serverName, portNum, serverPath));
      }

    }
  } catch (Exception e) { // Erreur dans la reception du fichier
    //e.printMessage();
    for (int i = 1; i <= newservers.size(); i++) delete newservers[i-1];
    fetching--;
    if (fetching > 0) fetchServersInfo();
    else fetching = -1;
    return false;
  }
  
  // on doit mettre a jour les prefs de serveurs ici
  int nbserv;
  nbserv = newservers.size();
  if (nbserv>0)
  {
    SetIntPreference(String(kInternetServerNumberKey), nbserv);
    for (int i = 1; i <= nbserv; i++)
    {
      char tmp[11];
      snprintf(tmp,sizeof(tmp),"%d",i);
      SetStrPreference(String(kInternetServerKey)+String(tmp), newservers[i-1]->hostName);
      SetIntPreference(String(kInternetServerPortKey)+String(tmp), newservers[i-1]->portNum);
    }
  }
  nbserv = newmetaservers.size();
  if (nbserv>0)
  {
    SetIntPreference(String(kInternetMetaServerNumberKey), nbserv);
    for (int i = 1; i <= nbserv; i++)
    {
      char tmp[11];
      snprintf(tmp,sizeof(tmp),"%d",i);
      SetStrPreference(String(kInternetMetaServerKey)    +String(tmp), newmetaservers[i-1]->hostName);
      SetIntPreference(String(kInternetMetaServerPortKey)+String(tmp), newmetaservers[i-1]->portNum);
      SetStrPreference(String(kInternetMetaServerPathKey)+String(tmp), newmetaservers[i-1]->hostPath);
    }
  }
  servers = newservers;
  fetching = 0;
  return true;
}

int PuyoHttpServerList::fetchingNewData()
{
  if (fetching <0)
  {
    fetching = 0;
    return -1;
  } else return fetching;
}

String PuyoHttpServerList::getServerNameAtIndex(int index) const
{
    return servers[index]->hostName;
}

int PuyoHttpServerList::getServerPortAtIndex(int index) const
{
    return servers[index]->portNum;
}

int PuyoHttpServerList::getNumServer() const
{
    return servers.size();
}


// Actions
class ServerSelectAction : public Action {
public:
    ServerSelectAction(InternetGameMenu &igm, String serverName, int portNum)
      : gameMenu(igm), serverName(serverName), portNum(portNum) {}
    void action() {
        gameMenu.setSelectedServer(serverName, portNum);
    }
private:
    InternetGameMenu &gameMenu;
    String serverName;
    int portNum;
};

class PushNetCenterMenuAction : public Action
{
public:
    PushNetCenterMenuAction(Text *serverName, Text *serverPort, Text *userName) : serverName(serverName), serverPort(serverPort), userName(userName) {}
    
    void action()
    {
      try {
        PuyoInternetGameCenter *gameCenter = new PuyoInternetGameCenter(serverName->getValue(),
                                                                        atoi(serverPort->getValue()), userName->getValue());
        NetCenterMenu *newNetCenterMenu = new NetCenterMenu(gameCenter);
        newNetCenterMenu->setAutoRelease(true);
        newNetCenterMenu->build();
        (GameUIDefaults::SCREEN_STACK)->push(newNetCenterMenu);
      } catch (Exception e) {
        fprintf(stderr, "Error while connecting to %s\n", serverName->getValue().c_str());
        e.printMessage();
        AudioManager::playSound("ebenon.wav", 0.5);
      }
    }
private:
    Text *serverName;
    Text *serverPort;
    Text *userName;
};

void NetworkInternetAction::action()
{
  if (*menuToCreate == NULL) {
    *menuToCreate = new InternetGameMenu();
    (*menuToCreate)->build();
  }
  (GameUIDefaults::SCREEN_STACK)->push(*menuToCreate);
}

void NetworkGameMenu::build() {
  lanGameMenu.build();
  add(new Text("Network Game"));
  add(new Button("Local Area Network Game", new PuyoPushMenuAction(&lanGameMenu, mainScreen)));
  add(new Button("Internet Game", &internetAction));
  add(new Button("Cancel",        new PuyoPopMenuAction(mainScreen)));
}

InternetGameMenu::InternetGameMenu()
  : PuyoScreen(),
    servers(),
    story(666),
    container(),
    playerName(PuyoGame::getPlayerName(-2), PuyoGame::getDefaultPlayerKey(-2)),
    serverName(kInternetCurrentServerDefaultValue,kInternetCurrentServerKey),
    serverPort(kInternetCurrentServerPortDefaultValue,kInternetCurrentServerPortKey)
{
}

void InternetGameMenu::build()
{
    add(&story);
    add(&container);
    container.add(&menu);

    container.setPosition(Vec3(5,195));
    container.setSize(Vec3(menuBG_wide->w, menuBG_wide->h, 0));
    container.setBackground(menuBG_wide);
  
    serverSelectionPanel = new VBox;
    serverSelectionPanel->add(new Text("Server List"));
    serverListPanel = new ListWidget(6);
    
    serverSelectionPanel->add(serverListPanel);
    updating = new Button("Update");
    updating->mdontMove = false;
    serverSelectionPanel->add(updating);

    VBox *rightPanel = new VBox();
    rightPanel->add(new Separator(1,1));
    rightPanel->add(new Text("Internet Game"));
    rightPanel->add(new Separator(10,10));
    rightPanel->add(new Text("Nickname"));
    rightPanel->add(&playerName);
    rightPanel->add(new Separator(10,10));
    rightPanel->add(new Text("Server"));
    rightPanel->add(&serverName);
    rightPanel->add(new Text("Port"));
    rightPanel->add(&serverPort);
    rightPanel->add(new Separator(10,10));
    HBox *hbox = new HBox();
    hbox->add(new Button("Join", new PushNetCenterMenuAction(&serverName, &serverPort, &playerName)));
    hbox->add(new Button("Cancel", new PopScreenAction()));
    rightPanel->add(hbox);
    rightPanel->add(new Separator(1,1));
    menu.add(serverSelectionPanel);
    menu.add(rightPanel);
}

void InternetGameMenu::idle(double currentTime)
{
    if (servers.listHasChanged())
    {
      serverListPanel->clear();
      for (int i = 0 ; i < servers.getNumServer() ; i++) {
        serverListPanel->set (i, new Button(servers.getServerNameAtIndex(i),
            new ServerSelectAction(*this, servers.getServerNameAtIndex(i),
            servers.getServerPortAtIndex(i))));
      }
      updating->setValue("Update");
      updating->mdontMove = false;
    }

    int state = servers.fetchingNewData();
    if (state > 0)
    {
        int X = (int)(currentTime*3) % 6;
        static const char *txt[6] = {
            ".........Loading",
            "......Loading...",
            "...Loading......",
            "Loading.........",
            "...Loading......",
            "......Loading..."
        };
        updating->setValue(txt[X]);
        updating->mdontMove = true;
    }
    else if (state < 0)
    {
      updating->setValue("Update");
      updating->mdontMove = false;
    }
}

void InternetGameMenu::setSelectedServer(const String &serverName, int portNum)
{
    char sportNum[256];
    sprintf(sportNum, "%d", portNum);
    this->serverName.setValue(serverName);
    this->serverPort.setValue(sportNum);
    this->portNum = portNum;
}
