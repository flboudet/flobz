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

#include "PuyoNetworkMenu.h"
#include "PuyoInternetGameCenter.h"
#include "preferences.h"
#include "ios_socket.h"
#include "ios_standardmessage.h"

using namespace ios_fc;

class HttpDocument {
public:
    HttpDocument(String hostName, String path, int portNum);
    VoidBuffer getDocumentContent() const { return docContent; }
private:
    String getLine();
    static const String contentLength;
    static const String rqPart[];
    class HttpHeaderElement;
    Socket httpSocket;
    InputStream *httpInputStream;
    int msgSize;
    Buffer<char> docContent;
};

class HttpDocument::HttpHeaderElement {
public:
    HttpHeaderElement(String rawElement);
    String name;
    String content;
};

class NetCenterMenu : public PuyoScreen, PuyoNetGameCenterListener {
public:
    NetCenterMenu(PuyoNetGameCenter *netCenter);
    void build();
    void onChatMessage(const String &msgAuthor, const String &msg);
    void cycle();
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
    class NetCenterChatArea;
    NetCenterChatArea *chatArea;
    NetCenterCycled *cycled;
    PuyoNetGameCenter *netCenter;
};

class NetCenterMenu::NetCenterChatArea : public VBox {
public:
    NetCenterChatArea::NetCenterChatArea(int height);
    void addChat(String name, String text);
private:
    int height;
    HBox **lines;
    Text **names;
    Text **texts;
};

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
        //names[i]->requestDraw();
        //texts[i]->requestDraw();
    }
    names[height-1]->setValue(name);
    texts[height-1]->setValue(text);
    //names[height-1]->requestDraw();
    //texts[height-1]->requestDraw();
    requestDraw();
}

class PushNetCenterMenuAction : public Action {
public:
    PushNetCenterMenuAction(Text *serverName, Text *userName) : serverName(serverName), userName(userName) {}
    void action() {
        PuyoInternetGameCenter *gameCenter = new PuyoInternetGameCenter(serverName->getValue(),
                                                                        4567, userName->getValue());
        NetCenterMenu *newNetCenterMenu = new NetCenterMenu(gameCenter);
        newNetCenterMenu->build();
        (GameUIDefaults::SCREEN_STACK)->push(newNetCenterMenu);
    }
private:
    Text *serverName;
    Text *userName;
};

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

HttpDocument::HttpHeaderElement::HttpHeaderElement(String rawElement)
{
    int sep = 0;
    while ((sep < rawElement.length()) && (rawElement[sep] != ':')) { sep++; }
    if (sep+1 >= rawElement.length()) {
        name = rawElement;
    }
    else {
        name = rawElement.substring(0, sep);
        content = rawElement.substring(sep+2, rawElement.length());
    }
}

const String HttpDocument::contentLength = "Content-Length";
const String HttpDocument::rqPart[] =
{"GET ", " HTTP/1.0\r\n\
Host: ", ":", "\r\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.5) Gecko/20050105 Epiphany/1.4.7\r\n\
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png\r\n\
Accept-Language: en\r\n\
\r\n"};

HttpDocument::HttpDocument(String hostName,  String path, int portNum)
    : httpSocket(hostName, portNum), httpInputStream(httpSocket.getInputStream()), msgSize(-1)
{
    String request = rqPart[0] + path + rqPart[1] + hostName + rqPart[2] + portNum + rqPart[3];
    //printf("request:%s\n", (const char *)request);
    httpSocket.getOutputStream()->streamWrite(VoidBuffer(request, strlen(request)));
    String currentLine;
    do {
        currentLine = getLine();
        HttpHeaderElement currentHeader(currentLine);
        if (currentHeader.name == contentLength) {
            msgSize = atoi((const char *)currentHeader.content);
            printf("Taille doc: %d\n", msgSize);
        }
    } while (currentLine.length() > 0);
    Buffer<char> msg(msgSize==-1 ? 1024 : msgSize);
    httpInputStream->streamRead(msg);
    docContent = msg;
}

String HttpDocument::getLine()
{
    Buffer<char> tmp(2);
    String test;
    do {
        httpInputStream->streamRead(tmp, 1);
        tmp[1] = 0;
        if ((tmp[0] != 10) && (tmp[0] != 13))
            test += (const char *)tmp;
    } while (tmp[0] != '\n');
    return test;
}

class PuyoHttpServerList::PuyoHttpServer {
public:
    PuyoHttpServer(String hostName, int portNum) : hostName(hostName), portNum(portNum) {}
    String hostName;
    int portNum;
};

PuyoHttpServerList::PuyoHttpServerList(String hostName, String path, int portNum)
{
    HttpDocument doc(hostName, path, portNum);
    StandardMessage msg(doc.getDocumentContent());
    int nbServers = msg.getInt("NBSERV");
    for (int i = 0 ; i < nbServers ; i++) {
        char tmpStr[256];
        sprintf(tmpStr, "SERVNAME%.2d", i);
        String serverName = msg.getString(tmpStr);
        sprintf(tmpStr, "PORTNUM%.2d", i);
        int portNum = msg.getInt(tmpStr);
        servers.add(new PuyoHttpServer(serverName, portNum));
    }
}


String PuyoHttpServerList::getServerNameAtIndex(int index) const
{
    return servers[index]->hostName;
}

int PuyoHttpServerList::getServerPortAtIndex(int index) const
{
    return servers[index]->portNum;
}

int PuyoHttpServerList::getNumServer() const
{
    return servers.size();
}


// Actions
class ServerSelectAction : public Action {
public:
    ServerSelectAction(String serverName, int portNum) : serverName(serverName), portNum(portNum) {}
    void action() {
        printf("Serveur selectionne: %s:%d\n", (const char *)serverName, portNum);
    }
private:
    String serverName;
    int portNum;
};

InternetGameMenu::InternetGameMenu() : servers("www.ios-software.com", "/flobopuyo/fpservers", 80)
{
}

void InternetGameMenu::build() {
    serverSelectionPanel = new HBox;
    serverSelectionPanel->add(new Text("Select a server:"));
    serverListPanel = new VBox;
    EditFieldWithLabel *playerName, *serverName;
    
    for (int i = 0 ; i < servers.getNumServer() ; i++) {
        serverListPanel->add (new Button(servers.getServerNameAtIndex(i),
                                         new ServerSelectAction(servers.getServerNameAtIndex(i),
                                                                servers.getServerPortAtIndex(i))));
    }
    
    serverSelectionPanel->add(serverListPanel);
    
    add(new Text("Internet Game"));
    playerName = new EditFieldWithLabel("Player name:", "toto");
    serverName = new EditFieldWithLabel("Server name:", "durandal.homeunix.com");
    add(playerName);
    add(serverSelectionPanel);
    add(serverName);
    add(new Button("Join", new PushNetCenterMenuAction(serverName->getEditField(), playerName->getEditField())));
    add(new Button("Cancel", new PopScreenAction()));
}

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
    EditFieldWithLabel *sayField = new EditFieldWithLabel("Say:", "");
    SayAction *say = new SayAction(netCenter, sayField->getEditField());
    sayField->getEditField()->setAction(ON_START, say);
    chatArea = new NetCenterChatArea(10);
    add(new Text("Network Game Center"));
    add(chatArea);
    add(sayField);
    add(new Button("Exit", new PopScreenAction()));
}

void NetCenterMenu::onChatMessage(const String &msgAuthor, const String &msg)
{
    printf("%s:%s\n", (const char *)msgAuthor, (const char *)msg);
    chatArea->addChat(msgAuthor, msg);
}
