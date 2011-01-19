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

#ifndef _PUYOINTERNETMENU_H
#define _PUYOINTERNETMENU_H

#include "ios_httpdocument.h"
#include "ios_igpclient.h"
#include "gameui.h"
#include "Frame.h"
#include "FramedButton.h"
#include "FramedEditField.h"
#include "ListView.h"
#include "PuyoCommander.h"
#include "MainScreen.h"
#include "InternetGameCenter.h"
#include <vector>

using namespace ios_fc;

class PuyoServer {
public:
    PuyoServer(String hostName, int portNum, String path) : hostName(hostName), hostPath(path), portNum(portNum) {}
    virtual ~PuyoServer() {}
    String hostName;
    String hostPath;
    int portNum;
};

class PingablePuyoServer;

class PingablePuyoServerResponder {
public:
    virtual ~PingablePuyoServerResponder() {}
    virtual void puyoServerDidPing(PingablePuyoServer &server) = 0;
};

class PingablePuyoServer : public PuyoServer, public IdleComponent {
public:
    PingablePuyoServer(String hostName, int portNum, String path, PingablePuyoServerResponder *responder);
    virtual ~PingablePuyoServer();
    // IdleComponent implementation
    virtual void idle(double currentTime);
    bool answeredToPing() const;
private:
    ios_fc::UDPMessageBox<ios_fc::FastMessage> m_pingBox;
    ios_fc::IGPClient m_igpclient;
    IGPClient::PingTransaction *m_pingTransaction;
    PingablePuyoServerResponder *m_responder;
    bool m_alreadyReported;
};

class AbstractPuyoMetaServerConnection;

class PuyoMetaServerConnectionResponder {
public:
    virtual ~PuyoMetaServerConnectionResponder() {}
    virtual void PuyoMetaServerListHasChanged(AbstractPuyoMetaServerConnection &metaServerConnection) = 0;
};

class AbstractPuyoMetaServerConnection {
public:
    AbstractPuyoMetaServerConnection(PuyoMetaServerConnectionResponder *responder);
    virtual ~AbstractPuyoMetaServerConnection() {}
    virtual void fetch() = 0;
    virtual std::vector<PuyoServer> getServers() const = 0;
    virtual bool failed() const { return false; }
protected:
    virtual void onServerListHasChanged();
private:
    PuyoMetaServerConnectionResponder *m_responder;
};

class DummyMetaServerConnection : public AbstractPuyoMetaServerConnection {
public:
    DummyMetaServerConnection(PuyoMetaServerConnectionResponder *responder);
    virtual void fetch();
    virtual std::vector<PuyoServer> getServers() const;
};

class HttpMetaServerConnection : public AbstractPuyoMetaServerConnection, public IdleComponent {
public:
    HttpMetaServerConnection(String hostName, String hostPath, int portNum, PuyoMetaServerConnectionResponder *responder);
    virtual ~HttpMetaServerConnection();
    virtual void fetch();
    virtual std::vector<PuyoServer> getServers() const;
    // IdleComponent implementation
    virtual void idle(double currentTime);
    virtual bool failed() const { return m_nErrors > 5; }
private:
    HttpDocument *m_doc;
    String m_hostName, m_hostPath;
    int m_portNum;
    std::vector<PuyoServer> m_servers;
    int m_nErrors;
};



class PuyoServerList;

class PuyoServerListResponder {
public:
    virtual ~PuyoServerListResponder() {}
    virtual void PuyoServerListHasChanged(PuyoServerList &serverList) = 0;
};

class PuyoServerList : public PuyoMetaServerConnectionResponder, public PingablePuyoServerResponder {
public:
    PuyoServerList(PuyoServerListResponder *responder);
    virtual ~PuyoServerList();
    void fetch();
    std::vector<PuyoServer> getServers();
    // PuyoMetaServerConnectionResponder implementation
    virtual void PuyoMetaServerListHasChanged(AbstractPuyoMetaServerConnection &metaServerConnection);
    // PingablePuyoServerResponder implementation
    virtual void puyoServerDidPing(PingablePuyoServer &server);
private:
    std::vector<AbstractPuyoMetaServerConnection *> m_metaservers;
    std::vector<PingablePuyoServer *> m_servers;
    PuyoServerListResponder *m_responder;
	int fetching;
	bool firstTime;
};


class InternetGameMenu : public MainScreenMenu, public IdleComponent, public Action, public PuyoServerListResponder {
public:
    InternetGameMenu(MainScreen * mainScreen);
    virtual ~InternetGameMenu();
    void build();
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    void setSelectedServer(const String &s, int port);
    virtual void idle(double currentTime);
    virtual IdleComponent *getIdleComponent() { return this; }
    void enterNetCenterMenu(InternetGameCenter *gameCenter);
    // PuyoServerListResponder implementation
    virtual void PuyoServerListHasChanged(PuyoServerList &serverList);
private:
    Frame screenTitleFrame;
    Text internetGameText;
    PuyoServerList servers;
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
    PuyoPopMenuAction backAction;
    FramedButton joinButton, backButton;
};

#endif // _PUYOINTERNETMENU_H

