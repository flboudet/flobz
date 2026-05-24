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

#include "NetCenterMenu.h"
#include "TwoPlayersGameStarter.h"
#include "NetworkGameStarter.h"
#include "ios_time.h"
#include <sstream>

using namespace event_manager;

class NetworkTwoPlayerGameWidgetFactory : public GameWidgetFactory {
public:
    NetworkTwoPlayerGameWidgetFactory(ios_fc::MessageBox &mbox, unsigned int randomSeed, FPServerIGPMessageBox *igpbox/* = NULL */)
      : _mbox(mbox), _randomSeed(randomSeed), _igpbox(igpbox), _gameId(0) {}
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, const std::string & centerFace, Action *gameOverAction)
    {
        NetworkGameWidget *negawi = new NetworkGameWidget();
        negawi->initWithGUI(floboSetTheme, levelTheme, _mbox, _gameId++, _randomSeed++, gameOverAction, _igpbox);
        return negawi;
    }
private:
    ios_fc::MessageBox &_mbox;
    unsigned int _randomSeed;
    FPServerIGPMessageBox *_igpbox;
    int _gameId;
};

void NetCenterDialogMenu::NetCenterDialogMenuAction::action()
{
    if (! _isCancelAction)
        _targetMenu->grantCurrentGame();
    else _targetMenu->cancelCurrentGame();
}

