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

#include "ios_standardmessage.h"
#include "InternetMenu.h"
#include "InternetGameCenter.h"
#include "NetCenterMenu.h"
#include "PlayerNameUtils.h"

using namespace std;
using namespace event_manager;

static const std::string kInternetMetaServerKey = "Menu.Internet.MetaServer.";
static const std::string kInternetMetaServerPathKey = "Menu.Internet.MetaServer.Path.";
static const std::string kInternetMetaServerPortKey = "Menu.Internet.MetaServer.Port.";
static const std::string kInternetMetaServerNumberKey = "Menu.Internet.MetaServer.Number.";

static const std::string kInternetPasswordKey = "Menu.Internet.Password";
static const std::string kInternetCurrentServerKey = "Menu.Internet.CurrentServer";
static const std::string kInternetCurrentServerDefaultValue = "aley.fovea.cc";
static const std::string kInternetCurrentServerPortKey = "Menu.Internet.CurrentServer.Port";
static const std::string kInternetCurrentServerPortDefaultValue = "4567";

AbstractMetaFPServerConnection::AbstractMetaFPServerConnection(MetaFPServerConnectionResponder *responder)
  : _responder(responder)
{}

void AbstractMetaFPServerConnection::onServerListHasChanged()
{
    _responder->metaFPServerListHasChanged(*this);
}

DummyMetaServerConnection::DummyMetaServerConnection(MetaFPServerConnectionResponder *responder)
  : AbstractMetaFPServerConnection(responder)
{
}

void DummyMetaServerConnection::fetch()
{
    onServerListHasChanged();
}

vector<FPServer> DummyMetaServerConnection::getServers() const
{
    vector<FPServer> result;
    result.push_back(FPServer("immunity.local", 4567, "immunity"));
    result.push_back(FPServer("durandal.local", 4567, "durandal"));
    result.push_back(FPServer("gfive.local", 4567, "gfive"));
    return result;
}

HttpMetaServerConnection::HttpMetaServerConnection(const std::string &hostName, const std::string & hostPath, int portNum, MetaFPServerConnectionResponder *responder)
  : AbstractMetaFPServerConnection(responder), _doc(NULL),
    _hostName(hostName), _hostPath(hostPath), _portNum(portNum), _nErrors(0)
{
    GameUIDefaults::GAME_LOOP->addIdle(this);
}

HttpMetaServerConnection::~HttpMetaServerConnection()
{
    GameUIDefaults::GAME_LOOP->removeIdle(this);
    if (_doc != NULL)
        delete _doc;
}

void HttpMetaServerConnection::fetch()
{
    if (_nErrors > 5) {
        GameUIDefaults::GAME_LOOP->removeIdle(this);
        return;
    }
    try {
        _servers.clear();
        _doc = new HttpDocument(_hostName.c_str(), _hostPath.c_str(), _portNum);
    } catch (const std::exception &e) {
        ios_fc::printException(e);
        _nErrors += 1;
        _doc = NULL;
    }
}

std::vector<FPServer> HttpMetaServerConnection::getServers() const
{
    return _servers;
}

void HttpMetaServerConnection::idle(double currentTime)
{
    if (_nErrors > 5) {
        GameUIDefaults::GAME_LOOP->removeIdle(this);
        _servers.push_back(FPServer("aley.fovea.cc", 4567, ""));
        _servers.push_back(FPServer("localhost", 4567, ""));
        onServerListHasChanged();
        return;
    }
    if (_doc == NULL) return;
    try {
		if (_doc->documentIsReady()) {
            StandardMessage msg(_doc->getDocumentContent());
            int nbServers = msg.getInt("NBSERV");
            for (int i = 0 ; i < nbServers ; i++) {
                char tmpStr[256];
                sprintf(tmpStr, "SERVNAME%.2d", i);
                std::string serverName = msg.getString(tmpStr).c_str(); // TODO string
                sprintf(tmpStr, "PORTNUM%.2d", i);
                int portNum = msg.getInt(tmpStr);
                _servers.push_back(FPServer(serverName, portNum, ""));
            }
            delete _doc;
            _doc = NULL;
            onServerListHasChanged();
        }
    } catch (const std::exception &e) { // Erreur dans la reception du fichier
        ios_fc::printException(e);
        _nErrors += 1;
    }
}

