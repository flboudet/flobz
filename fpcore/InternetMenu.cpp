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

#include "ios_standardmessage.h"
#include "InternetMenu.h"
#include "preferences.h"
#include "InternetGameCenter.h"
#include "NetCenterMenu.h"
#include "audio.h"

using namespace std;
using namespace event_manager;

static const char * kInternetMetaServerKey = "Menu.Internet.MetaServer.";
static const char * kInternetMetaServerPathKey = "Menu.Internet.MetaServer.Path.";
static const char * kInternetMetaServerPortKey = "Menu.Internet.MetaServer.Port.";
static const char * kInternetMetaServerNumberKey = "Menu.Internet.MetaServer.Number.";

static const char * kInternetPasswordKey = "Menu.Internet.Password";
static const char * kInternetCurrentServerKey = "Menu.Internet.CurrentServer";
static const char * kInternetCurrentServerDefaultValue = "aley.fovea.cc";
static const char * kInternetCurrentServerPortKey = "Menu.Internet.CurrentServer.Port";
static const char * kInternetCurrentServerPortDefaultValue = "4567";

AbstractPuyoMetaServerConnection::AbstractPuyoMetaServerConnection(PuyoMetaServerConnectionResponder *responder)
  : m_responder(responder)
{}

void AbstractPuyoMetaServerConnection::onServerListHasChanged()
{
    m_responder->PuyoMetaServerListHasChanged(*this);
}

DummyMetaServerConnection::DummyMetaServerConnection(PuyoMetaServerConnectionResponder *responder)
  : AbstractPuyoMetaServerConnection(responder)
{
}

void DummyMetaServerConnection::fetch()
{
    onServerListHasChanged();
}

vector<PuyoServer> DummyMetaServerConnection::getServers() const
{
    vector<PuyoServer> result;
    result.push_back(PuyoServer("immunity.local", 4567, "immunity"));
    result.push_back(PuyoServer("durandal.local", 4567, "durandal"));
    result.push_back(PuyoServer("gfive.local", 4567, "gfive"));
    return result;
}

HttpMetaServerConnection::HttpMetaServerConnection(String hostName, String hostPath, int portNum, PuyoMetaServerConnectionResponder *responder)
  : AbstractPuyoMetaServerConnection(responder), m_doc(NULL),
    m_hostName(hostName), m_hostPath(hostPath), m_portNum(portNum), m_nErrors(0)
{
    GameUIDefaults::GAME_LOOP->addIdle(this);
}

HttpMetaServerConnection::~HttpMetaServerConnection()
{
    GameUIDefaults::GAME_LOOP->removeIdle(this);
    if (m_doc != NULL)
        delete m_doc;
}

void HttpMetaServerConnection::fetch()
{
    if (m_nErrors > 5) {
        GameUIDefaults::GAME_LOOP->removeIdle(this);
        return;
    }
    try {
        m_servers.clear();
        m_doc = new HttpDocument(m_hostName, m_hostPath, m_portNum);
    } catch (Exception e) {
        e.printMessage();
        m_nErrors += 1;
        m_doc = NULL;
    }
}

std::vector<PuyoServer> HttpMetaServerConnection::getServers() const
{
    return m_servers;
}

void HttpMetaServerConnection::idle(double currentTime)
{
    if (m_nErrors > 5) {
        GameUIDefaults::GAME_LOOP->removeIdle(this);
        m_servers.push_back(PuyoServer("aley.fovea.cc", 4567, String("")));
        m_servers.push_back(PuyoServer("localhost", 4567, String("")));
        onServerListHasChanged();
        return;
    }
    if (m_doc == NULL) return;
    try {
		if (m_doc->documentIsReady()) {
            StandardMessage msg(m_doc->getDocumentContent());
            int nbServers = msg.getInt("NBSERV");
            for (int i = 0 ; i < nbServers ; i++) {
                char tmpStr[256];
                sprintf(tmpStr, "SERVNAME%.2d", i);
                String serverName = msg.getString(tmpStr);
                sprintf(tmpStr, "PORTNUM%.2d", i);
                int portNum = msg.getInt(tmpStr);
                m_servers.push_back(PuyoServer(serverName, portNum, String("")));
            }
            delete m_doc;
            m_doc = NULL;
            onServerListHasChanged();
        }
    } catch (Exception e) { // Erreur dans la reception du fichier
        e.printMessage();
        m_nErrors += 1;
    }
}