NetCenterDialogMenu::NetCenterDialogMenu(NetCenterMenu *targetMenu, FloboGameInvitation &associatedInvitation,
                       const std::string &title, const std::string &message, const std::string &optLine,
                       bool hasAcceptButton, bool hasCancelButton)
    : _associatedInvitation(associatedInvitation),
      _menu(theCommander->getWindowFramePicture()),
      _cancelAction(targetMenu, true), _acceptAction(targetMenu, false),
      _hasAcceptButton(hasAcceptButton), _hasCancelButton(hasCancelButton),
      _titleFrame(theCommander->getSeparatorFramePicture()),
      _dialogTitle(title), _dialogMsg(message), _optMsg(NULL),
      _acceptButton(theCommander->getLocalizedString("Accept"), &_acceptAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
      _cancelButton(theCommander->getLocalizedString("Cancel"), &_cancelAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture())
{
    if (optLine != "") _optMsg = new Text(optLine);
}

NetCenterDialogMenu::~NetCenterDialogMenu()
{
    if (_optMsg != NULL) delete _optMsg;
}

void NetCenterDialogMenu::build()
{
    Vec3 dialogPos = getPosition();
    dialogPos.x = 50;
    dialogPos.y = 195;
    setPosition(dialogPos);
    setSize(Vec3(350., 200.));
    _menu.setPolicy(USE_MIN_SIZE);
    _titleFrame.setPreferedSize(Vec3(0, 20));
    _titleFrame.add(&_dialogTitle);
    _menu.add(&_titleFrame);
    _menu.add(&_sep1);
    _menu.add(&_dialogMsg);
    if (_optMsg != NULL) {
        _menu.add(&_sep2);
        _menu.add(_optMsg);
    }
    if (_hasAcceptButton)
        _buttons.add(&_acceptButton);
    if (_hasCancelButton)
        _buttons.add(&_cancelButton);
    _menu.add(&_buttons);
    getParentScreen()->grabEventsOnWidget(this);
    transitionToContent(&_menu);
}

void NetCenterDialogMenu::eventOccured(GameControlEvent *event)
{
    SliderContainer::eventOccured(event);
    // We intercept the back event so it will do the same as the cancel action
    switch (event->cursorEvent) {
        case kBack:
            event->caught = true;
            _cancelAction.action();
            break;
        default:
            break;
    }
}

std::string NetCenterPlayerList::PlayerEntry::getRankString(int rank)
{
    std::stringstream s;
    if (rank < 0)
        s << "[?]";
    else if (rank < 30)
        s << "[" << (30-rank) << "k]";
    else
        s << "[" << (rank-29) << "d]";
    return s.str();
}

std::string NetCenterPlayerList::PlayerEntry::getStatusString(int status)
{
  switch (status) {
  case PEER_NORMAL:
    return "";
  case PEER_PLAYING:
    return theCommander->getLocalizedString(" (playing)");
  default:
    return theCommander->getLocalizedString(" (unknown)");
  }
}

NetCenterPlayerList::NetCenterPlayerList(int size, NetCenterMenu *targetMenu, IosSurface *upArrow, IosSurface *downArrow, GameLoop *loop)
    : ListView(size, upArrow, downArrow, theCommander->getListFramePicture(), loop),
      _targetMenu(targetMenu)
{}

NetCenterPlayerList::~NetCenterPlayerList()
{
}

void NetCenterPlayerList::addNewPlayer(const std::string & playerName, PeerAddress playerAddress, const PeerInfo &info)
{
    Action *playerSelectedAction = new PlayerSelectedAction(_targetMenu, playerAddress, playerName);
    PlayerEntry *newEntry = new PlayerEntry(playerName, playerAddress, info, playerSelectedAction);
    if (info._self)
        newEntry->setEnabled(false);
    _entries.add(newEntry);
    addEntry(newEntry);
}

void NetCenterPlayerList::removePlayer(PeerAddress playerAddress)
{
    for (int i = 0 ; i < _entries.size() ; i++) {
        if (_entries[i]->_playerAddress == playerAddress) {
            PlayerEntry *currentEntry = _entries[i];
            removeEntry(currentEntry);
            _entries.removeAt(i);
            delete currentEntry;
            return;
        }
    }
}

void NetCenterPlayerList::updatePlayer(const std::string & playerName, PeerAddress playerAddress, const PeerInfo &info)
{
    for (int i = 0 ; i < _entries.size() ; i++) {
        if (_entries[i]->_playerAddress == playerAddress) {
            PlayerEntry *currentEntry = _entries[i];
            currentEntry->updateEntry(playerName, info);
            return;
        }
    }
}

void NetCenterPlayerList::PlayerSelectedAction::action()
{
    _targetMenu->playerSelected(_address, _playerName);
}

class SayAction : public Action {
public:
    SayAction(NetGameCenter *netCenter, Text *message) : _netCenter(netCenter), _message(message) {}
    void action() {
      _netCenter->sendMessage(_message->getValue());
    }
private:
    NetGameCenter *_netCenter;
    Text *_message;
};

std::string NetCenterTwoNameProvider::getPlayerName(int playerNumber) const
{
    switch (playerNumber) {
    case 0:
        return _netCenter.getSelfName();
    case 1:
    default:
	return _netCenter.getOpponentName();
    }
}

NetCenterMenu::NetCenterMenu(MainScreen *mainScreen, NetGameCenter *netCenter,
                             const std::string &title, GameLoop *loop)
    : MainScreenMenu(mainScreen, loop),
      _topFrame(theCommander->getWindowFramePicture()),
      _titleFrame(theCommander->getSeparatorFramePicture()),
      _title(title),
      _playerListText(theCommander->getLocalizedString("Player List")),
      _chatAreaText(theCommander->getLocalizedString("Chat Area")),
      _cancelButton(theCommander->getLocalizedString("Disconnect"), &_backAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
      _backAction(mainScreen),
      _playerList(5, this, theCommander->getUpArrow(), theCommander->getDownArrow()), _cycled(this),
      _netCenter(netCenter), _onScreenDialog(NULL),
      _shouldSelfDestroy(false), _nameProvider(*netCenter),
      _chatBox(*this),
      _topSeparator(0, 5), _middleSeparator(0, 5), _bottomSeparator(0, 5),
      _speedSelector(1, theCommander->getRadioOnPicture(), theCommander->getRadioOffPicture(), "Config.TwoPlayerGameDifficulty", theCommander->getPreferencesManager())
{
    GameUIDefaults::GAME_LOOP->addIdle(&_cycled);
    this->setBorderVisible(false);
    _netCenter->addListener(this);
    // Adding all the already connected peers to the list
    for (int i = 0 ; i < _netCenter->getPeerCount() ; i++) {
        PeerAddress curPeerAddress = _netCenter->getPeerAddressAtIndex(i);
        PeerInfo curPeerInfo = _netCenter->getPeerInfoForAddress(curPeerAddress);
        _playerList.addNewPlayer(_netCenter->getPeerNameAtIndex(i), curPeerAddress, curPeerInfo);
    }
}

NetCenterMenu::~NetCenterMenu()
{
#ifdef DEBUG
    printf("Deleting the net center\n");
#endif
    // Delete the network center because no one else would do it
    delete _netCenter;
}

void NetCenterMenu::cycle()
{
    _netCenter->idle();
}

void NetCenterMenu::build()
{
    add(&_container);

    _menu.setPolicy(USE_MAX_SIZE);
    _mainBox.setPolicy(USE_MAX_SIZE);
    _topbox.setPolicy(USE_MAX_SIZE);
    _playerbox.setPolicy(USE_MAX_SIZE);

    _container.add(&_mainBox);

    _speedSelector.addButton(theCommander->getLocalizedString("Beginner"));
    _speedSelector.addButton(theCommander->getLocalizedString("Normal"));
    _speedSelector.addButton(theCommander->getLocalizedString("Expert"));
    _menu.add(&_speedSelector);
    _menu.add(&_cancelButton);

    _playerbox.add(&_playerListText);
    _playerbox.add(&_playerList);

    _topbox.setInnerMargin(10.);
    _topbox.add(&_playerbox);
    _topbox.add(&_menu);

    _titleFrame.setPreferedSize(Vec3(0, 20));
    _titleFrame.add(&_title);

    _topFrame.setPreferedSize(Vec3(0, 240));
    _topFrame.add(&_titleFrame);
    _topFrame.add(&_topbox);

    _mainBox.add(&_topSeparator);
    _mainBox.add(&_topFrame);
    _mainBox.add(&_middleSeparator);
    _mainBox.add(&_chatBox);
}

void NetCenterMenu::sendChat(const std::string &chatText)
{
    _netCenter->sendMessage(chatText);
}

void NetCenterMenu::onChatMessage(const std::string &msgAuthor, const std::string &msg)
{
#ifdef DEBUG
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
#endif
    _chatBox.addChat(msgAuthor, msg);
}

void NetCenterMenu::onPlayerConnect(const std::string &playerName, PeerAddress playerAddress)
{
    PeerInfo info = _netCenter->getPeerInfoForAddress(playerAddress);
    _playerList.addNewPlayer(playerName, playerAddress, info);
}

void NetCenterMenu::onPlayerDisconnect(const std::string &playerName, PeerAddress playerAddress)
{
    _playerList.removePlayer(playerAddress);
}

void NetCenterMenu::onPlayerUpdated(const std::string &playerName, PeerAddress playerAddress)
{
    PeerInfo info = _netCenter->getPeerInfoForAddress(playerAddress);
    _playerList.updatePlayer(playerName, playerAddress, info);
}

void NetCenterMenu::onGameInvitationReceived(FloboGameInvitation &invitation)
{
    // If already waiting for a game, cancel the invitation
    if (this->_onScreenDialog != NULL) {
        _netCenter->cancelGameInvitation(invitation);
    }
    else {
        std::string levelSpeed = theCommander->getLocalizedString("Game speed:");
        levelSpeed += " ";
        switch (invitation._gameSpeed) {
            default:
            case 0:
                levelSpeed += theCommander->getLocalizedString("Beginner");
                break;
            case 1:
                levelSpeed += theCommander->getLocalizedString("Normal");
                break;
            case 2:
                levelSpeed += theCommander->getLocalizedString("Expert");
                break;
        }
        _onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Invitation for a game").c_str(),
                                                 invitation._opponentName + theCommander->getLocalizedString(" invited you to play").c_str(),
                                                 levelSpeed.c_str(),
                                                 true);
        _container.add(_onScreenDialog);
        _onScreenDialog->build();
        this->focus(_onScreenDialog);
    }
}

void NetCenterMenu::eventOccured(GameControlEvent *event)
{
    MainScreenMenu::eventOccured(event);
    // We intercept the back event so it will be impossible
    // to go to the previous menu by hitting the back button.
    // (otherwise, it's too easy to disconnect by mistake)
    switch (event->cursorEvent) {
        case kBack:
            event->caught = true;
            break;
    default:
      break;
    }
}

void NetCenterMenu::onWidgetVisibleChanged(bool visible)
{
#ifdef DEBUG
    printf("netcentermenu visible: %s\n", visible ? "true" : "false");
#endif
    if (visible)
        _netCenter->setStatus(PEER_NORMAL);
}

void NetCenterMenu::onWidgetRemoved(WidgetContainer *parent)
{
#ifdef DEBUG
     printf("netcentermenu removed\n");
#endif
    delete this;
}

void NetCenterMenu::grantCurrentGame()
{
    if (this->_onScreenDialog != NULL) {
        _netCenter->acceptGameInvitation(_onScreenDialog->_associatedInvitation);
    }
}

void NetCenterMenu::cancelCurrentGame()
{
    if (this->_onScreenDialog != NULL) {
        _netCenter->cancelGameInvitation(_onScreenDialog->_associatedInvitation);
    }
}

void NetCenterMenu::onGameAcceptedNegociationPending(FloboGameInvitation &invitation)
{
    _container.remove(_onScreenDialog);
    delete _onScreenDialog;
    _onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Negociating game"),
                                             theCommander->getLocalizedString("A game is being prepared with"),
                                             invitation._opponentName.c_str(), false, false);
    _container.add(_onScreenDialog);
    _onScreenDialog->build();
    this->focus(_onScreenDialog);
}

