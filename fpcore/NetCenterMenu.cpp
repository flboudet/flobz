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

#include "NetCenterMenu.h"
#include "PuyoTwoPlayerStarter.h"
#include "PuyoNetworkStarter.h"
#include "ios_time.h"
#include <sstream>

using namespace event_manager;

class PuyoNetworkTwoPlayerGameWidgetFactory : public GameWidgetFactory {
public:
    PuyoNetworkTwoPlayerGameWidgetFactory(ios_fc::MessageBox &mbox, unsigned int randomSeed, FPServerIGPMessageBox *igpbox/* = NULL */)
      : mbox(mbox), randomSeed(randomSeed), igpbox(igpbox), gameId(0) {}
    GameWidget *createGameWidget(FloboSetTheme &puyoThemeSet, LevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        PuyoNetworkGameWidget *negawi = new PuyoNetworkGameWidget();
        negawi->initWithGUI(puyoThemeSet, levelTheme, mbox, gameId++, randomSeed++, gameOverAction, igpbox);
        return negawi;
    }
private:
    ios_fc::MessageBox &mbox;
    unsigned int randomSeed;
    FPServerIGPMessageBox *igpbox;
    int gameId;
};

void NetCenterDialogMenu::NetCenterDialogMenuAction::action()
{
    if (! isCancelAction)
        targetMenu->grantCurrentGame();
    else targetMenu->cancelCurrentGame();
}

