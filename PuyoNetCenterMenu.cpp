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

#include "PuyoNetCenterMenu.h"
#include "PuyoTwoPlayerStarter.h"
#include "PuyoNetworkStarter.h"
#include "ios_time.h"

class PuyoNetworkTwoPlayerGameWidgetFactory : public PuyoGameWidgetFactory {
public:
    PuyoNetworkTwoPlayerGameWidgetFactory(ios_fc::MessageBox &mbox, unsigned int randomSeed) : mbox(mbox), randomSeed(randomSeed), gameId(0) {}
    PuyoGameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new PuyoNetworkGameWidget(puyoThemeSet, levelTheme, mbox, gameId++, randomSeed++, gameOverAction);
    }
private:
    ios_fc::MessageBox &mbox;
    unsigned int randomSeed;
    int gameId;
};

void NetCenterDialogMenu::NetCenterDialogMenuAction::action()
{
    if (! isCancelAction)
        targetMenu->grantCurrentGame();
    else targetMenu->cancelCurrentGame();
}

NetCenterDialogMenu::NetCenterDialogMenu(NetCenterMenu *targetMenu, PuyoGameInvitation &associatedInvitation, String title, String message, bool hasAcceptButton)
    : menuBG(IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/menubg.png"))),
      cancelAction(targetMenu, true), acceptAction(targetMenu, false), hasAcceptButton(hasAcceptButton),
      associatedInvitation(associatedInvitation), dialogTitle(title), dialogMsg(message),
      acceptButton(theCommander->getLocalizedString("Accept"), &acceptAction),
      cancelButton(theCommander->getLocalizedString("Cancel"), &cancelAction)
{}

NetCenterDialogMenu::~NetCenterDialogMenu()
{
  menu.remove(&dialogTitle);
  menu.remove(&sep1);
  menu.remove(&dialogMsg);
  menu.remove(&sep2);
  buttons.remove(&cancelButton);
  if (hasAcceptButton)
    buttons.remove(&acceptButton);
  menu.remove(&buttons);
  if (menuBG != NULL) {
    IIM_Free(menuBG);
  }
}

void NetCenterDialogMenu::build()
{
    Vec3 dialogPos = getPosition();
    dialogPos.x = 50;
    dialogPos.y = 195;
    setPosition(dialogPos);
    setSize(Vec3(menuBG->w, menuBG->h, 0));
    setBackground(menuBG);
    menu.add(&dialogTitle);
    menu.add(&sep1);
    menu.add(&dialogMsg);
    menu.add(&sep2);
    if (hasAcceptButton)
        buttons.add(&acceptButton);
    buttons.add(&cancelButton);
    menu.add(&buttons);
    getParentScreen()->grabEventsOnWidget(this);
    transitionToContent(&menu);
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

NetCenterPlayerList::NetCenterPlayerList(int size, NetCenterMenu *targetMenu, GameLoop *loop)
    : ListView(size, IIM_Load_Absolute_DisplayFormat(theCommander->getDataPathManager().getPath("gfx/downarrow.png")), loop), targetMenu(targetMenu)
{}

void NetCenterPlayerList::addNewPlayer(String playerName, PeerAddress playerAddress, int status)
{
    Action *playerSelectedAction = new PlayerSelectedAction(targetMenu, playerAddress, playerName);
    PlayerEntry *newEntry = new PlayerEntry(playerName, playerAddress, status, playerSelectedAction);
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

void NetCenterPlayerList::updatePlayer(String playerName, PeerAddress playerAddress, int status)
{
    for (int i = 0 ; i < entries.size() ; i++) {
        if (entries[i]->playerAddress == playerAddress) {
            PlayerEntry *currentEntry = entries[i];
            currentEntry->updateEntry(playerName, status);
            return;
        }
    }
}

void NetCenterPlayerList::PlayerSelectedAction::action()
{
    printf("ZZZ\n");
    targetMenu->playerSelected(address, playerName);
}

class SayAction : public Action {
public:
    SayAction(PuyoNetGameCenter *netCenter, Text *message) : netCenter(netCenter), message(message) {}
    void action() {
        printf("Message: %s\n", (const char *)message->getValue());
        netCenter->sendMessage(message->getValue());
    }
private:
    PuyoNetGameCenter *netCenter;
    Text *message;
};

String NetCenterTwoNameProvider::getPlayer1Name() const
{
    return netCenter.getSelfName();
}

String NetCenterTwoNameProvider::getPlayer2Name() const
{
    return netCenter.getOpponentName();
}

NetCenterMenu::NetCenterMenu(PuyoMainScreen *mainScreen, PuyoNetGameCenter *netCenter, GameLoop *loop)
    : PuyoMainScreenMenu(mainScreen, loop),
      netCenter(netCenter), playerListText(theCommander->getLocalizedString("Player List")),
      chatAreaText(theCommander->getLocalizedString("Chat Area")),
      cycled(this),
      playerList(5, this), onScreenDialog(NULL), shouldSelfDestroy(false),
      nameProvider(*netCenter), chatBox(*this),
      title(theCommander->getLocalizedString("Network Game Center")), backAction(mainScreen),
      cancelButton(theCommander->getLocalizedString("Disconnect"), &backAction)
{
    GameUIDefaults::GAME_LOOP->addIdle(&cycled);
    netCenter->addListener(this);
}

NetCenterMenu::~NetCenterMenu()
{
    printf("Deleting the net center\n");
    // Warn the mainScreen that we are not in the net game center any more
    mainScreen->setInNetGameCenter(false);
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

    container.add(&mainBox);
    container.setPosition(Vec3(5,5));
    container.setSize(Vec3(630,470, 0));

    menu.add(&title);
    menu.add(&cancelButton);
    
    playerbox.setPolicy(USE_MIN_SIZE);
    playerbox.add(&playerListText);
    playerbox.add(&playerList);

    topbox.add(&menu);
    topbox.add(&playerbox);

    mainBox.add(&topbox);
    mainBox.add(&chatBox);

}

void NetCenterMenu::sendChat(String chatText)
{
    netCenter->sendMessage(chatText);
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
    chatBox.addChat(msgAuthor, msg);
}

void NetCenterMenu::onPlayerConnect(String playerName, PeerAddress playerAddress)
{
    //printf("Connect: %s\n", (const char *)(netCenter->getPeerNameAtIndex(playerIndex)));
    playerList.addNewPlayer(playerName, playerAddress, netCenter->getPeerStatusForAddress(playerAddress));
}

void NetCenterMenu::onPlayerDisconnect(String playerName, PeerAddress playerAddress)
{
    playerList.removePlayer(playerAddress);
}

void NetCenterMenu::onPlayerUpdated(String playerName, PeerAddress playerAddress)
{
    playerList.updatePlayer(playerName, playerAddress, netCenter->getPeerStatusForAddress(playerAddress));
}

void NetCenterMenu::onGameInvitationReceived(PuyoGameInvitation &invitation)
{
    // If already waiting for a game, cancel the invitation
    if (this->onScreenDialog != NULL) {
        netCenter->cancelGameInvitation(invitation);
    }
    else {
        onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Invitation for a game"), invitation.opponentName + theCommander->getLocalizedString(" invited you to play"), true);
        container.add(onScreenDialog);
        onScreenDialog->build();
        this->focus(onScreenDialog);
    }
}

void NetCenterMenu::onWidgetVisibleChanged(bool visible)
{
    printf("netcentermenu visible: %s\n", visible ? "true" : "false");
    /*if (!visible)
        delete this;*/
}

void NetCenterMenu::onWidgetRemoved(WidgetContainer *parent)
{
     printf("netcentermenu removed");
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

void NetCenterMenu::onGameInvitationCanceledReceived(PuyoGameInvitation &invitation)
{
    if (this->onScreenDialog != NULL) {
        if (invitation.opponentAddress == onScreenDialog->associatedInvitation.opponentAddress) {
            container.remove(onScreenDialog);
            delete onScreenDialog;
            onScreenDialog = NULL;
        }
    }
}

void NetCenterMenu::onGameGrantedWithMessagebox(MessageBox *mbox, PuyoGameInvitation &invitation)
{
    printf("Game granted with a seed of %d\n", invitation.gameRandomSeed);
    PuyoNetworkTwoPlayerGameWidgetFactory *factory = new PuyoNetworkTwoPlayerGameWidgetFactory(*mbox, invitation.gameRandomSeed);
    TwoPlayersStarterAction *starterAction = new TwoPlayersStarterAction(0, *factory, &nameProvider);
    
    starterAction->action();
    
    if (this->onScreenDialog != NULL) {
        container.remove(onScreenDialog);
        delete(onScreenDialog);
        onScreenDialog = NULL;
    }
}

void NetCenterMenu::playerSelected(PeerAddress playerAddress, String playerName)
{
    PuyoGameInvitation invitation;
    invitation.gameRandomSeed = (unsigned long)(fmod(getTimeMs(), (double)0xFFFFFFFF));
    invitation.opponentAddress = playerAddress;
    onScreenDialog = new NetCenterDialogMenu(this, invitation, theCommander->getLocalizedString("Asking for a game"), String(theCommander->getLocalizedString("Waiting ")) + playerName + theCommander->getLocalizedString(" for confirmation"), false);
    container.add(onScreenDialog);
    onScreenDialog->build();
    this->focus(onScreenDialog);
    
    netCenter->requestGame(invitation);
}

void NetCenterMenu::show()
{
  netCenter->setStatus(PEER_NORMAL);
  //Screen::show();
}
