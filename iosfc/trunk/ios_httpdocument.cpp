/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
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

#include "ios_httpdocument.h"

namespace ios_fc {

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
    : socketIsConnected(false), docIsReady(false), headerIsDone(false), httpSocket(hostName, portNum, true),
      httpInputStream(httpSocket.getInputStream()), msgSize(-1), path(path),
      hostName(hostName), portNum(portNum)
{
}

bool HttpDocument::documentIsReady() 
{
    if (docIsReady) return true;

    if (!socketIsConnected) {
        if (httpSocket.isConnected()) {
            socketIsConnected = true;
            httpSocket.setNonBlockingMode(false);
            String request = rqPart[0] + path + rqPart[1] + hostName + rqPart[2] + portNum + rqPart[3];
            httpSocket.getOutputStream()->streamWrite(VoidBuffer(request, strlen(request)));
        }
        else return false;
    }

    if (!headerIsDone) {
        do {
            currentLine = "";
            if (!getLine()) return false;
            printf("Current line:%s\n", (const char *)currentLine);
            HttpHeaderElement currentHeader(currentLine);
            if (currentHeader.name == contentLength) {
                msgSize = atoi((const char *)currentHeader.content);
                printf("Taille doc: %d\n", msgSize);
            }
        } while (currentLine.length() > 0);
        headerIsDone = true;
    }
    
    Buffer<char> msg(msgSize==-1 ? 1024 : msgSize);
    httpInputStream->streamRead(msg);
    docContent = msg;
    docIsReady = true;
    return true;
}

bool HttpDocument::getLine()
{
    int available = httpInputStream->streamAvailable();
    if (available > 0) {
        Buffer<char> tmp(2);
        do {
            httpInputStream->streamRead(tmp, 1);
            tmp[1] = 0;
            if ((tmp[0] != 10) && (tmp[0] != 13))
                currentLine += (const char *)tmp;
            available--;
        } while ((tmp[0] != '\n') && (available > 0));
        return true;
    }
    else return false;
}

}

