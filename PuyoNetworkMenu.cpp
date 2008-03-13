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
    PushLanNetCenterMenuAction(PuyoMainScreen * mainScreen, EditField *serverPort, EditField *userName)
        : mainScreen(mainScreen), serverPort(serverPort), userName(userName) {}
    
    void action()
    {
        PuyoLanGameCenter *gameCenter = new PuyoLanGameCenter(atoi(serverPort->getValue()), userName->getValue());
        NetCenterMenu *newNetCenterMenu = new NetCenterMenu(mainScreen, gameCenter);
        newNetCenterMenu->build();
        mainScreen->pushMenu(newNetCenterMenu, true);
    }
private:
    PuyoMainScreen * mainScreen;
    EditField *serverPort;
    EditField *userName;
};

#include "PuyoGame.h"

LANGameMenu::LANGameMenu(PuyoMainScreen * mainScreen)
  : PuyoMainScreenMenu(mainScreen),
    titleFrame(theCommander->getSeparatorFramePicture()),
    lanTitle(theCommander->getLocalizedString("LAN Game")),
    playerNameLabel(theCommander->getLocalizedString("Player name:"),
                    PuyoGame::getDefaultPlayerName(-1), PuyoGame::getDefaultPlayerKey(-1),
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


/*void PushNetCenterMenuAction::action()
{
    try {
        PuyoInternetGameCenter *gameCenter = new PuyoInternetGameCenter(serverName->getValue(),
                                                                        atoi(serverPort->getValue()), userName->getValue());
        NetCenterMenu *newNetCenterMenu = new NetCenterMenu(mainScreen, gameCenter);
        newNetCenterMenu->build();
        mainScreen->pushMenu(newNetCenterMenu, true);
    } catch (Exception e) {
        fprintf(stderr, "Error while connecting to %s\n", serverName->getValue().c_str());
        e.printMessage();
        AudioManager::playSound("ebenon.wav", 0.5);
    }
}*/

void NetworkInternetAction::action()
{
    if (*menuToCreate == NULL) {
        *menuToCreate = new InternetGameMenu(mainScreen);
        (*menuToCreate)->build();
    }
    mainScreen->pushMenu(*menuToCreate, true);
}

NetworkGameMenu::NetworkGameMenu(PuyoMainScreen * mainScreen)
    : locale(theCommander->getDataPathManager(), "locale", "main"),
      PuyoMainScreenMenu(mainScreen), lanGameMenu(mainScreen),
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

InternetGameMenu::InternetGameMenu(PuyoMainScreen * mainScreen)
  : PuyoMainScreenMenu(mainScreen),
    upArrow(IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/uparrow.png"))),
    downArrow(IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/downarrow.png"))),
    serverSelectionPanel(theCommander->getWindowFramePicture()),
    servers(),
    container(),
    serverListPanel(20, upArrow, downArrow),
    serverListText("Server List"), rightPanel(theCommander->getWindowFramePicture()),
    updating("Update", NULL,
	     theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    separator1_1(1,1), separator1_2(1,1), separator1_3(1, 1), separator10_1(10,10), separator10_2(10,10),
    internetGameText("Internet Game"), nicknameText("Nickname"), serverText("Server"), portText("Port"),
    playerName(PuyoGame::getPlayerName(-2), PuyoGame::getDefaultPlayerKey(-2),
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    serverName(kInternetCurrentServerDefaultValue,kInternetCurrentServerKey,
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    serverPort(kInternetCurrentServerPortDefaultValue,kInternetCurrentServerPortKey,
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    /*pushNetCenter(mainScreen, &(serverName.getEditField()), &(serverPort.getEditField()),
		  &(playerName.getEditField())),*/ backAction(mainScreen),
    joinButton("Join", this,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    backButton("Back", &backAction,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture())
{
    this->setBorderVisible(false);
}

InternetGameMenu::~InternetGameMenu()
{
    IIM_Free(upArrow);
    IIM_Free(downArrow);
}

void InternetGameMenu::build()
{
    add(&container);
    container.add(&menu);

    container.setPosition(Vec3(5,195));
    //container.setSize(Vec3(menuBG_wide->w, menuBG_wide->h, 0));
  
    serverSelectionPanel.setPolicy(USE_MIN_SIZE);
    serverSelectionPanel.add(&serverListText);
    serverSelectionPanel.add(&serverListPanel);
    serverSelectionPanel.add(&updating);

    rightPanel.add(&separator1_1);
    rightPanel.add(&internetGameText);
    rightPanel.add(&separator1_2);
    rightPanel.add(&nicknameText);
    rightPanel.add(&playerName);
    rightPanel.add(&separator10_1);
    rightPanel.add(&serverText);
    rightPanel.add(&serverName);
    rightPanel.add(&portText);
    rightPanel.add(&serverPort);
    rightPanel.add(&separator10_2);
    hbox.add(&joinButton);
    hbox.add(&backButton);
    rightPanel.add(&hbox);
    rightPanel.add(&separator1_3);
    menu.add(&serverSelectionPanel);
    menu.add(&rightPanel);
}

class PuyoInternetDialog : public SliderContainer, public SliderContainerListener
{
public:
    PuyoInternetDialog(String dialogTitle);
    virtual ~PuyoInternetDialog();
    virtual void onSlideInside(SliderContainer &slider);
    void close();
protected:
    VBox m_contentBox;
private:
    Frame m_dialogFrame;
    Frame m_titleFrame;
    Text m_titleText;
    bool m_closing;
};

PuyoInternetDialog::PuyoInternetDialog(String dialogTitle)
  : SliderContainer(), m_dialogFrame(theCommander->getWindowFramePicture()),
    m_titleFrame(theCommander->getSeparatorFramePicture()),
    m_titleText(dialogTitle), m_closing(false)
{
    m_titleFrame.add(&m_titleText);
    m_titleFrame.setPreferedSize(Vec3(0, 20));
    m_dialogFrame.add(&m_titleFrame);
    m_dialogFrame.add(&m_contentBox);
    setPreferedSize(Vec3(300, 200));
    setPosition(Vec3(150, 150));
    this->addListener(*this);
    transitionToContent(&m_dialogFrame);
}

PuyoInternetDialog::~PuyoInternetDialog()
{
    //this->getParentScreen()->ungrabEventsOnWidget(this);
}

void PuyoInternetDialog::onSlideInside(SliderContainer &slider)
{
    if (!m_closing)
        return;
    this->getParentScreen()->remove(this);
    delete this;
}

void PuyoInternetDialog::close()
{
    m_closing = true;
    transitionToContent(NULL);
}

class PuyoInternetErrorDialog : public PuyoInternetDialog, public Action
{
public:
    PuyoInternetErrorDialog(String errorMessageL1, String errorMessageL2);
    virtual ~PuyoInternetErrorDialog();
    virtual void action(Widget *sender, GameUIEnum actionType, GameControlEvent *event);
private:
    VBox m_textBox;
    HBox m_view;
    Text m_errorMessageL1, m_errorMessageL2;
    FramedButton m_okButton;
    IIM_Surface *m_errorIconImage;
    Image m_errorIcon;
};

PuyoInternetErrorDialog::PuyoInternetErrorDialog(String errorMessageL1, String errorMessageL2)
  : PuyoInternetDialog("Error"), m_errorMessageL1(errorMessageL1),
    m_errorMessageL2(errorMessageL2), m_okButton("OK", this,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    m_errorIconImage(IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/errorpuyo.png"))),
    m_errorIcon(m_errorIconImage)
{
    m_view.setPolicy(USE_MIN_SIZE);
    m_textBox.add(&m_errorMessageL1);
    m_textBox.add(&m_errorMessageL2);
    m_view.add(&m_errorIcon);
    m_view.add(&m_textBox);
    m_contentBox.add(&m_view);
    m_contentBox.add(&m_okButton);
}

PuyoInternetErrorDialog::~PuyoInternetErrorDialog()
{
    IIM_Free(m_errorIconImage);
    
}

void PuyoInternetErrorDialog::action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
{
    if (sender == m_okButton.getButton()) {
        close();
    }
}

class PuyoInternetConnectDialog : public PuyoInternetDialog, public Action
{
public:
    PuyoInternetConnectDialog(String serverName, PuyoInternetGameCenter *gameCenter, InternetGameMenu *owner);
    virtual ~PuyoInternetConnectDialog();
    virtual void idle(double currentTime);
private:
    Text m_messageL1, m_messageL2;
    PuyoInternetGameCenter *m_gameCenter;
    InternetGameMenu *m_owner;
};

PuyoInternetConnectDialog::PuyoInternetConnectDialog(String serverName, PuyoInternetGameCenter *gameCenter, InternetGameMenu *owner)
    : PuyoInternetDialog("Connecting"),
      m_messageL1("Connecting to server"), m_messageL2(serverName),
      m_gameCenter(gameCenter), m_owner(owner)
{
    m_contentBox.add(&m_messageL1);
    m_contentBox.add(&m_messageL2);
}

PuyoInternetConnectDialog::~PuyoInternetConnectDialog()
{
    m_owner->enterNetCenterMenu(m_gameCenter);
}

void PuyoInternetConnectDialog::idle(double currentTime)
{
    m_gameCenter->idle();
    if (m_gameCenter->isConnected())
        close();
    PuyoInternetDialog::idle(currentTime);
}

void InternetGameMenu::enterNetCenterMenu(PuyoInternetGameCenter *gameCenter)
{
    NetCenterMenu *newNetCenterMenu = new NetCenterMenu(mainScreen, gameCenter);
    newNetCenterMenu->build();
    mainScreen->pushMenu(newNetCenterMenu, true);
}

void InternetGameMenu::action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
{
    try {
        PuyoInternetGameCenter *gameCenter = new PuyoInternetGameCenter(serverName.getEditField().getValue(),
                                                                        atoi(serverPort.getEditField().getValue()), playerName.getEditField().getValue());
        PuyoInternetConnectDialog *connectionDialog = new PuyoInternetConnectDialog(serverName.getEditField().getValue(), gameCenter, this);
        this->getParentScreen()->add(connectionDialog);
        this->getParentScreen()->grabEventsOnWidget(connectionDialog);
    } catch (Exception e) {
        fprintf(stderr, "Error while connecting to %s\n", serverName.getEditField().getValue().c_str());
        e.printMessage();
        PuyoInternetErrorDialog *errorDialog = new PuyoInternetErrorDialog("Cannot connect to", serverName.getEditField().getValue());
        this->getParentScreen()->add(errorDialog);
        this->getParentScreen()->grabEventsOnWidget(errorDialog);
        AudioManager::playSound("ebenon.wav", 0.5);
    }
}

void InternetGameMenu::idle(double currentTime)
{
    if (servers.listHasChanged())
    {
      while (serverListPanel.getFullSize() > 0) {
        ListViewEntry *entry = serverListPanel.getEntryAt(0);
        serverListPanel.removeEntry(entry);
        delete entry;
      }
      for (int i = 0 ; i < servers.getNumServer() ; i++) {
        serverListPanel.addEntry(new ListViewEntry(servers.getServerNameAtIndex(i),
            new ServerSelectAction(*this, servers.getServerNameAtIndex(i),
            servers.getServerPortAtIndex(i))));
      }
      updating.setValue("Update");
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
        updating.setValue(txt[X]);
    }
    else if (state < 0)
    {
      updating.setValue("Update");
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
