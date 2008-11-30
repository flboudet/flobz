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
 
#ifndef _IOSHTTPDOCUMENT
#define _IOSHTTPDOCUMENT

#include "ios_socket.h"

namespace ios_fc {

class HttpDocument {
public:
    HttpDocument(String hostName, String path, int portNum);
    VoidBuffer getDocumentContent() const { return docContent; }
    bool documentIsReady();

private:
    bool getLine();
    static const String contentLength;
    static const String rqPart[];
    class HttpHeaderElement;
    bool socketIsConnected;
    bool docIsReady;
    bool headerIsDone;
    Socket httpSocket;
    InputStream *httpInputStream;
    int msgSize;
    Buffer<char> docContent;
    int docContentOffset;
    String path;
    String hostName;
    int portNum;
    String currentLine;
};

class HttpDocument::HttpHeaderElement {
public:
    HttpHeaderElement(String rawElement);
    String name;
    String content;
};


}

#endif // _IOSHTTPDOCUMENT
