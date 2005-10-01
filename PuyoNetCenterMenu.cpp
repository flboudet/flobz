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
#include "PuyoNetworkStarter.h"

NetCenterMenu::NetCenterChatArea::NetCenterChatArea(int height)
    : height(height), lines(new (HBox *[height])), names(new (Text *[height])), texts(new (Text *[height]))
{
    for (int i = 0 ; i < height ; i++) {
        lines[i] = new HBox;
        names[i] = new Text("");
        texts[i] = new Text("");
        lines[i]->add(names[i]);
        lines[i]->add(texts[i]);
        add(lines[i]);
    }
}

void NetCenterMenu::NetCenterChatArea::addChat(String name, String text)
{
    for (int i = 0 ; i < height-1 ; i++) {
        names[i]->setValue(names[i+1]->getValue());
        texts[i]->setValue(texts[i+1]->getValue());
    }
    names[height-1]->setValue(name);
    texts[height-1]->setValue(text);
}

void NetCenterMenu::NetCenterPlayerList::addNewPlayer(String playerName, PeerAddress playerAddress)
{
    PlayerEntry *newEntry = new PlayerEntry(playerName, playerAddress, new PlayerSelectedAction(targetMenu, playerAddress));
    entries.add(newEntry);
    add(newEntry);
}

void NetCenterMenu::NetCenterPlayerList::PlayerSelectedAction::action()
{
    printf("ZZZ\n");
    targetMenu->playerSelected(address);
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
    : netCenter(netCenter)/*, chatArea(8), playerList(this)*/
  , story(666)
{
    cycled = new NetCenterCycled(this);
    netCenter->addListener(this);
}

void NetCenterMenu::idle(double currentTime)
{
    cycled->idle(currentTime);
}

void NetCenterMenu::cycle()
{
    //PuyoScreen::idle(currentTime);
    //printf("Idle\n");
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
    
    ListWidget *playerlist = new ListWidget(8);
    VBox *chatlist = new VBox();
    
    menu->add(new Text("Network Game Center"));
    menu->add(new Button("Change Nick", new PopScreenAction()));
    menu->add(new Button("Options", new PopScreenAction()));
    menu->add(new Button("Disconnect", new PopScreenAction()));

    playerbox->add(new Text("Player List"));
    playerbox->add(playerlist);

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

    chatbox->add(new Text("Chat Area"));
    chatbox->add(chatlist);

    main->add(topbox);
    main->add(chatbox);

    // container.setBackground(menuBG_wide);
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
    //chatArea.addChat(msgAuthor, msg);
}

void NetCenterMenu::onPlayerConnect(String playerName, PeerAddress playerAddress)
{
    //printf("Connect: %s\n", (const char *)(netCenter->getPeerNameAtIndex(playerIndex)));
    //playerList.addNewPlayer(playerName, playerAddress);
}

void NetCenterMenu::onPlayerDisconnect(String playerName, PeerAddress playerAddress)
{
}

void NetCenterMenu::gameInvitationAgainst(String playerName, PeerAddress playerAddress)
{
    netCenter->acceptInvitationWith(playerAddress);
}

void NetCenterMenu::gameCanceledAgainst(String playerName, PeerAddress playerAddress)
{
}

void NetCenterMenu::gameGrantedWithMessagebox(MessageBox *mbox)
{
    //PuyoStarter *starter = new PuyoNetworkStarter(theCommander, 0, mbox);
    //starter->run(0,0,0,0,0);
    //GameUIDefaults::SCREEN_STACK->push(starter);
}

void NetCenterMenu::playerSelected(PeerAddress playerAddress)
{
    printf("Click joueur\n");
    netCenter->requestGameWith(playerAddress);
}

