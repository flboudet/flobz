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
#include "PuyoCommander.h"
#include "PuyoInternetGameCenter.h"
#include "PuyoTwoPlayerStarter.h"
#include "PuyoChatBox.h"

class NetCenterMenu;

class NetCenterDialogMenu : public SliderContainer {
public:
    NetCenterDialogMenu(NetCenterMenu *targetMenu, PuyoGameInvitation &associatedInvitation, String title, String message, bool hasAcceptButton);
    virtual ~NetCenterDialogMenu();
    void build();
public:
    PuyoGameInvitation associatedInvitation;
private:
    IIM_Surface   *menuBG;
    class NetCenterDialogMenuAction : public Action {
    public:
        NetCenterDialogMenuAction(NetCenterMenu *targetMenu, bool isCancelAction)
        : targetMenu(targetMenu), isCancelAction(isCancelAction) {}
        void action();
    private:
        NetCenterMenu *targetMenu;
        bool isCancelAction;
    };
    NetCenterDialogMenuAction cancelAction, acceptAction;
    bool hasAcceptButton;
    VBox menu;
    HBox buttons;
    Text dialogTitle, dialogMsg;
    Text sep1, sep2;
    Button acceptButton, cancelButton;
};

class NetCenterPlayerList : public ListView {
public:
    NetCenterPlayerList(int size, NetCenterMenu *targetMenu, GameLoop *loop = NULL);
    void addNewPlayer(String playerName, PeerAddress playerAddress, int status);
    void removePlayer(PeerAddress playerAddress);
    void updatePlayer(String playerName, PeerAddress playerAddress, int status);
private:
    class PlayerSelectedAction : public Action {
    public:
        PlayerSelectedAction(NetCenterMenu *targetMenu, PeerAddress address, String playerName)
        : targetMenu(targetMenu), address(address), playerName(playerName) {}
        void action();
    private:
        NetCenterMenu *targetMenu;
        PeerAddress address;
        String playerName;
    };
    class PlayerEntry : public ListViewEntry {
    public:
        PlayerEntry(String playerName, PeerAddress playerAddress, int status, Action *action)
        : ListViewEntry(playerName + getStatusString(status), action),
	  playerAddress(playerAddress), status(status), action(action) {}
        ~PlayerEntry() { delete action; }
	void updateEntry(String playerName, int status) {
	  setText(playerName + getStatusString(status));
	  this->status = status;
	}
        PeerAddress playerAddress;
	int status;
    private:
        Action *action;
	static String getStatusString(int status);
    };
    Vector<PlayerEntry> entries;
    NetCenterMenu *targetMenu;
};

class NetCenterMenu;

class NetCenterTwoNameProvider : public PuyoTwoNameProvider {
public:
    NetCenterTwoNameProvider(PuyoNetGameCenter &netCenter) : netCenter(netCenter) {}
    String getPlayer1Name() const;
    String getPlayer2Name() const;
private:
    PuyoNetGameCenter &netCenter;
};

class NetCenterMenu : public PuyoMainScreenMenu, PuyoNetGameCenterListener, ChatBoxDelegate {
public:
    NetCenterMenu(PuyoMainScreen *mainScreen, PuyoNetGameCenter *netCenter, GameLoop *loop = NULL);
    ~NetCenterMenu();
    virtual void build();
    void onChatMessage(const String &msgAuthor, const String &msg);
    void onPlayerConnect(String playerName, PeerAddress playerAddress);
    void onPlayerDisconnect(String playerName, PeerAddress playerAddress);
    void onPlayerUpdated(String playerName, PeerAddress playerAddress);
    void onGameInvitationReceived(PuyoGameInvitation &invitation);
    void onGameInvitationCanceledReceived(PuyoGameInvitation &invitation);
    void onGameGrantedWithMessagebox(MessageBox *mbox, PuyoGameInvitation &invitation);
    void grantCurrentGame();
    void cancelCurrentGame();
    void cycle();
    void playerSelected(PeerAddress playerAddress, String playerName);
    void selfDestroy() { shouldSelfDestroy = true; }
    void show();
    virtual void sendChat(String chatText);
    // Notification
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
    HBox topbox;
    VBox menu;
    VBox playerbox;
    Text playerListText, chatAreaText, title;
    Button cancelButton;
    PuyoPopMenuAction backAction;
    NetCenterPlayerList playerList;
    NetCenterCycled cycled;
    PuyoNetGameCenter *netCenter;
    ZBox container;
    NetCenterDialogMenu *onScreenDialog;
    bool shouldSelfDestroy;
    NetCenterTwoNameProvider nameProvider;
    ChatBox chatBox;
    Separator topSeparator, middleSeparator, bottomSeparator;
};
    
#endif // _PUYONETCENTERMENU

