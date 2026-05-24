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

#ifndef _PUYONETCENTERMENU
#define _PUYONETCENTERMENU

#include "gameui.h"
#include "ListView.h"
#include "Frame.h"
#include "FramedButton.h"
#include "MainScreen.h"
#include "FPCommander.h"
#include "InternetGameCenter.h"
#include "TwoPlayersGameStarter.h"
#include "ChatBox.h"
#include "RadioButton.h"

class NetCenterMenu;

class NetCenterDialogMenu : public SliderContainer {
public:
    NetCenterDialogMenu(NetCenterMenu *targetMenu, FloboGameInvitation &associatedInvitation, const std::string &title, const std::string &message, const std::string &optLine, bool hasAcceptButton, bool hasCancelButton=true);
    virtual ~NetCenterDialogMenu();
    void build();
    // Notification
    virtual void eventOccured(event_manager::GameControlEvent *event);
public:
    FloboGameInvitation _associatedInvitation;
private:
    class NetCenterDialogMenuAction : public Action {
    public:
        NetCenterDialogMenuAction(NetCenterMenu *targetMenu, bool isCancelAction)
        : _targetMenu(targetMenu), _isCancelAction(isCancelAction) {}
        void action();
    private:
        NetCenterMenu *_targetMenu;
        bool _isCancelAction;
    };
    Frame _menu;
    NetCenterDialogMenuAction _cancelAction, _acceptAction;
    bool _hasAcceptButton, _hasCancelButton;
    HBox _buttons;
    Frame _titleFrame;
    Text _dialogTitle, _dialogMsg, * _optMsg;
    Text _sep1, _sep2;
    FramedButton _acceptButton, _cancelButton;
};

class NetCenterPlayerList : public ListView {
public:
    NetCenterPlayerList(int size, NetCenterMenu *targetMenu, IosSurface *upArrow, IosSurface *downArrow, GameLoop *loop = NULL);
    virtual ~NetCenterPlayerList();
    void addNewPlayer(const std::string & playerName, PeerAddress playerAddress, const PeerInfo &info);
    void removePlayer(PeerAddress playerAddress);
    void updatePlayer(const std::string & playerName, PeerAddress playerAddress, const PeerInfo &info);
private:
    class PlayerSelectedAction : public Action {
    public:
        PlayerSelectedAction(NetCenterMenu *targetMenu, PeerAddress address, const std::string &playerName)
        : _address(address), _targetMenu(targetMenu), _playerName(playerName) {}
        void action();
    private:
        PeerAddress _address;
        NetCenterMenu *_targetMenu;
        std::string _playerName;
    };
    class PlayerEntry : public ListViewEntry {
    public:
        PlayerEntry(const std::string &playerName, PeerAddress playerAddress, const PeerInfo &info, Action *action)
        : ListViewEntry(getRankString(info._rank) + playerName + getStatusString(info._status), action),
        _playerAddress(playerAddress), _status(info._status), _rank(info._rank), _action(action) {}
        ~PlayerEntry() { delete _action; }
        void updateEntry(const std::string &playerName, const PeerInfo &info) {
            setText(getRankString(info._rank) + playerName + getStatusString(info._status));
            this->_status = _status;
            this->_rank = _rank;
        }
        PeerAddress _playerAddress;
        int _status;
        int _rank;
    private:
        Action *_action;
        static std::string getStatusString(int status);
        static std::string getRankString(int rank);
    };
    Vector<PlayerEntry> _entries;
    NetCenterMenu *_targetMenu;
};

class NetCenterMenu;

class NetCenterTwoNameProvider : public PlayerNameProvider {
public:
    NetCenterTwoNameProvider(NetGameCenter &netCenter) : _netCenter(netCenter) {}
    std::string getPlayerName(int playerNumber) const;
private:
    NetGameCenter &_netCenter;
};

class NetCenterMenu : public MainScreenMenu, NetGameCenterListener, ChatBoxDelegate {
public:
    NetCenterMenu(MainScreen *mainScreen, NetGameCenter *netCenter,
                  const std::string &title, GameLoop *loop = NULL);
    ~NetCenterMenu();
    virtual void build();
    void onChatMessage(const std::string &msgAuthor, const std::string &msg);
    void onPlayerConnect(const std::string &playerName, PeerAddress playerAddress);
    void onPlayerDisconnect(const std::string &playerName, PeerAddress playerAddress);
    void onPlayerUpdated(const std::string &playerName, PeerAddress playerAddress);
    void onGameInvitationReceived(FloboGameInvitation &invitation);
    void onGameInvitationCanceledReceived(FloboGameInvitation &invitation);
    void onGameAcceptedNegociationPending(FloboGameInvitation &invitation);
    void onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation);
    void grantCurrentGame();
    void cancelCurrentGame();
    void cycle();
    void playerSelected(PeerAddress playerAddress, const std::string &playerName);
    void selfDestroy() { _shouldSelfDestroy = true; }
    virtual void sendChat(const std::string &chatText);
    // Notification
    virtual void eventOccured(event_manager::GameControlEvent *event);
    virtual void onWidgetVisibleChanged(bool visible);
    virtual void onWidgetRemoved(WidgetContainer *parent);
private:
    class NetCenterCycled : public CycledComponent {
    public:
        NetCenterMenu *_netCenter;
        NetCenterCycled(NetCenterMenu *netCenter) : CycledComponent(0.02), _netCenter(netCenter) {}
        void cycle() {
            _netCenter->cycle();
        }
    };
    VBox _mainBox;
    Frame _topFrame;
    Frame _titleFrame;
    Text _title;
    HBox _topbox;
    VBox _menu;
    VBox _playerbox;
    Text _playerListText, _chatAreaText;
    FramedButton _cancelButton;
    PopMainScreenMenuAction _backAction;
    NetCenterPlayerList _playerList;
    NetCenterCycled _cycled;
    NetGameCenter *_netCenter;
    ZBox _container;
    NetCenterDialogMenu *_onScreenDialog;
    bool _shouldSelfDestroy;
    NetCenterTwoNameProvider _nameProvider;
    ChatBox _chatBox;
    Separator _topSeparator, _middleSeparator, _bottomSeparator;
    RadioButton _speedSelector;
};

#endif // _PUYONETCENTERMENU