PuyoServerList::PuyoServerList(PuyoServerListResponder *responder)
  : m_responder(responder)
{
  char servname[256];
  char servpath[1024];
  int nbserv;

  // For debugging purpose
  //m_metaservers.push_back(new DummyMetaServerConnection(this));

  // Making a meta-server list from prefs
  nbserv = GetIntPreference(kInternetMetaServerNumberKey, 1);
  for (int i = 1; i <= nbserv; i++)
  {
    GetStrPreference (String(kInternetMetaServerKey)+i, servname, i==1?"aley.fovea.cc":"Error", 256);
    GetStrPreference (String(kInternetMetaServerPathKey)+i, servpath, i==1?"/flobopuyo/fpservers":"/fpservers", 1024);
    m_metaservers.push_back(new HttpMetaServerConnection(servname, servpath,
                              GetIntPreference (String(kInternetMetaServerPortKey)+i, 80), this));
  }
}

PuyoServerList::~PuyoServerList()
{
    for (std::vector<AbstractPuyoMetaServerConnection *>::iterator iter = m_metaservers.begin() ;
         iter != m_metaservers.end() ; iter++) {
         delete *iter;
    }
    for (std::vector<PingablePuyoServer *>::iterator iter = m_servers.begin() ;
         iter != m_servers.end() ; iter++) {
         delete *iter;
    }
}

void PuyoServerList::fetch()
{
    // Delete all the already found servers
    for (std::vector<PingablePuyoServer *>::iterator iter = m_servers.begin() ;
         iter != m_servers.end() ; iter++) {
         delete *iter;
    }
    m_servers.clear();
    // Notify the responder that we have a clear list now
    m_responder->PuyoServerListHasChanged(*this);
    // Launch the fetching process
    for (std::vector<AbstractPuyoMetaServerConnection *>::iterator iter = m_metaservers.begin() ;
         iter != m_metaservers.end() ; iter++) {
         (*iter)->fetch();
    }
}

void PuyoServerList::PuyoMetaServerListHasChanged(AbstractPuyoMetaServerConnection &metaServerConnection)
{
    vector<PuyoServer> servers = metaServerConnection.getServers();
    for (std::vector<PuyoServer>::iterator iter = servers.begin() ;
         iter != servers.end() ; iter++) {
         printf("List: %s:%d\n", (const char *)(iter->hostName), iter->portNum);
         m_servers.push_back(new PingablePuyoServer(iter->hostName, iter->portNum, iter->hostPath, this));
    }
}

std::vector<PuyoServer> PuyoServerList::getServers()
{
    std::vector<PuyoServer> result;
    for (std::vector<PingablePuyoServer *>::iterator iter = m_servers.begin() ;
         iter != m_servers.end() ; iter++) {
         if ((*iter)->answeredToPing())
            result.push_back(PuyoServer((*iter)->hostName, (*iter)->portNum, (*iter)->hostPath));
    }
    return result;
}

void PuyoServerList::puyoServerDidPing(PingablePuyoServer &server)
{
    m_responder->PuyoServerListHasChanged(*this);
}



PingablePuyoServer::PingablePuyoServer(String hostName, int portNum, String path, PingablePuyoServerResponder *responder)
  : PuyoServer(hostName, portNum, path),
    m_pingBox(hostName, 0, portNum), m_igpclient(m_pingBox, false), m_responder(responder), m_alreadyReported(false)
{
    GameUIDefaults::GAME_LOOP->addIdle(this);
    m_pingTransaction = m_igpclient.ping(10000., 1000.);
}

PingablePuyoServer::~PingablePuyoServer()
{
    delete m_pingTransaction;
}