NetCenterDialogMenu::NetCenterDialogMenu(NetCenterMenu *targetMenu, FloboGameInvitation &associatedInvitation, String title, String message, String optLine, bool hasAcceptButton, bool hasCancelButton)
    : associatedInvitation(associatedInvitation),
      menu(theCommander->getWindowFramePicture()),
      cancelAction(targetMenu, true), acceptAction(targetMenu, false),
      hasAcceptButton(hasAcceptButton), hasCancelButton(hasCancelButton),
      titleFrame(theCommander->getSeparatorFramePicture()),
      dialogTitle(title), dialogMsg(message), optMsg(NULL),
      acceptButton(theCommander->getLocalizedString("Accept"), &acceptAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
      cancelButton(theCommander->getLocalizedString("Cancel"), &cancelAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture())
{
    if (optLine != "") optMsg = new Text(optLine);
}

NetCenterDialogMenu::~NetCenterDialogMenu()
{
    if (optMsg != NULL) delete optMsg;
}

void NetCenterDialogMenu::build()
{
    Vec3 dialogPos = getPosition();
    dialogPos.x = 50;
    dialogPos.y = 195;
    setPosition(dialogPos);
    setSize(Vec3(350., 200.));
    menu.setPolicy(USE_MIN_SIZE);
    titleFrame.setPreferedSize(Vec3(0, 20));
    titleFrame.add(&dialogTitle);
    menu.add(&titleFrame);
    menu.add(&sep1);
    menu.add(&dialogMsg);
    if (optMsg != NULL) {
        menu.add(&sep2);
        menu.add(optMsg);
    }
    if (hasAcceptButton)
        buttons.add(&acceptButton);
    if (hasCancelButton)
        buttons.add(&cancelButton);
    menu.add(&buttons);
    getParentScreen()->grabEventsOnWidget(this);
    transitionToContent(&menu);
}

void NetCenterDialogMenu::eventOccured(GameControlEvent *event)
{
    SliderContainer::eventOccured(event);
    // We intercept the back event so it will do the same as the cancel action
    switch (event->cursorEvent) {
        case kBack:
            event->caught = true;
            cancelAction.action();
            break;
        default:
            break;
    }
}

String NetCenterPlayerList::PlayerEntry::getRankString(int rank)
{
    std::stringstream s;
    if (rank < 0)
        s << "[?]";
    else if (rank < 30)
        s << "[" << (30-rank) << "k]";
    else
        s << "[" << (rank-29) << "d]";
    return String(s.str().c_str());
}

String NetCenterPlayerList::PlayerEntry::getStatusString(int status)
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
      targetMenu(targetMenu)
{}

NetCenterPlayerList::~NetCenterPlayerList()
{
}

void NetCenterPlayerList::addNewPlayer(String playerName, PeerAddress playerAddress, const PeerInfo &info)
{
    Action *playerSelectedAction = new PlayerSelectedAction(targetMenu, playerAddress, playerName);
    PlayerEntry *newEntry = new PlayerEntry(playerName, playerAddress, info, playerSelectedAction);
    if (info.self)
        newEntry->setEnabled(false);
    entries.add(newEntry);
    addEntry(newEntry);
}

void NetCenterPlayerList::removePlayer(PeerAddress playerAddress)
{
    for (int i = 0 ; i < entries.size() ; i++) {
        if (entries[i]->playerAddress == playerAddress) {
            PlayerEntry *currentEntry = entries[i];
            removeEntry(currentEntry);
            entries.removeAt(i);
            delete currentEntry;
            return;
        }
    }
}

void NetCenterPlayerList::updatePlayer(String playerName, PeerAddress playerAddress, const PeerInfo &info)
{
    for (int i = 0 ; i < entries.size() ; i++) {
        if (entries[i]->playerAddress == playerAddress) {
            PlayerEntry *currentEntry = entries[i];
            currentEntry->updateEntry(playerName, info);
            return;
        }
    }
}

void NetCenterPlayerList::PlayerSelectedAction::action()
{
    targetMenu->playerSelected(address, playerName);
}

class SayAction : public Action {
public:
    SayAction(NetGameCenter *netCenter, Text *message) : netCenter(netCenter), message(message) {}
    void action() {
      netCenter->sendMessage(message->getValue());
    }
private:
    NetGameCenter *netCenter;
    Text *message;
};

String NetCenterTwoNameProvider::getPlayerName(int playerNumber) const
{
    switch (playerNumber) {
    case 0:
        return netCenter.getSelfName();
    case 1:
    default:
	return netCenter.getOpponentName();
    }
}

NetCenterMenu::NetCenterMenu(MainScreen *mainScreen, NetGameCenter *netCenter,
                             String title, GameLoop *loop)
    : MainScreenMenu(mainScreen, loop),
      topFrame(theCommander->getWindowFramePicture()),
      titleFrame(theCommander->getSeparatorFramePicture()),
      title(title),
      playerListText(theCommander->getLocalizedString("Player List")),
      chatAreaText(theCommander->getLocalizedString("Chat Area")),
      cancelButton(theCommander->getLocalizedString("Disconnect"), &backAction,
		   theCommander->getButtonFramePicture(), theCommander->getButtonOverFramePicture()),
      backAction(mainScreen),
      playerList(5, this, theCommander->getUpArrow(), theCommander->getDownArrow()), cycled(this),
      netCenter(netCenter), onScreenDialog(NULL),
      shouldSelfDestroy(false), nameProvider(*netCenter),
      chatBox(*this),
      topSeparator(0, 5), middleSeparator(0, 5), bottomSeparator(0, 5),
      m_speedSelector(1, theCommander->getRadioOnPicture(), theCommander->getRadioOffPicture(), "Config.TwoPlayerGameDifficulty", theCommander->getPreferencesManager())
{
    GameUIDefaults::GAME_LOOP->addIdle(&cycled);
    this->setBorderVisible(false);
    netCenter->addListener(this);
    // Adding all the already connected peers to the list
    for (int i = 0 ; i < netCenter->getPeerCount() ; i++) {
        PeerAddress curPeerAddress = netCenter->getPeerAddressAtIndex(i);
        PeerInfo curPeerInfo = netCenter->getPeerInfoForAddress(curPeerAddress);
        playerList.addNewPlayer(netCenter->getPeerNameAtIndex(i), curPeerAddress, curPeerInfo);
    }
}

NetCenterMenu::~NetCenterMenu()
{
#ifdef DEBUG
    printf("Deleting the net center\n");
#endif
    // Delete the network center because no one else would do it
    delete netCenter;
}

void NetCenterMenu::cycle()
{
    netCenter->idle();
}

void NetCenterMenu::build()
{
    add(&container);

    menu.setPolicy(USE_MAX_SIZE);
    mainBox.setPolicy(USE_MAX_SIZE);
    topbox.setPolicy(USE_MAX_SIZE);
    playerbox.setPolicy(USE_MAX_SIZE);

    container.add(&mainBox);

    m_speedSelector.addButton(theCommander->getLocalizedString("Beginner"));
    m_speedSelector.addButton(theCommander->getLocalizedString("Normal"));
    m_speedSelector.addButton(theCommander->getLocalizedString("Expert"));
    menu.add(&m_speedSelector);
    menu.add(&cancelButton);

    playerbox.add(&playerListText);
    playerbox.add(&playerList);

    topbox.setInnerMargin(10.);
    topbox.add(&playerbox);
    topbox.add(&menu);

    titleFrame.setPreferedSize(Vec3(0, 20));
    titleFrame.add(&title);

    topFrame.setPreferedSize(Vec3(0, 240));
    topFrame.add(&titleFrame);
    topFrame.add(&topbox);

    mainBox.add(&topSeparator);
    mainBox.add(&topFrame);
    mainBox.add(&middleSeparator);
    mainBox.add(&chatBox);
}

void NetCenterMenu::sendChat(String chatText)
{
    netCenter->sendMessage(chatText);
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
#ifdef DEBUG
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
#endif
    chatBox.addChat(msgAuthor, msg);
}

void NetCenterMenu::onPlayerConnect(String playerName, PeerAddress playerAddress)
{
    PeerInfo info = netCenter->getPeerInfoForAddress(playerAddress);
    playerList.addNewPlayer(playerName, playerAddress, info);
}

void NetCenterMenu::onPlayerDisconnect(String playerName, PeerAddress playerAddress)
{
    playerList.removePlayer(playerAddress);
}

void NetCenterMenu::onPlayerUpdated(String playerName, PeerAddress playerAddress)
{
    PeerInfo info = netCenter->getPeerInfoForAddress(playerAddress);
    playerList.updatePlayer(playerName, playerAddress, info);
}

void NetCenterMenu::onGameInvitationReceived(FloboGameInvitation &invitation)
{
    // If already waiting for a game, cancel the invitation
    if (this->onScreenDialog != NULL) {
        netCenter->cancelGameInvitation(invitation);
    }
    else {
        String levelSpeed = theCommander->getLocalizedString("Game speed:");
        levelSpeed += " ";
        switch (invitation.gameSpeed) {
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
        onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Invitation for a game"),
                                                 invitation.opponentName + theCommander->getLocalizedString(" invited you to play"),
                                                 levelSpeed,
                                                 true);
        container.add(onScreenDialog);
        onScreenDialog->build();
        this->focus(onScreenDialog);
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
        netCenter->setStatus(PEER_NORMAL);
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
    if (this->onScreenDialog != NULL) {
        netCenter->acceptGameInvitation(onScreenDialog->associatedInvitation);
    }
}

void NetCenterMenu::cancelCurrentGame()
{
    if (this->onScreenDialog != NULL) {
        netCenter->cancelGameInvitation(onScreenDialog->associatedInvitation);
    }
}

void NetCenterMenu::onGameAcceptedNegociationPending(FloboGameInvitation &invitation)
{
    container.remove(onScreenDialog);
    delete onScreenDialog;
    onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Negociating game"),
                                             theCommander->getLocalizedString("A game is being prepared with"),
                                             invitation.opponentName, false, false);
    container.add(onScreenDialog);
    onScreenDialog->build();
    this->focus(onScreenDialog);
}

void NetCenterMenu::onGameInvitationCanceledReceived(FloboGameInvitation &invitation)
{
    if (this->onScreenDialog != NULL) {
        if (invitation.opponentAddress == onScreenDialog->associatedInvitation.opponentAddress) {
            container.remove(onScreenDialog);
            delete onScreenDialog;
            onScreenDialog = NULL;
        }
    }
}

void NetCenterMenu::onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation)
{
    PuyoNetworkTwoPlayerGameWidgetFactory *factory = new PuyoNetworkTwoPlayerGameWidgetFactory(*mbox, invitation.gameRandomSeed, netCenter->getIgpBox());
    NetworkGameStateMachine *starter = new NetworkGameStateMachine(factory, mbox, (GameDifficulty)(invitation.gameSpeed), &nameProvider);
    starter->evaluate();

    if (this->onScreenDialog != NULL) {
        container.remove(onScreenDialog);
        delete(onScreenDialog);
        onScreenDialog = NULL;
    }
}

void NetCenterMenu::playerSelected(PeerAddress playerAddress, String playerName)
{
    FloboGameInvitation invitation;
    invitation.gameRandomSeed = (unsigned long)(fmod(getTimeMs(), (double)0xFFFFFFFF));
    invitation.opponentAddress = playerAddress;
    invitation.gameSpeed = m_speedSelector.getState() - 1;
    onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Asking for a game"), String(theCommander->getLocalizedString("Waiting ")) + playerName + theCommander->getLocalizedString(" for confirmation"), String(""), false);
    container.add(onScreenDialog);
    onScreenDialog->build();
    this->focus(onScreenDialog);

    netCenter->requestGame(invitation);
}


