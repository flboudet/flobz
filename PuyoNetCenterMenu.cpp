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

class PuyoNetworkTwoPlayerGameWidgetFactory : public PuyoGameWidgetFactory {
public:
    PuyoNetworkTwoPlayerGameWidgetFactory(ios_fc::MessageBox &mbox) : mbox(mbox), gameId(0) {}
    PuyoGameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new PuyoNetworkGameWidget(puyoThemeSet, levelTheme, mbox, gameId++, gameOverAction);
    }
private:
    ios_fc::MessageBox &mbox;
    int gameId;
};

void NetCenterDialogMenu::NetCenterDialogMenuAction::action()
{
    if (! isCancelAction)
        targetMenu->grantCurrentGame();
    else targetMenu->cancelCurrentGame();
}

extern IIM_Surface *menuBG; // I know what you think..

NetCenterDialogMenu::NetCenterDialogMenu(NetCenterMenu *targetMenu, PeerAddress associatedPeer, String title, String message, bool hasAcceptButton)
    : cancelAction(targetMenu, true), acceptAction(targetMenu, false), hasAcceptButton(hasAcceptButton),
      associatedPeer(associatedPeer), dialogTitle(title), dialogMsg(message),
      acceptButton("Accept", &acceptAction), cancelButton("Cancel", &cancelAction)
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
  //remove(&menu);
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
    transitionToContent(&menu);
}

NetCenterChatArea::NetCenterChatArea(int height)
    : height(height), lines(new (HBox *[height])), names(new (Text *[height])), texts(new (Text *[height]))
{
    for (int i = 0 ; i < height ; i++) {
        lines[i] = new HBox;
        names[i] = new Text("");
        names[i]->setFont(GameUIDefaults::FONT_SMALL_ACTIVE);
        names[i]->setPreferedSize(Vec3(100, 12, 0));
        texts[i] = new Text("");
        texts[i]->setFont(GameUIDefaults::FONT_SMALL_INFO);
        texts[i]->setPreferedSize(Vec3(540, 12, 0));
        lines[i]->add(names[i]);
        lines[i]->add(texts[i]);
        add(lines[i]);
    }
}

NetCenterChatArea::~NetCenterChatArea()
{
    for (int i = 0 ; i < height ; i++) {
        delete texts[i];
        delete names[i];
        delete lines[i];
    }
    delete[] lines;
    delete[] names;
    delete[] texts;
}

void NetCenterChatArea::addChat(String name, String text)
{
    while (text.length() < 150)
        text += " ";
    for (int i = 0 ; i < height-1 ; i++) {
        names[i]->setValue(names[i+1]->getValue());
        texts[i]->setValue(texts[i+1]->getValue());
    }
    names[height-1]->setValue(name);
    texts[height-1]->setValue(text);
}

String NetCenterPlayerList::PlayerEntry::getStatusString(int status)
{
  switch (status) {
  case PEER_NORMAL:
    return "";
  case PEER_PLAYING:
    return " (playing)";
  default:
    return " (unknown)";
  }
}

void NetCenterPlayerList::addNewPlayer(String playerName, PeerAddress playerAddress, int status)
{
    PlayerEntry *newEntry = new PlayerEntry(playerName, playerAddress, status, new PlayerSelectedAction(targetMenu, playerAddress, playerName));
    entries.add(newEntry);
    add(newEntry);
}

