/**
 * iosfc::IGPDatagram: contains an object for handling IGP messages.
 * 
 * This file is part of the iOS Foundation Classes project.
 *
 * authors:
 *  Jean-Christophe Hoelt <jeko@ios-software.com>
 *  Guillaume Borios      <gyom@ios-software.com>
 *  Florent Boudet       <flobo@ios-software.com>
 *  Michel Metzger -- participate during the developement of Shubunkin!
 *
 * http://www.ios-software.com/
 *
 * Released under the terms of the GNU General Public Licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef _IGPDATAGRAM_H
#define _IGPDATAGRAM_H

#include "ios_message.h"
#include "ios_memory.h"

namespace ios_fc {

class IGPDatagram {
public:
    static const char MSGIDENT[];
    static const char IGPIDENT[];
    static const char IGPORIGIDENT[];
    static const char IGPDESTIDENT[];
    static const char IGPMSG[];
    
    enum IGPMsgIdent {
        ClientMsgAutoAssignID,
        ClientMsgAssignID,
        ClientMsgGetID,
        ClientMsgToClient,
        ServerMsgInformID,
        ServerMsgBadRequest,
        ServerMsgToClient
    };
    IGPDatagram(Message *data);
    IGPDatagram(IGPDatagram &datagram) : message(datagram.message),  msgIdent(msgIdent) {}
    
    virtual ~IGPDatagram() {}
    
    Message *getMessage() const { return message; }
    int getMsgIdent() const { return msgIdent; }
    
    class ClientMsgAutoAssignIDDatagram;
    class ServerMsgInformIDDatagram;
    class ClientMsgGetIDDatagram;
    class ClientMsgToClientDatagram;
    class ServerMsgToClientDatagram;
    class ClientMsgAssignIDDatagram;
protected:
    IGPDatagram(Message *data, IGPMsgIdent ident);
    Message *message;
private:
    int msgIdent;
};

class IGPDatagram::ClientMsgAutoAssignIDDatagram : public IGPDatagram {
public:
    ClientMsgAutoAssignIDDatagram(Message *data) : IGPDatagram(data, ClientMsgAutoAssignID) {
        message->addBoolProperty("RELIABLE", true);
    }
};

class IGPDatagram::ClientMsgAssignIDDatagram : public IGPDatagram {
public:
    ClientMsgAssignIDDatagram(Message *data, int igpIdent) : IGPDatagram(data, ClientMsgAssignID) {
        message->addInt(IGPIDENT, igpIdent);
        message->addBoolProperty("RELIABLE", true);
    }
    ClientMsgAssignIDDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = message->getInt(IGPIDENT);
    }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
};

class IGPDatagram::ClientMsgGetIDDatagram : public IGPDatagram {
public:
    ClientMsgGetIDDatagram(Message *data) : IGPDatagram(data, ClientMsgGetID) {
        message->addBoolProperty("RELIABLE", true);
    }
};

class IGPDatagram::ServerMsgInformIDDatagram : public IGPDatagram {
public:
    ServerMsgInformIDDatagram(Message *data, int igpIdent) : IGPDatagram(data, ServerMsgInformID), igpIdent(igpIdent) {
        message->addInt(IGPIDENT, igpIdent);
        message->addBoolProperty("RELIABLE", true);
    }
    ServerMsgInformIDDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = message->getInt(IGPIDENT);
    }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
};

class IGPDatagram::ClientMsgToClientDatagram : public IGPDatagram {
public:
    ClientMsgToClientDatagram(Message *data, int igpIdent, VoidBuffer msg, bool reliable) : IGPDatagram(data, ClientMsgToClient), igpIdent(igpIdent), msg(msg) {
        message->addInt(IGPIDENT, igpIdent);
        message->addCharArray(IGPMSG, this->msg);
        message->addBoolProperty("RELIABLE", reliable);
    }
    ClientMsgToClientDatagram(IGPDatagram &datagram) : IGPDatagram(datagram), msg(message->getCharArray(IGPMSG)) {
        igpIdent = message->getInt(IGPIDENT);
    }
    VoidBuffer getIgpMessage() const { return msg; }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
    Buffer<char> msg;
};

class IGPDatagram::ServerMsgToClientDatagram : public IGPDatagram {
public:
    ServerMsgToClientDatagram(Message *data, int igpOrigIdent, int igpDestIdent, VoidBuffer msg, bool reliable)
    : IGPDatagram(data, ServerMsgToClient), igpIdent(igpOrigIdent), igpDestIdent(igpDestIdent), msg(msg) {
        message->addInt(IGPORIGIDENT, igpIdent);
        message->addInt(IGPDESTIDENT, igpDestIdent);
        message->addCharArray(IGPMSG, this->msg);
        message->addBoolProperty("RELIABLE", reliable);
    }
    ServerMsgToClientDatagram(IGPDatagram &datagram) : IGPDatagram(datagram), msg(message->getCharArray(IGPMSG)) {
        igpIdent = message->getInt(IGPORIGIDENT);
        igpDestIdent = message->getInt(IGPDESTIDENT);
    }
    VoidBuffer getIgpMessage() const { return msg; }
    int getIgpOriginIdent() const { return igpIdent; }
    int getIgpDestinationIdent() const { return igpDestIdent; }
private:
    int igpIdent, igpDestIdent;
    Buffer<char> msg;
};

};

#endif // _IGPDATAGRAM_H

