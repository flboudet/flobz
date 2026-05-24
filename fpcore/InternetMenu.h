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

#ifndef _INTERNETMENU_H_
#define _INTERNETMENU_H_

#include "ios_httpdocument.h"
#include "ios_igpclient.h"
#include "gameui.h"
#include "Frame.h"
#include "FramedButton.h"
#include "FramedEditField.h"
#include "ListView.h"
#include "FPCommander.h"
#include "MainScreen.h"
#include "InternetGameCenter.h"
#include <vector>

using namespace ios_fc;

class FPServer {
public:
    FPServer(const std::string & hostName, int portNum, const std::string & path) : _hostName(hostName), _hostPath(path), _portNum(portNum) {}
    virtual ~FPServer() {}
    std::string _hostName;
    std::string _hostPath;
    int _portNum;
};

class PingableFPServer;

class PingableFPServerResponder {
public:
    virtual ~PingableFPServerResponder() {}
    virtual void fpServerDidPing(PingableFPServer &server) = 0;
};

class PingableFPServer : public FPServer, public IdleComponent {
public:
    PingableFPServer(const std::string & hostName, int portNum, const std::string & path, PingableFPServerResponder *responder);
    virtual ~PingableFPServer();
    // IdleComponent implementation
    virtual void idle(double currentTime);
    bool answeredToPing() const;
private:
    std::unique_ptr<ios_fc::DatagramSocket> _pingSocket;
    std::unique_ptr<FPServerMessageBox> _pingBox;
    std::unique_ptr<ios_fc::IGPClient> _igpclient;
    IGPClient::PingTransaction *_pingTransaction;
    PingableFPServerResponder *_responder;
    bool _alreadyReported;
};

class AbstractMetaFPServerConnection;

class MetaFPServerConnectionResponder {
public:
    virtual ~MetaFPServerConnectionResponder() {}
    virtual void metaFPServerListHasChanged(AbstractMetaFPServerConnection &metaServerConnection) = 0;
};

class AbstractMetaFPServerConnection {
public:
    AbstractMetaFPServerConnection(MetaFPServerConnectionResponder *responder);
    virtual ~AbstractMetaFPServerConnection() {}
    virtual void fetch() = 0;
    virtual std::vector<FPServer> getServers() const = 0;
    virtual bool failed() const { return false; }
protected:
    virtual void onServerListHasChanged();
private:
    MetaFPServerConnectionResponder *_responder;
};

class DummyMetaServerConnection : public AbstractMetaFPServerConnection {
public:
    DummyMetaServerConnection(MetaFPServerConnectionResponder *responder);
    virtual void fetch();
    virtual std::vector<FPServer> getServers() const;
};

class HttpMetaServerConnection : public AbstractMetaFPServerConnection, public IdleComponent {
public:
    HttpMetaServerConnection(const std::string & hostName, const std::string & hostPath, int portNum, MetaFPServerConnectionResponder *responder);
    virtual ~HttpMetaServerConnection();
    virtual void fetch();
    virtual std::vector<FPServer> getServers() const;
    // IdleComponent implementation
    virtual void idle(double currentTime);
    virtual bool failed() const { return _nErrors > 5; }
private:
    HttpDocument *_doc;
    std::string _hostName, _hostPath;
    int _portNum;
    std::vector<FPServer> _servers;
    int _nErrors;
};



class FPServerList;

class FPServerListResponder {
public:
    virtual ~FPServerListResponder() {}
    virtual void FPServerListHasChanged(FPServerList &serverList) = 0;
};

class FPServerList : public MetaFPServerConnectionResponder, public PingableFPServerResponder {
public:
    FPServerList(FPServerListResponder *responder);
    virtual ~FPServerList();
    void fetch();
    std::vector<FPServer> getServers();
    // MetaFPServerConnectionResponder implementation
    virtual void metaFPServerListHasChanged(AbstractMetaFPServerConnection &metaServerConnection);
    // PingableFPServerResponder implementation
    virtual void fpServerDidPing(PingableFPServer &server);
private:
    std::vector<AbstractMetaFPServerConnection *> _metaservers;
    std::vector<PingableFPServer *> _servers;
    FPServerListResponder *_responder;
	int _fetching;
	bool _firstTime;
};


class InternetGameMenu : public MainScreenMenu, public IdleComponent, public Action, public FPServerListResponder {
public:
    InternetGameMenu(MainScreen * mainScreen);
    virtual ~InternetGameMenu();
    void build();
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    void setSelectedServer(const std::string &s, int port);
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
    void enterNetCenterMenu(InternetGameCenter *gameCenter);
    // FPServerListResponder implementation
    virtual void FPServerListHasChanged(FPServerList &serverList);
private:
    Frame _screenTitleFrame;
    Text _internetGameText;
    FPServerList _servers;
    VBox _serverSelectionPanel;
    ListView _serverListPanel;
    Text _serverListText;
    FramedButton _updating;
    VBox _rightPanel;
    Separator _separator1_1, _separator1_2, _separator1_3,  _separator10_1, _separator10_2;
    Text _nicknameText, _passwordText, _serverText, _portText;
    HBox _hbox, _menu;
    HBox _bottomPanel;
    Separator _bottomPanelSeparator, _rightPanelSeparator;
    SliderContainer _container;
    FramedEditField _playerName, _password;
    FramedEditField _serverName, _serverPort;
    int _portNum;
    PopMainScreenMenuAction _backAction;
    FramedButton _joinButton, _backButton;
};

#endif // _INTERNETMENU_H_

