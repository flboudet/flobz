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

#ifndef _PUYONETCENTERMENU
#define _PUYONETCENTERMENU

#include "gameui.h"
#include "ListView.h"
#include "Frame.h"
#include "FramedButton.h"
#include "MainScreen.h"
#include "PuyoCommander.h"
#include "InternetGameCenter.h"
#include "PuyoTwoPlayerStarter.h"
#include "ChatBox.h"
#include "RadioButton.h"

class NetCenterMenu;

class NetCenterDialogMenu : public SliderContainer {
public:
    NetCenterDialogMenu(NetCenterMenu *targetMenu, FloboGameInvitation &associatedInvitation, String title, String message, String optLine, bool hasAcceptButton, bool hasCancelButton=true);
    virtual ~NetCenterDialogMenu();
    void build();
    // Notification
    virtual void eventOccured(event_manager::GameControlEvent *event);
public:
    FloboGameInvitation associatedInvitation;
private:
    class NetCenterDialogMenuAction : public Action {
    public:
        NetCenterDialogMenuAction(NetCenterMenu *targetMenu, bool isCancelAction)
        : targetMenu(targetMenu), isCancelAction(isCancelAction) {}
        void action();
    private:
        NetCenterMenu *targetMenu;
        bool isCancelAction;
    };
    Frame menu;
    NetCenterDialogMenuAction cancelAction, acceptAction;
    bool hasAcceptButton, hasCancelButton;
    HBox buttons;
    Frame titleFrame;
    Text dialogTitle, dialogMsg, * optMsg;
    Text sep1, sep2;
    FramedButton acceptButton, cancelButton;
};

class NetCenterPlayerList : public ListView {
public:
    NetCenterPlayerList(int size, NetCenterMenu *targetMenu, IosSurface *upArrow, IosSurface *downArrow, GameLoop *loop = NULL);
    virtual ~NetCenterPlayerList();
    void addNewPlayer(String playerName, PeerAddress playerAddress, const PeerInfo &info);
    void removePlayer(PeerAddress playerAddress);
    void updatePlayer(String playerName, PeerAddress playerAddress, const PeerInfo &info);
private:
    class PlayerSelectedAction : public Action {
    public:
        PlayerSelectedAction(NetCenterMenu *targetMenu, PeerAddress address, String playerName)
        : targetMenu(targetMenu), address(address), playerName(playerName) {}
        void action();
    private:
        PeerAddress address;
        NetCenterMenu *targetMenu;
        String playerName;
    };
    class PlayerEntry : public ListViewEntry {
    public:
        PlayerEntry(String playerName, PeerAddress playerAddress, const PeerInfo &info, Action *action)
        : ListViewEntry(getRankString(info.rank) + playerName + getStatusString(info.status), action),
        playerAddress(playerAddress), status(info.status), rank(info.rank), action(action) {}
        ~PlayerEntry() { delete action; }
        void updateEntry(String playerName, const PeerInfo &info) {
            setText(getRankString(info.rank) + playerName + getStatusString(info.status));
            this->status = status;
            this->rank = rank;
        }
        PeerAddress playerAddress;
        int status;
        int rank;
    private:
        Action *action;
        static String getStatusString(int status);
        static String getRankString(int rank);
    };
    Vector<PlayerEntry> entries;
    NetCenterMenu *targetMenu;
};

class NetCenterMenu;

class NetCenterTwoNameProvider : public PlayerNameProvider {
public:
    NetCenterTwoNameProvider(NetGameCenter &netCenter) : netCenter(netCenter) {}
    String getPlayerName(int playerNumber) const;
private:
    NetGameCenter &netCenter;
};

class NetCenterMenu : public MainScreenMenu, NetGameCenterListener, ChatBoxDelegate {
public:
    NetCenterMenu(MainScreen *mainScreen, NetGameCenter *netCenter,
                  String title, GameLoop *loop = NULL);
    ~NetCenterMenu();
    virtual void build();
    void onChatMessage(const String &msgAuthor, const String &msg);
    void onPlayerConnect(String playerName, PeerAddress playerAddress);
    void onPlayerDisconnect(String playerName, PeerAddress playerAddress);
    void onPlayerUpdated(String playerName, PeerAddress playerAddress);
    void onGameInvitationReceived(FloboGameInvitation &invitation);
    void onGameInvitationCanceledReceived(FloboGameInvitation &invitation);
    void onGameAcceptedNegociationPending(FloboGameInvitation &invitation);
    void onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation);
    void grantCurrentGame();
    void cancelCurrentGame();
    void cycle();
    void playerSelected(PeerAddress playerAddress, String playerName);
    void selfDestroy() { shouldSelfDestroy = true; }
    virtual void sendChat(String chatText);
    // Notification
    virtual void eventOccured(event_manager::GameControlEvent *event);
    virtual void onWidgetVisibleChanged(bool visible);
    virtual void onWidgetRemoved(WidgetContainer *parent);
private:
    class NetCenterCycled : public CycledComponent {
    public:
        NetCenterMenu *netCenter;
        NetCenterCycled(NetCenterMenu *netCenter) : CycledComponent(0.02), netCenter(netCenter) {}
        void cycle() {
            netCenter->cycle();
        }
    };
    VBox mainBox;
    Frame topFrame;
    Frame titleFrame;
    Text title;
    HBox topbox;
    VBox menu;
    VBox playerbox;
    Text playerListText, chatAreaText;
    FramedButton cancelButton;
    PuyoPopMenuAction backAction;
    NetCenterPlayerList playerList;
    NetCenterCycled cycled;
    NetGameCenter *netCenter;
    ZBox container;
    NetCenterDialogMenu *onScreenDialog;
    bool shouldSelfDestroy;
    NetCenterTwoNameProvider nameProvider;
    ChatBox chatBox;
    Separator topSeparator, middleSeparator, bottomSeparator;
    RadioButton m_speedSelector;
};

#endif // _PUYONETCENTERMENU

