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
#include "PuyoNetCenterMenu.h"

extern IIM_Surface *menuBG_wide; // I know what you think..

using namespace ios_fc;

class HttpDocument {
public:
    HttpDocument(String hostName, String path, int portNum);
    VoidBuffer getDocumentContent() const { return docContent; }
    bool documentIsReady();

private:
    String getLine();
    static const String contentLength;
    static const String rqPart[];
    class HttpHeaderElement;
    bool docIsReady;
    Socket httpSocket;
    InputStream *httpInputStream;
    int msgSize;
    Buffer<char> docContent;
    String path;
    String hostName;
    int portNum;
};

class HttpDocument::HttpHeaderElement {
public:
    HttpHeaderElement(String rawElement);
    String name;
    String content;
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
    : docIsReady(false), httpSocket(hostName, portNum), httpInputStream(httpSocket.getInputStream()), msgSize(-1), path(path), hostName(hostName), portNum(portNum)
{
}

bool HttpDocument::documentIsReady() 
{
    if (docIsReady) return true;
    return false;

    // 1 - check if we can write to the output ?
    String request = rqPart[0] + path + rqPart[1] + hostName + rqPart[2] + portNum + rqPart[3];
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
    return false;
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

HttpDocument *doc = NULL;

PuyoHttpServerList::PuyoHttpServerList(String hostName, String path, int portNum)
{
  isReady = false;
  //doc = new HttpDocument(hostName, path, portNum);
}

bool PuyoHttpServerList::listIsReady() 
{
    if (isReady) return true;
    /*if (!doc->documentIsReady())*/ return false;
    
    StandardMessage msg(doc->getDocumentContent());
    int nbServers = msg.getInt("NBSERV");
    for (int i = 0 ; i < nbServers ; i++) {
        char tmpStr[256];
        sprintf(tmpStr, "SERVNAME%.2d", i);
        String serverName = msg.getString(tmpStr);
        sprintf(tmpStr, "PORTNUM%.2d", i);
        int portNum = msg.getInt(tmpStr);
        servers.add(new PuyoHttpServer(serverName, portNum));
    }
    isReady = true;
    return true;
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
    ServerSelectAction(InternetGameMenu &igm, String serverName, int portNum)
      : gameMenu(igm), serverName(serverName), portNum(portNum) {}
    void action() {
        gameMenu.setSelectedServer(serverName, portNum);
    }
private:
    InternetGameMenu &gameMenu;
    String serverName;
    int portNum;
};

class PushNetCenterMenuAction : public Action {
public:
    PushNetCenterMenuAction(Text *serverName, Text *userName) : serverName(serverName), userName(userName) {}
    void action() {
        fprintf(stderr, "Connecting to %s..\n", serverName->getValue().c_str());
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

void NetworkGameMenu::build() {
  internetGameMenu.build();
  lanGameMenu.build();
  add(new Text("Network Game"));
  add(new Button("Local Area Network Game", new PuyoPushMenuAction(&lanGameMenu, mainScreen)));
  add(new Button("Internet Game", new PushScreenAction(&internetGameMenu)));
  add(new Button("Cancel",        new PuyoPopMenuAction(mainScreen)));
}

InternetGameMenu::InternetGameMenu()
  : PuyoScreen()
  , servers("www.ios-software.com", "/flobopuyo/fpservers", 80)
  , story(666)
  , container()
{
}

void InternetGameMenu::build()
{
    add(&story);
    add(&container);
    container.add(&menu);

    container.setPosition(Vec3(5,195));
    container.setSize(Vec3(menuBG_wide->w, menuBG_wide->h, 0));
    container.setBackground(menuBG_wide);
  
    serverSelectionPanel = new VBox;
    serverSelectionPanel->add(new Text("Server List"));
    serverListPanel = new ListWidget(6);
    
    serverSelectionPanel->add(serverListPanel);
    updating = new Button("Update");
    serverSelectionPanel->add(updating);

    // temporary entry into the server list
    serverListPanel->add (new Button("durandal.homeunix.com",
                                     new ServerSelectAction(*this, "durandal.homeunix.com", 6581)));
    
    playerName = new EditField("Kaori"); // TODO: get that from single player name
    serverName = new Text("---");
    VBox *rightPanel = new VBox();
    rightPanel->add(new Separator(1,1));
    rightPanel->add(new Text("Internet Game"));
    rightPanel->add(new Separator(10,10));
    rightPanel->add(new Text("Nickname"));
    rightPanel->add(playerName);
    rightPanel->add(new Separator(10,10));
    rightPanel->add(new Text("Server"));
    rightPanel->add(serverName);
    rightPanel->add(new Separator(10,10));
    HBox *hbox = new HBox();
    hbox->add(new Button("Join", new PushNetCenterMenuAction(serverName, playerName)));
    hbox->add(new Button("Cancel", new PopScreenAction()));
    rightPanel->add(hbox);
    rightPanel->add(new Separator(1,1));
    menu.add(serverSelectionPanel);
    menu.add(rightPanel);
}

void InternetGameMenu::idle(double currentTime)
{
    if (servers.listIsReady())
    {
        serverListPanel->clear();
        for (int i = 0 ; i < servers.getNumServer() ; i++) {
            serverListPanel->set (i, new Button(servers.getServerNameAtIndex(i),
                                                new ServerSelectAction(*this, servers.getServerNameAtIndex(i),
                                                                       servers.getServerPortAtIndex(i))));
        }
        updating->setValue("Update");
        updating->mdontMove = false;
    }
    else
    {
        int X = (int)(currentTime*3) % 6;
        static const char *txt[6] = {
            ".........Loading",
            "......Loading...",
            "...Loading......",
            "Loading.........",
            "...Loading......",
            "......Loading..."
        };
        updating->setValue(txt[X]);
        updating->mdontMove = true;
    }
}

void InternetGameMenu::setSelectedServer(const String &serverName, int portNum)
{
    this->serverName->setValue(serverName);
    this->portNum = portNum;
}
