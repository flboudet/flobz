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
#include "PuyoCommander.h"
#include "PuyoInternetGameCenter.h"

class NetCenterMenu;

class NetCenterDialogMenu : public SliderContainer {
public:
    NetCenterDialogMenu(PeerAddress associatedPeer, String title, String message, Action *cancelAction, Action *acceptAction = NULL);
    void build();
public:
    PeerAddress associatedPeer;
private:
    VBox menu;
    HBox buttons;
    Text dialogTitle, dialogMsg;
    Text sep1, sep2;
    Button acceptButton, cancelButton;
};

class NetCenterPlayerList : public ListWidget {
public:
    NetCenterPlayerList(int size, NetCenterMenu *targetMenu, GameLoop *loop = NULL) : ListWidget(size, loop), targetMenu(targetMenu) {}
    void addNewPlayer(String playerName, PeerAddress playerAddress);
    void removePlayer(PeerAddress playerAddress);
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
    class PlayerEntry : public Button {
    public:
        PlayerEntry(String playerName, PeerAddress playerAddress, Action *action)
        : Button(playerName, action), playerAddress(playerAddress), action(action) {}
        ~PlayerEntry() { delete action; }
        PeerAddress playerAddress;
    private:
        Action *action;
    };
    Vector<PlayerEntry> entries;
    NetCenterMenu *targetMenu;
};

class NetCenterChatArea : public VBox {
public:
    NetCenterChatArea(int height);
    ~NetCenterChatArea();
    void addChat(String name, String text);
private:
    int height;
    HBox **lines;
    Text **names;
    Text **texts;
};

class NetCenterMenu : public PuyoScreen, PuyoNetGameCenterListener {
public:
    NetCenterMenu(PuyoNetGameCenter *netCenter);
    void build();
    void onChatMessage(const String &msgAuthor, const String &msg);
    void onPlayerConnect(String playerName, PeerAddress playerAddress);
    void onPlayerDisconnect(String playerName, PeerAddress playerAddress);
    void gameInvitationAgainst(String playerName, PeerAddress playerAddress);
    void grantCurrentGame();
    void cancelCurrentGame();
    void gameCanceledAgainst(String playerName, PeerAddress playerAddress);
    void gameGrantedWithMessagebox(MessageBox *mbox);
    void cycle();
    void playerSelected(PeerAddress playerAddress, String playerName);
    void idle(double currentTime);
private:
    class NetCenterCycled : public CycledComponent {
    public:
        NetCenterMenu *netCenter;
        NetCenterCycled(NetCenterMenu *netCenter) : CycledComponent(0.02), netCenter(netCenter) {}
        void cycle() {
            netCenter->cycle();
        }
    };
    
    Text playerListText, chatAreaText;   
    NetCenterPlayerList playerList;
    NetCenterChatArea chatArea;
    NetCenterCycled *cycled;
    PuyoNetGameCenter *netCenter;
    PuyoStoryWidget story;
    SliderContainer container;
    NetCenterDialogMenu *onScreenDialog;
};
    
#endif // _PUYONETCENTERMENU

