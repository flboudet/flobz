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

#include "ios_memory.h"

namespace ios_fc {

class IGPDatagram {
public:
    enum IGPMsgIdent {
        ClientMsgAutoAssignID,
        ClientMsgAssignID,
        ClientMsgGetID,
        ClientMsgToClient,
        ServerMsgInformID,
        ServerMsgBadRequest,
        ServerMsgToClient
    };
    IGPDatagram(VoidBuffer data);
    IGPDatagram(IGPDatagram &datagram) : message(datagram.message), msgSize(datagram.msgSize),  msgIdent(msgIdent), complete(complete) {}
    virtual ~IGPDatagram() {}
    VoidBuffer serialize();
    int getMsgIdent() const { return msgIdent; }
    class ClientMsgAutoAssignIDDatagram;
    class ServerMsgInformIDDatagram;
    class ClientMsgGetIDDatagram;
    class ClientMsgToClientDatagram;
    class ServerMsgToClientDatagram;
    class ClientMsgAssignIDDatagram;
protected:
    IGPDatagram(IGPMsgIdent ident, int msgSize);
    Buffer<char> message;
    inline void writeBigEndianIntToMessage(int integer, int offset) {
        message[offset]     = (integer & 0xFF000000) >> 24;
        message[offset + 1] = (integer & 0x00FF0000) >> 16;
        message[offset + 2] = (integer & 0x0000FF00) >> 8;
        message[offset + 3] = (integer & 0x000000FF);
    }
    inline int readBigEndianIntFromMessage(int offset) const {
        return ((unsigned char)message[offset] << 24) | ((unsigned char)message[offset + 1] << 16) | ((unsigned char)message[offset + 2] << 8) | (unsigned char)message[offset + 3];
    }
private:
    int msgSize;
    int msgIdent;
    bool complete;
};

class IGPDatagram::ClientMsgAutoAssignIDDatagram : public IGPDatagram {
public:
    ClientMsgAutoAssignIDDatagram() : IGPDatagram(ClientMsgAutoAssignID, 0) {}
};

class IGPDatagram::ClientMsgAssignIDDatagram : public IGPDatagram {
public:
    ClientMsgAssignIDDatagram(int igpIdent) : IGPDatagram(ClientMsgAssignID, 4) {
        writeBigEndianIntToMessage(igpIdent, 8);
    }
    ClientMsgAssignIDDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = readBigEndianIntFromMessage(8);
    }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
};

class IGPDatagram::ClientMsgGetIDDatagram : public IGPDatagram {
public:
    ClientMsgGetIDDatagram() : IGPDatagram(ClientMsgGetID, 0) {}
};

class IGPDatagram::ServerMsgInformIDDatagram : public IGPDatagram {
public:
    ServerMsgInformIDDatagram(int igpIdent) : IGPDatagram(ServerMsgInformID, 4), igpIdent(igpIdent) {
        writeBigEndianIntToMessage(igpIdent, 8);
    }
    ServerMsgInformIDDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = readBigEndianIntFromMessage(8);
    }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
};

class IGPDatagram::ClientMsgToClientDatagram : public IGPDatagram {
public:
    ClientMsgToClientDatagram(int igpIdent, VoidBuffer msg) : IGPDatagram(ClientMsgToClient, 4 + msg.size()), igpIdent(igpIdent) {
        writeBigEndianIntToMessage(igpIdent, 8);
        Memory::memcpy((char *)(message.ptr()) + 12, msg.ptr(), msg.size());
    }
    ClientMsgToClientDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = readBigEndianIntFromMessage(8);
    }
    VoidBuffer getMessage() const {
        VoidBuffer result(message);
        result += 12;
        return result;
        //return message + 12;
    }
    int getIgpIdent() const { return igpIdent; }
private:
    int igpIdent;
};

class IGPDatagram::ServerMsgToClientDatagram : public IGPDatagram {
public:
    ServerMsgToClientDatagram(int igpOrigIdent, int igpDestIdent, VoidBuffer msg) : IGPDatagram(ServerMsgToClient, 8 + msg.size()), igpIdent(igpIdent), igpDestIdent(igpDestIdent) {
        writeBigEndianIntToMessage(igpOrigIdent, 8);
        writeBigEndianIntToMessage(igpDestIdent, 12);
        Memory::memcpy((char *)(message.ptr()) + 16, msg.ptr(), msg.size());
    }
    ServerMsgToClientDatagram(IGPDatagram &datagram) : IGPDatagram(datagram) {
        igpIdent = readBigEndianIntFromMessage(8);
        igpDestIdent = readBigEndianIntFromMessage(12);
    }
    VoidBuffer getMessage() const {
        VoidBuffer result(message);
        result += 16;
        return result;
        //return message + 16;
    }
    int getIgpOriginIdent() const { return igpIdent; }
    int getIgpDestinationIdent() const { return igpDestIdent; }
private:
    int igpIdent, igpDestIdent;
};

};

#endif // _IGPDATAGRAM_H