FPServerList::FPServerList(FPServerListResponder *responder)
  : _responder(responder)
{
    std::string servname;
    std::string servpath;
    int nbserv;

    // For debugging purpose
    //_metaservers.push_back(new DummyMetaServerConnection(this));

    // Making a meta-server list from prefs
    nbserv = theCommander->getPreferencesManager()->getIntPreference(kInternetMetaServerNumberKey, 1);
    for (int i = 1; i <= nbserv; i++)
    {
        servname = theCommander->getPreferencesManager()->getStrPreference(kInternetMetaServerKey+std::to_string(i), i==1?"aley.fovea.cc":"Error");
        servpath = theCommander->getPreferencesManager()->getStrPreference (kInternetMetaServerPathKey+std::to_string(i), i==1?"/flobopop/fpservers":"/fpservers");
        _metaservers.push_back(new HttpMetaServerConnection(servname.c_str(), servpath.c_str(),
                                                             theCommander->getPreferencesManager()->getIntPreference(kInternetMetaServerPortKey+std::to_string(i), 80), this));
  }
}

FPServerList::~FPServerList()
{
    for (std::vector<AbstractMetaFPServerConnection *>::iterator iter = _metaservers.begin() ;
         iter != _metaservers.end() ; iter++) {
         delete *iter;
    }
    for (std::vector<PingableFPServer *>::iterator iter = _servers.begin() ;
         iter != _servers.end() ; iter++) {
         delete *iter;
    }
}

void FPServerList::fetch()
{
    // Delete all the already found servers
    for (std::vector<PingableFPServer *>::iterator iter = _servers.begin() ;
         iter != _servers.end() ; iter++) {
         delete *iter;
    }
    _servers.clear();
    // Notify the responder that we have a clear list now
    _responder->FPServerListHasChanged(*this);
    // Launch the fetching process
    for (std::vector<AbstractMetaFPServerConnection *>::iterator iter = _metaservers.begin() ;
         iter != _metaservers.end() ; iter++) {
         (*iter)->fetch();
    }
}

void FPServerList::metaFPServerListHasChanged(AbstractMetaFPServerConnection &metaServerConnection)
{
    vector<FPServer> servers = metaServerConnection.getServers();
    for (std::vector<FPServer>::iterator iter = servers.begin() ;
         iter != servers.end() ; iter++) {
         printf("List: %s:%d\n", iter->_hostName.c_str(), iter->_portNum);
         _servers.push_back(new PingableFPServer(iter->_hostName, iter->_portNum, iter->_hostPath, this));
    }
}

std::vector<FPServer> FPServerList::getServers()
{
    std::vector<FPServer> result;
    for (std::vector<PingableFPServer *>::iterator iter = _servers.begin() ;
         iter != _servers.end() ; iter++) {
         if ((*iter)->answeredToPing())
            result.push_back(FPServer((*iter)->_hostName, (*iter)->_portNum, (*iter)->_hostPath));
    }
    return result;
}

void FPServerList::fpServerDidPing(PingableFPServer &server)
{
    _responder->FPServerListHasChanged(*this);
}



PingableFPServer::PingableFPServer(const std::string & hostName, int portNum, const std::string & path, PingableFPServerResponder *responder)
  : FPServer(hostName, portNum, path),
    _responder(responder), _alreadyReported(false)
{
    _pingSocket.reset(new DatagramSocket());
    _pingSocket->connect(hostName, portNum);
    _pingBox.reset(new FPServerMessageBox(_pingSocket.get()));
    _igpclient.reset(new IGPClient(*_pingBox, false));
    GameUIDefaults::GAME_LOOP->addIdle(this);
    _pingTransaction = _igpclient->ping(10000., 1000.);
}

PingableFPServer::~PingableFPServer()
{
    delete _pingTransaction;
}

void PingableFPServer::idle(double currentTime)
{
    if (!_pingTransaction->completed())
        _igpclient->idle();
    else {
        if ((!_alreadyReported) && (_pingTransaction->success())) {
            _alreadyReported = true;
            _responder->fpServerDidPing(*this);
        }
    }
}

bool PingableFPServer::answeredToPing() const
{
    return _alreadyReported;
}

// Actions
class ServerSelectAction : public Action {
public:
    ServerSelectAction(InternetGameMenu &igm, const std::string & serverName, int portNum)
      : _gameMenu(igm), _serverName(serverName), _portNum(portNum) {}
    void action() {
        _gameMenu.setSelectedServer(_serverName, _portNum);
    }
private:
    InternetGameMenu &_gameMenu;
    std::string _serverName;
    int _portNum;
};

