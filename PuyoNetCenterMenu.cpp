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

NetCenterMenu::NetCenterChatArea::NetCenterChatArea(int height)
    : height(height), lines(new (HBox *)[height]), names(new (Text *)[height]), texts(new (Text *)[height])
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
    requestDraw();
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

NetCenterMenu::NetCenterMenu(PuyoNetGameCenter *netCenter) : netCenter(netCenter)
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
    
    add(new Text("Network Game Center"));
    
    HBox *screenCenter = new HBox();
    
    VBox *chatBox = new VBox();
    chatArea = new NetCenterChatArea(10);
    EditFieldWithLabel *sayField = new EditFieldWithLabel("Say:", "");
    SayAction *say = new SayAction(netCenter, sayField->getEditField());
    sayField->getEditField()->setAction(ON_START, say);
    chatBox->add(chatArea);
    chatBox->add(sayField);
    
    playerList = new VBox();
    playerList->add(new Text("Players"));
    
    screenCenter->add(chatBox);
    screenCenter->add(playerList);
    
    add(screenCenter);
    add(new Button("Exit", new PopScreenAction()));
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
    chatArea->addChat(msgAuthor, msg);
}

void NetCenterMenu::onPlayerConnect(int playerIndex)
{
    printf("Connect: %s\n", (const char *)(netCenter->getPeerNameAtIndex(playerIndex)));
    playerList->add(new Text(netCenter->getPeerNameAtIndex(playerIndex)));
}
