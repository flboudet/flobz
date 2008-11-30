#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ios_socket.h"
#include "ios_standardmessage.h"

using namespace ios_fc;

const char *requeste="GET / HTTP/1.1\n\
Host: durandal.homeunix.com:80\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.5) Gecko/20050105 Epiphany/1.4.7\n\
";
const char *requesty=
"GET / HTTP/1.0\n\
Host: slashdot.org:80\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.5) Gecko/20050105 Epiphany/1.4.7\n\
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png\n\
Accept-Language: en\n\
\n";

const char *requestz=
"GET / HTTP/1.1\n\
Host: durandal.homeunix.com:80\n\
User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.7.5) Gecko/20050105 Epiphany/1.4.7\n\
Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\n\
Accept-Language: fr,en;q=0.5\n\
Accept-Encoding: gzip,deflate\n\
Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\n\
Keep-Alive: 300\n\
Connection: keep-alive\n";

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

class PuyoHttpServerList {
public:
    PuyoHttpServerList(String hostName, String path, int portNum);
    String getServerNameAtIndex(int index) const;
    int getServerPortAtIndex(int index) const;
    int getNumServer() const;
private:
    class PuyoHttpServer;
    AdvancedBuffer<PuyoHttpServer *> servers;
};

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

int main()
{
    try {
        //PuyoHttpServerList toto("durandal.homeunix.com", "/flobopuyo/fpservers", 80);
        PuyoHttpServerList toto("www.ios-software.com", "/flobopuyo/fpservers", 80);
        for (int i = 0 ; i < toto.getNumServer() ; i++) {
            printf("Serveur %d: %s:%d\n", i, (const char *)toto.getServerNameAtIndex(i), toto.getServerPortAtIndex(i));
        }
    } catch (Exception e) {
        printf("Exception:%s\n", (const char *)(e.getMessage()));
    }
    return 0;
}