void NetCenterMenu::onGameInvitationCanceledReceived(FloboGameInvitation &invitation)
{
    if (this->_onScreenDialog != NULL) {
        if (invitation._opponentAddress == _onScreenDialog->_associatedInvitation._opponentAddress) {
            _container.remove(_onScreenDialog);
            delete _onScreenDialog;
            _onScreenDialog = NULL;
        }
    }
}

void NetCenterMenu::onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation)
{
    NetworkTwoPlayerGameWidgetFactory *factory = new NetworkTwoPlayerGameWidgetFactory(*mbox, invitation._gameRandomSeed, _netCenter->getIgpBox());
    NetworkGameStateMachine *starter = new NetworkGameStateMachine(factory, mbox, (GameDifficulty)(invitation._gameSpeed), &_nameProvider);
    starter->evaluate();

    if (this->_onScreenDialog != NULL) {
        _container.remove(_onScreenDialog);
        delete(_onScreenDialog);
        _onScreenDialog = NULL;
    }
}

void NetCenterMenu::playerSelected(PeerAddress playerAddress, const std::string &playerName)
{
    FloboGameInvitation invitation;
    invitation._gameRandomSeed = (unsigned long)(fmod(getTimeMs(), (double)0xFFFFFFFF));
    invitation._opponentAddress = playerAddress;
    invitation._gameSpeed = _speedSelector.getState() - 1;
    invitation._gameNbSets = 0; // TODO: allow multisets games
    _onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Asking for a game"), (theCommander->getLocalizedString("Waiting ") + playerName.c_str() + theCommander->getLocalizedString(" for confirmation")).c_str(), "", false); // TODO string
    _container.add(_onScreenDialog);
    _onScreenDialog->build();
    this->focus(_onScreenDialog);

    _netCenter->requestGame(invitation);
}