InternetGameMenu::InternetGameMenu(MainScreen * mainScreen)
  : MainScreenMenu(mainScreen),
    _screenTitleFrame(theCommander->getSeparatorFramePicture()),
    _internetGameText(theCommander->getLocalizedString("Internet Game")),
    _servers(this),
    //_serverSelectionPanel(theCommander->getWindowFramePicture()),
    _serverListPanel(20, theCommander->getUpArrow(), theCommander->getDownArrow(), theCommander->getListFramePicture()),
    _serverListText(theCommander->getLocalizedString("Server List")),
    _updating(theCommander->getLocalizedString("Update"), this, theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    //_rightPanel(theCommander->getWindowFramePicture()),
    _separator1_1(1,1), _separator1_2(1,1), _separator1_3(1, 1), _separator10_1(10,10), //_separator10_2(10,10),
    _nicknameText(theCommander->getLocalizedString("Nickname"), NULL, false),
    _passwordText(theCommander->getLocalizedString("Password"), NULL, false),
    _serverText(theCommander->getLocalizedString("Server"), NULL, false),
    _portText(theCommander->getLocalizedString("Port"), NULL, false),
    _container(),
    _playerName(PlayerNameUtils::getPlayerName(-2).c_str(), PlayerNameUtils::getDefaultPlayerKey(-2).c_str(),
               theCommander->getPreferencesManager(),
               theCommander->getEditFieldFramePicture(),
               theCommander->getEditFieldOverFramePicture()),
    _password("", kInternetPasswordKey, theCommander->getPreferencesManager(),
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    _serverName(kInternetCurrentServerDefaultValue,kInternetCurrentServerKey,
               theCommander->getPreferencesManager(),
               theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    _serverPort(kInternetCurrentServerPortDefaultValue,kInternetCurrentServerPortKey,
               theCommander->getPreferencesManager(),
	       theCommander->getEditFieldFramePicture(), theCommander->getEditFieldOverFramePicture()),
    _backAction(mainScreen),
    _joinButton(theCommander->getLocalizedString("Join").c_str(), this,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    _backButton(theCommander->getLocalizedString("Back").c_str(), &_backAction,
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()) // TODO string
{
    //this->setBorderVisible(false);
    _servers.fetch();
}

InternetGameMenu::~InternetGameMenu()
{
}

void InternetGameMenu::build()
{
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _screenTitleFrame.add(&_internetGameText);
    add(&_screenTitleFrame);

    add(&_menu);
    //add(&_container);
    //_container.add(&_menu);

    //_container.setPosition(Vec3(5,195));
    //_container.setSize(Vec3(menuBG_wide->w, menuBG_wide->h, 0));

    _serverSelectionPanel.setInnerMargin(10);
    _serverSelectionPanel.setPolicy(USE_MIN_SIZE);
    _serverSelectionPanel.add(&_serverListText);
    _serverSelectionPanel.add(&_serverListPanel);
    _serverSelectionPanel.add(&_updating);

    _rightPanel.setInnerMargin(10);
    _rightPanel.add(&_separator1_1);

    _rightPanel.add(&_separator1_2);
    _nicknameText.setTextAlign(TEXT_LEFT_ALIGN);
    _rightPanel.add(&_nicknameText);
    _rightPanel.add(&_playerName);
    _passwordText.setTextAlign(TEXT_LEFT_ALIGN);
    _rightPanel.add(&_passwordText);
    _rightPanel.add(&_password);
    _rightPanel.add(&_separator10_1);
    _serverText.setTextAlign(TEXT_LEFT_ALIGN);
    _rightPanel.add(&_serverText);
    _rightPanel.add(&_serverName);
    _portText.setTextAlign(TEXT_LEFT_ALIGN);
    _rightPanel.add(&_portText);
    _rightPanel.add(&_serverPort);
    _rightPanel.add(&_separator10_2);
    _hbox.setPreferedSize(Vec3(0, _joinButton.getPreferedSize().y + 20));
    _hbox.add(&_rightPanelSeparator);
    _hbox.add(&_joinButton);

    _rightPanel.add(&_hbox);
    _rightPanel.add(&_separator1_3);
    _menu.add(&_serverSelectionPanel);
    _menu.add(&_rightPanel);

    _bottomPanel.setInnerMargin(10);
    _bottomPanel.setPreferedSize(Vec3(0, _backButton.getPreferedSize().y + 20));
    _bottomPanel.setPolicy(USE_MIN_SIZE);
    _bottomPanel.add(&_bottomPanelSeparator);
    _bottomPanel.add(&_backButton);
    add(&_bottomPanel);
}

class InternetDialog : public SliderContainer, public SliderContainerListener
{
public:
    InternetDialog(const std::string &dialogTitle);
    virtual ~InternetDialog();
    virtual void onSlideInside(SliderContainer &slider);
    void close();
    void idle(double v);
protected:
    VBox _contentBox;
private:
    Frame _dialogFrame;
    Frame _titleFrame;
    Text _titleText;
    bool _closing, _closed;
};

InternetDialog::InternetDialog(const std::string &dialogTitle)
  : SliderContainer(), _dialogFrame(theCommander->getWindowFramePicture()),
    _titleFrame(theCommander->getSeparatorFramePicture()),
    _titleText(dialogTitle), _closing(false), _closed(false)
{
    _titleFrame.add(&_titleText);
    _titleFrame.setPreferedSize(Vec3(0.0f, 20.0f));
    _dialogFrame.add(&_titleFrame);
    _dialogFrame.add(&_contentBox);
    setPreferedSize(Vec3(450.0f, 200.0f));
    setSize(getPreferedSize());
    setPosition(Vec3(95.0f, 150.0f));
    this->addListener(*this);
    transitionToContent(&_dialogFrame);
}

InternetDialog::~InternetDialog()
{
    //this->getParentScreen()->ungrabEventsOnWidget(this);
}

void InternetDialog::onSlideInside(SliderContainer &slider)
{
    if (!_closing)
        return;
    //this->getParentScreen()->remove(this);
    _closed = true;
}

void InternetDialog::close()
{
    _closing = true;
    transitionToContent(NULL);
}

void InternetDialog::idle(double v)
{
    SliderContainer::idle(v);
    if (_closed) delete this;
}

class InternetErrorDialog : public InternetDialog, public Action
{
public:
    InternetErrorDialog(const std::string &errorMessageL1, const std::string &errorMessageL2);
    virtual ~InternetErrorDialog();
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    Text _errorMessageL1, _errorMessageL2;
    FramedButton _okButton;
    IosSurfaceRef _errorIconImage;
    Image _errorIcon;
};

InternetErrorDialog::InternetErrorDialog(const std::string &errorMessageL1, const std::string &errorMessageL2)
  : InternetDialog(theCommander->getLocalizedString("Error").c_str()), _errorMessageL1(errorMessageL1),
    _errorMessageL2(errorMessageL2), _okButton(theCommander->getLocalizedString("OK").c_str(), this, // TODO string
	       theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
    _errorIconImage(theCommander->getSurface(IMAGE_RGBA, "gfx/erroricon.png")),
    _errorIcon(_errorIconImage)
{
    _contentBox.setInnerMargin(10);
    _contentBox.setPolicy(USE_MAX_SIZE);
    _contentBox.add(&_errorIcon);
    _contentBox.add(&_errorMessageL1);
    _contentBox.add(&_errorMessageL2);
    _contentBox.add(&_okButton);
    setSize(Vec3(450.0f, 200.0f));
}

InternetErrorDialog::~InternetErrorDialog()
{
}

void InternetErrorDialog::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (sender == _okButton.getButton()) {
        close();
    }
}

class InternetConnectDialog : public InternetDialog, public Action
{
public:
    InternetConnectDialog(const std::string &serverName, InternetGameCenter *gameCenter, InternetGameMenu *owner);
    virtual ~InternetConnectDialog();
    virtual void idle(double currentTime);
    void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    Text _messageL1, _messageL2;
    FramedButton _cancelButton;
    InternetGameCenter *_gameCenter;
    InternetGameMenu *_owner;
    double _startTime;
    bool _timeout;
    static const double CONNECT_TIMEOUT;
};

const double InternetConnectDialog::CONNECT_TIMEOUT = 5.;

InternetConnectDialog::InternetConnectDialog(const std::string &serverName, InternetGameCenter *gameCenter, InternetGameMenu *owner)
    : InternetDialog("Connecting"),
      _messageL1("Connecting to server"), _messageL2(serverName),
      _cancelButton(theCommander->getLocalizedString("Cancel").c_str(), this, // TODO: string
		     theCommander->getButtonFramePicture(),
		     theCommander->getButtonOverFramePicture()),
      _gameCenter(gameCenter), _owner(owner), _startTime(0.), _timeout(false)
{
    _contentBox.add(&_messageL1);
    _contentBox.add(&_messageL2);
    _contentBox.add(&_cancelButton);
}

InternetConnectDialog::~InternetConnectDialog()
{
    if (_gameCenter->isConnected()) {
        _owner->enterNetCenterMenu(_gameCenter);
	return;
    }
    else if ((_gameCenter->isDenied()) || (_timeout)) {
        InternetErrorDialog *errorDialog;
	if (_timeout)
	  errorDialog = new InternetErrorDialog("Server didn't answered", _messageL2.getValue());
	else
	    errorDialog = new InternetErrorDialog(_gameCenter->getDenyString(), _gameCenter->getDenyStringMore());
        _owner->getParentScreen()->add(errorDialog);
        _owner->getParentScreen()->grabEventsOnWidget(errorDialog);
        theCommander->playSound("ebenon.wav", 0.5);
    }
    delete _gameCenter;
}

void InternetConnectDialog::idle(double currentTime)
{
    if (_startTime == 0.)
        _startTime = currentTime;
    if (currentTime - _startTime > CONNECT_TIMEOUT) {
        close();
	_timeout = true;
	InternetDialog::idle(currentTime);
	_startTime = 0.;
    }
    _gameCenter->idle();
    if ((_gameCenter->isConnected()) || (_gameCenter->isDenied()))
        close();
    InternetDialog::idle(currentTime);
}

void InternetConnectDialog::action(Widget *sender, int actionType, GameControlEvent *event)
{
  if (sender == _cancelButton.getButton()) {
    close();
  }
}

void InternetGameMenu::enterNetCenterMenu(InternetGameCenter *gameCenter)
{
    NetCenterMenu *newNetCenterMenu = new NetCenterMenu(_mainScreen, gameCenter,
                      theCommander->getLocalizedString("Internet Game Center").c_str()); // TODO: string
    newNetCenterMenu->build();
    _mainScreen->pushMenu(newNetCenterMenu, true);
}

void InternetGameMenu::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (sender == this->_joinButton.getButton()) {
        try {
            InternetGameCenter *gameCenter = new InternetGameCenter(_serverName.getEditField().getValue(),
                                                                            atoi(_serverPort.getEditField().getValue().c_str()), _playerName.getEditField().getValue(), _password.getEditField().getValue());
            InternetConnectDialog *connectionDialog = new InternetConnectDialog(_serverName.getEditField().getValue().c_str(), gameCenter, this);
            this->getParentScreen()->add(connectionDialog);
            this->getParentScreen()->grabEventsOnWidget(connectionDialog);
        } catch (const std::exception &e) {
            fprintf(stderr, "Error while connecting to %s\n", _serverName.getEditField().getValue().c_str());
            ios_fc::printException(e);
            InternetErrorDialog *errorDialog = new InternetErrorDialog("Cannot connect to", _serverName.getEditField().getValue());
            this->getParentScreen()->add(errorDialog);
            this->getParentScreen()->grabEventsOnWidget(errorDialog);
            theCommander->playSound("ebenon.wav", 0.5);
        }
    }
    else if (sender == this->_updating.getButton()) {
        this->_servers.fetch();
    }
}

void InternetGameMenu::idle(double currentTime)
{
    /*int state = _servers.fetchingNewData();
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
        _updating.setValue(txt[X]);
    }
    else if (state < 0)
    {
      _updating.setValue("Update");
    }*/
}

void InternetGameMenu::setSelectedServer(const std::string &serverName, int portNum)
{
    char sportNum[256];
    sprintf(sportNum, "%d", portNum);
    this->_serverName.setValue(serverName);
    this->_serverPort.setValue(sportNum);
    this->_portNum = portNum;
}

void InternetGameMenu::FPServerListHasChanged(FPServerList &serverList)
{
    while (_serverListPanel.getFullSize() > 0) {
        ListViewEntry *entry = _serverListPanel.getEntryAt(0);
        _serverListPanel.removeEntry(entry);
        delete entry;
    }
    std::vector<FPServer> servers = serverList.getServers();
    for (std::vector<FPServer>::iterator iter = servers.begin() ;
         iter != servers.end() ; iter++) {
         _serverListPanel.addEntry(new ListViewEntry(iter->_hostName,
                                                    new ServerSelectAction(*this, iter->_hostName,
                                                                           iter->_portNum)));
    }
    _updating.setValue(theCommander->getLocalizedString("Update"));
}