void PingablePuyoServer::idle(double currentTime)
{
    if (!m_pingTransaction->completed())
        m_igpclient.idle();
    else {
        if ((!m_alreadyReported) && (m_pingTransaction->success())) {
            m_alreadyReported = true;
            m_responder->puyoServerDidPing(*this);
        }
    }
}

bool PingablePuyoServer::answeredToPing() const
{
    return m_alreadyReported;
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

InternetGameMenu::InternetGameMenu(MainScreen * mainScreen)
  : MainScreenMenu(mainScreen),
    screenTitleFrame(theCommander->getSeparatorFramePicture()),
    internetGameText(theCommander->getLocalizedString("Internet Game")),
    servers(this),
    //serverSelectionPanel(theCommander->getWindowFramePicture()),
    serverListPanel(20, theCommander->getUpArrow(), theCommander->getDownArrow(), theCommander->getListFramePicture()),
    serverListText(theCommander->getLocalizedString("Server List")),
    updating(theCommander->getLocalizedString("Update"), this, theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    //rightPanel(theCommander->getWindowFramePicture()),
    separator1_1(1,1), separator1_2(1,1), separator1_3(1, 1), separator10_1(10,10), //separator10_2(10,10),
    nicknameText(theCommander->getLocalizedString("Nickname"), NULL, false),
    passwordText(theCommander->getLocalizedString("Password"), NULL, false),
    serverText(theCommander->getLocalizedString("Server"), NULL, false),
    portText(theCommander->getLocalizedString("Port"), NULL, false),
    container(),
    playerName(PuyoGame::getPlayerName(-2), PuyoGame::getDefaultPlayerKey(-2),
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    password("", kInternetPasswordKey,
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    serverName(kInternetCurrentServerDefaultValue,kInternetCurrentServerKey,
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    serverPort(kInternetCurrentServerPortDefaultValue,kInternetCurrentServerPortKey,
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    backAction(mainScreen),
    joinButton(theCommander->getLocalizedString("Join"), this,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    backButton(theCommander->getLocalizedString("Back"), &backAction,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture())
{
    //this->setBorderVisible(false);
    servers.fetch();
}

InternetGameMenu::~InternetGameMenu()
{
}

void InternetGameMenu::build()
{
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&internetGameText);
    add(&screenTitleFrame);

    add(&menu);
    //add(&container);
    //container.add(&menu);

    //container.setPosition(Vec3(5,195));
    //container.setSize(Vec3(menuBG_wide->w, menuBG_wide->h, 0));

    serverSelectionPanel.setInnerMargin(10);
    serverSelectionPanel.setPolicy(USE_MIN_SIZE);
    serverSelectionPanel.add(&serverListText);
    serverSelectionPanel.add(&serverListPanel);
    serverSelectionPanel.add(&updating);

    rightPanel.setInnerMargin(10);
    rightPanel.add(&separator1_1);

    rightPanel.add(&separator1_2);
    nicknameText.setTextAlign(TEXT_LEFT_ALIGN);
    rightPanel.add(&nicknameText);
    rightPanel.add(&playerName);
    passwordText.setTextAlign(TEXT_LEFT_ALIGN);
    rightPanel.add(&passwordText);
    rightPanel.add(&password);
    rightPanel.add(&separator10_1);
    serverText.setTextAlign(TEXT_LEFT_ALIGN);
    rightPanel.add(&serverText);
    rightPanel.add(&serverName);
    portText.setTextAlign(TEXT_LEFT_ALIGN);
    rightPanel.add(&portText);
    rightPanel.add(&serverPort);
    rightPanel.add(&separator10_2);
    hbox.setPreferedSize(Vec3(0, joinButton.getPreferedSize().y + 20));
    hbox.add(&rightPanelSeparator);
    hbox.add(&joinButton);

    rightPanel.add(&hbox);
    rightPanel.add(&separator1_3);
    menu.add(&serverSelectionPanel);
    menu.add(&rightPanel);

    bottomPanel.setInnerMargin(10);
    bottomPanel.setPreferedSize(Vec3(0, backButton.getPreferedSize().y + 20));
    bottomPanel.setPolicy(USE_MIN_SIZE);
    bottomPanel.add(&bottomPanelSeparator);
    bottomPanel.add(&backButton);
    add(&bottomPanel);
}

class PuyoInternetDialog : public SliderContainer, public SliderContainerListener
{
public:
    PuyoInternetDialog(String dialogTitle);
    virtual ~PuyoInternetDialog();
    virtual void onSlideInside(SliderContainer &slider);
    void close();
    void idle(double v);
protected:
    VBox m_contentBox;
private:
    Frame m_dialogFrame;
    Frame m_titleFrame;
    Text m_titleText;
    bool m_closing, m_closed;
};

PuyoInternetDialog::PuyoInternetDialog(String dialogTitle)
  : SliderContainer(), m_dialogFrame(theCommander->getWindowFramePicture()),
    m_titleFrame(theCommander->getSeparatorFramePicture()),
    m_titleText(dialogTitle), m_closing(false), m_closed(false)
{
    m_titleFrame.add(&m_titleText);
    m_titleFrame.setPreferedSize(Vec3(0.0f, 20.0f));
    m_dialogFrame.add(&m_titleFrame);
    m_dialogFrame.add(&m_contentBox);
    setPreferedSize(Vec3(450.0f, 200.0f));
    setSize(getPreferedSize());
    setPosition(Vec3(95.0f, 150.0f));
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
    //this->getParentScreen()->remove(this);
    m_closed = true;
}

void PuyoInternetDialog::close()
{
    m_closing = true;
    transitionToContent(NULL);
}

void PuyoInternetDialog::idle(double v)
{
    SliderContainer::idle(v);
    if (m_closed) delete this;
}

class PuyoInternetErrorDialog : public PuyoInternetDialog, public Action
{
public:
    PuyoInternetErrorDialog(String errorMessageL1, String errorMessageL2);
    virtual ~PuyoInternetErrorDialog();
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    Text m_errorMessageL1, m_errorMessageL2;
    FramedButton m_okButton;
    IosSurfaceRef m_errorIconImage;
    Image m_errorIcon;
};

PuyoInternetErrorDialog::PuyoInternetErrorDialog(String errorMessageL1, String errorMessageL2)
  : PuyoInternetDialog(theCommander->getLocalizedString("Error")), m_errorMessageL1(errorMessageL1),
    m_errorMessageL2(errorMessageL2), m_okButton(theCommander->getLocalizedString("OK"), this,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    m_errorIconImage(theCommander->getSurface(IMAGE_RGBA, "gfx/errorpuyo.png")),
    m_errorIcon(m_errorIconImage)
{
    m_contentBox.setInnerMargin(10);
    m_contentBox.setPolicy(USE_MAX_SIZE);
    m_contentBox.add(&m_errorIcon);
    m_contentBox.add(&m_errorMessageL1);
    m_contentBox.add(&m_errorMessageL2);
    m_contentBox.add(&m_okButton);
    setSize(Vec3(450.0f, 200.0f));
}

PuyoInternetErrorDialog::~PuyoInternetErrorDialog()
{
}

void PuyoInternetErrorDialog::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (sender == m_okButton.getButton()) {
        close();
    }
}

class PuyoInternetConnectDialog : public PuyoInternetDialog, public Action
{
public:
    PuyoInternetConnectDialog(String serverName, InternetGameCenter *gameCenter, InternetGameMenu *owner);
    virtual ~PuyoInternetConnectDialog();
    virtual void idle(double currentTime);
    void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    Text m_messageL1, m_messageL2;
    FramedButton m_cancelButton;
    InternetGameCenter *m_gameCenter;
    InternetGameMenu *m_owner;
    double m_startTime;
    bool m_timeout;
    static const double CONNECT_TIMEOUT;
};

const double PuyoInternetConnectDialog::CONNECT_TIMEOUT = 5.;

PuyoInternetConnectDialog::PuyoInternetConnectDialog(String serverName, InternetGameCenter *gameCenter, InternetGameMenu *owner)
    : PuyoInternetDialog("Connecting"),
      m_messageL1("Connecting to server"), m_messageL2(serverName),
      m_cancelButton(theCommander->getLocalizedString("Cancel"), this,
		     theCommander->getButtonFramePicture(),
		     theCommander->getButtonOverFramePicture()),
      m_gameCenter(gameCenter), m_owner(owner), m_startTime(0.), m_timeout(false)
{
    m_contentBox.add(&m_messageL1);
    m_contentBox.add(&m_messageL2);
    m_contentBox.add(&m_cancelButton);
}

PuyoInternetConnectDialog::~PuyoInternetConnectDialog()
{
    if (m_gameCenter->isConnected()) {
        m_owner->enterNetCenterMenu(m_gameCenter);
	return;
    }
    else if ((m_gameCenter->isDenied()) || (m_timeout)) {
        PuyoInternetErrorDialog *errorDialog;
	if (m_timeout)
	  errorDialog = new PuyoInternetErrorDialog(String("Server didn't answered"), m_messageL2.getValue());
	else
	    errorDialog = new PuyoInternetErrorDialog(m_gameCenter->getDenyString(), m_gameCenter->getDenyStringMore());
        m_owner->getParentScreen()->add(errorDialog);
        m_owner->getParentScreen()->grabEventsOnWidget(errorDialog);
        AudioManager::playSound("ebenon.wav", 0.5);
    }
    delete m_gameCenter;
}

void PuyoInternetConnectDialog::idle(double currentTime)
{
    if (m_startTime == 0.)
        m_startTime = currentTime;
    if (currentTime - m_startTime > CONNECT_TIMEOUT) {
        close();
	m_timeout = true;
	PuyoInternetDialog::idle(currentTime);
	m_startTime = 0.;
    }
    m_gameCenter->idle();
    if ((m_gameCenter->isConnected()) || (m_gameCenter->isDenied()))
        close();
    PuyoInternetDialog::idle(currentTime);
}

void PuyoInternetConnectDialog::action(Widget *sender, int actionType, GameControlEvent *event)
{
  if (sender == m_cancelButton.getButton()) {
    close();
  }
}

void InternetGameMenu::enterNetCenterMenu(InternetGameCenter *gameCenter)
{
    NetCenterMenu *newNetCenterMenu = new NetCenterMenu(mainScreen, gameCenter,
                      theCommander->getLocalizedString("Internet Game Center"));
    newNetCenterMenu->build();
    mainScreen->pushMenu(newNetCenterMenu, true);
}

void InternetGameMenu::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (sender == this->joinButton.getButton()) {
        try {
            InternetGameCenter *gameCenter = new InternetGameCenter(serverName.getEditField().getValue(),
                                                                            atoi(serverPort.getEditField().getValue()), playerName.getEditField().getValue(), password.getEditField().getValue());
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
    else if (sender == this->updating.getButton()) {
        this->servers.fetch();
    }
}

void InternetGameMenu::idle(double currentTime)
{
    /*int state = servers.fetchingNewData();
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
    }*/
}

void InternetGameMenu::setSelectedServer(const String &serverName, int portNum)
{
    char sportNum[256];
    sprintf(sportNum, "%d", portNum);
    this->serverName.setValue(serverName);
    this->serverPort.setValue(sportNum);
    this->portNum = portNum;
}

void InternetGameMenu::PuyoServerListHasChanged(PuyoServerList &serverList)
{
    while (serverListPanel.getFullSize() > 0) {
        ListViewEntry *entry = serverListPanel.getEntryAt(0);
        serverListPanel.removeEntry(entry);
        delete entry;
    }
    std::vector<PuyoServer> servers = serverList.getServers();
    for (std::vector<PuyoServer>::iterator iter = servers.begin() ;
         iter != servers.end() ; iter++) {
         serverListPanel.addEntry(new ListViewEntry(iter->hostName,
                                                   new ServerSelectAction(*this, iter->hostName,
                                                                          iter->portNum)));
    }
    updating.setValue(theCommander->getLocalizedString("Update"));
}