void NetCenterPlayerList::removePlayer(PeerAddress playerAddress)
{
    for (int i = 0 ; i < entries.size() ; i++) {
        if (entries[i]->playerAddress == playerAddress) {
            PlayerEntry *currentEntry = entries[i];
            remove(currentEntry);
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
    
NetCenterMenu::NetCenterMenu(PuyoNetGameCenter *netCenter)
    : netCenter(netCenter), playerListText("Player List"), chatAreaText("Chat Area"),
      cycled(this),
      playerList(8, this), chatArea(8), story(666), onScreenDialog(NULL), shouldSelfDestroy(false)
{
    GameUIDefaults::GAME_LOOP->addIdle(&cycled);
    netCenter->addListener(this);
}

NetCenterMenu::~NetCenterMenu()
{
    printf("Deleting the net center\n");
    // Delete the network center because no one else would do it
    delete netCenter;
}

void NetCenterMenu::cycle()
{
    netCenter->idle();
}

void NetCenterMenu::build()
{
    add(&story);
    add(&container);

    VBox *main   = new VBox();  
    container.add(main);
    container.setPosition(Vec3(5,5));
    container.setSize(Vec3(630,470, 0));

    HBox *topbox = new HBox();
    VBox *menu   = new VBox();
    VBox *chatbox = new VBox();
    VBox *playerbox = new VBox();
    
    menu->add(new Text("Network Game Center"));
    menu->add(new Button("Change Nick"));
    menu->add(new Button("Options"));
    menu->add(new Button("Disconnect", new PopScreenAction()));

    playerbox->add(&playerListText);
    playerbox->add(&playerList);

    topbox->add(menu);
    topbox->add(playerbox);
 
/*    
    HBox *screenCenter = new HBox();
    
    EditFieldWithLabel *sayField = new EditFieldWithLabel("Say:", "");
    SayAction *say = new SayAction(netCenter, sayField->getEditField());
    sayField->getEditField()->setAction(ON_START, say);
    chatBox->add(&chatArea);
    chatBox->add(sayField);
    
    playerList.add(new Text("Players"));
    
    screenCenter->add(chatBox);
    screenCenter->add(&playerList);
    add(screenCenter);
    add(new Button("Exit", new PopScreenAction()));
  */  

    chatbox->add(&chatAreaText);
    chatbox->add(&chatArea);
    chatbox->setPreferedSize(Vec3(640, 120, 0));

    main->add(topbox);
    main->add(chatbox);

    // container.setBackground(menuBG_wide);
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
    chatArea.addChat(msgAuthor, msg);
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

void NetCenterMenu::gameInvitationAgainst(String playerName, PeerAddress playerAddress)
{
    /*netCenter->acceptInvitationWith(playerAddress);*/
    // If already waiting for a game, cancel the invitation
    if (this->onScreenDialog != NULL) {
        netCenter->cancelGameWith(playerAddress);
    }
    else {
        onScreenDialog = new NetCenterDialogMenu(this, playerAddress, "Invitation for a game", playerName + " invited you to play", true);
        add(onScreenDialog);
        onScreenDialog->build();
        this->focus(onScreenDialog);
    }
}

void NetCenterMenu::grantCurrentGame()
{
    if (this->onScreenDialog != NULL) {
        netCenter->acceptInvitationWith(onScreenDialog->associatedPeer);
    }
}

void NetCenterMenu::cancelCurrentGame()
{
    if (this->onScreenDialog != NULL) {
        netCenter->cancelGameWith(onScreenDialog->associatedPeer);
    }
}

void NetCenterMenu::gameCanceledAgainst(String playerName, PeerAddress playerAddress)
{
    if (this->onScreenDialog != NULL) {
        if (playerAddress == onScreenDialog->associatedPeer) {
            remove(onScreenDialog);
            delete onScreenDialog;
            onScreenDialog = NULL;
        }
    }
}

void NetCenterMenu::gameGrantedWithMessagebox(MessageBox *mbox)
{
    PuyoNetworkTwoPlayerGameWidgetFactory *factory = new PuyoNetworkTwoPlayerGameWidgetFactory(*mbox);
    TwoPlayersStarterAction *starterAction = new TwoPlayersStarterAction(0, *factory);
    
    starterAction->action();
    
    if (this->onScreenDialog != NULL) {
        remove(onScreenDialog);
        delete(onScreenDialog);
        onScreenDialog = NULL;
    }
    
    //PuyoStarter *starter = new PuyoNetworkStarter(theCommander, 0, mbox);
    //starter->run(0,0,0,0,0);
    //GameUIDefaults::SCREEN_STACK->push(starter);
}

void NetCenterMenu::playerSelected(PeerAddress playerAddress, String playerName)
{
    printf("Click joueur\n");
    onScreenDialog = new NetCenterDialogMenu(this, playerAddress, "Asking for a game", String("Waiting ") + playerName + " for confirmation", false);
    add(onScreenDialog);
    onScreenDialog->build();
    this->focus(onScreenDialog);
    netCenter->requestGameWith(playerAddress);
}

void NetCenterMenu::show()
{
  netCenter->setStatus(PEER_NORMAL);
  Screen::show();
}
