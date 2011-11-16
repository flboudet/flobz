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
    FPServer(String hostName, int portNum, String path) : hostName(hostName), hostPath(path), portNum(portNum) {}
    virtual ~FPServer() {}
    String hostName;
    String hostPath;
    int portNum;
};

class PingableFPServer;

class PingableFPServerResponder {
public:
    virtual ~PingableFPServerResponder() {}
    virtual void fpServerDidPing(PingableFPServer &server) = 0;
};

class PingableFPServer : public FPServer, public IdleComponent {
public:
    PingableFPServer(String hostName, int portNum, String path, PingableFPServerResponder *responder);
    virtual ~PingableFPServer();
    // IdleComponent implementation
    virtual void idle(double currentTime);
    bool answeredToPing() const;
private:
    std::auto_ptr<ios_fc::DatagramSocket> m_pingSocket;
    std::auto_ptr<FPServerMessageBox> m_pingBox;
    std::auto_ptr<ios_fc::IGPClient> m_igpclient;
    IGPClient::PingTransaction *m_pingTransaction;
    PingableFPServerResponder *m_responder;
    bool m_alreadyReported;
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
    MetaFPServerConnectionResponder *m_responder;
};

class DummyMetaServerConnection : public AbstractMetaFPServerConnection {
public:
    DummyMetaServerConnection(MetaFPServerConnectionResponder *responder);
    virtual void fetch();
    virtual std::vector<FPServer> getServers() const;
};

class HttpMetaServerConnection : public AbstractMetaFPServerConnection, public IdleComponent {
public:
    HttpMetaServerConnection(String hostName, String hostPath, int portNum, MetaFPServerConnectionResponder *responder);
    virtual ~HttpMetaServerConnection();
    virtual void fetch();
    virtual std::vector<FPServer> getServers() const;
    // IdleComponent implementation
    virtual void idle(double currentTime);
    virtual bool failed() const { return m_nErrors > 5; }
private:
    HttpDocument *m_doc;
    String m_hostName, m_hostPath;
    int m_portNum;
    std::vector<FPServer> m_servers;
    int m_nErrors;
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
    std::vector<AbstractMetaFPServerConnection *> m_metaservers;
    std::vector<PingableFPServer *> m_servers;
    FPServerListResponder *m_responder;
	int fetching;
	bool firstTime;
};


class InternetGameMenu : public MainScreenMenu, public IdleComponent, public Action, public FPServerListResponder {
public:
    InternetGameMenu(MainScreen * mainScreen);
    virtual ~InternetGameMenu();
    void build();
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    void setSelectedServer(const String &s, int port);
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
    void enterNetCenterMenu(InternetGameCenter *gameCenter);
    // FPServerListResponder implementation
    virtual void FPServerListHasChanged(FPServerList &serverList);
private:
    Frame screenTitleFrame;
    Text internetGameText;
    FPServerList servers;
    VBox serverSelectionPanel;
    ListView serverListPanel;
    Text serverListText;
    FramedButton updating;
    VBox rightPanel;
    Separator separator1_1, separator1_2, separator1_3,  separator10_1, separator10_2;
    Text nicknameText, passwordText, serverText, portText;
    HBox hbox, menu;
    HBox bottomPanel;
    Separator bottomPanelSeparator, rightPanelSeparator;
    SliderContainer container;
    FramedEditField playerName, password;
    FramedEditField serverName, serverPort;
    int portNum;
    PopMainScreenMenuAction backAction;
    FramedButton joinButton, backButton;
};

#endif // _INTERNETMENU_H_

