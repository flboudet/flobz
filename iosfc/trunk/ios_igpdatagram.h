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
        ClientMsgBroadcast,
        ServerMsgInformID,
        ServerMsgBadRequest,
        ServerMsgToClient
    };
    IGPDatagram(VoidBuffer data);
    virtual ~IGPDatagram() {}
    VoidBuffer serialize();
    int getMsgIdent() const { return msgIdent; }
    class ClientMsgAutoAssignIDDatagram;
    class ServerMsgInformIDDatagram;
protected:
    IGPDatagram(IGPMsgIdent ident, VoidBuffer message);
private:
    int msgSize;
    int msgIdent;
    bool complete;
};

class IGPDatagram::ClientMsgAutoAssignIDDatagram : public IGPDatagram {
public:
    ClientMsgAutoAssignIDDatagram() : IGPDatagram(ClientMsgAutoAssignID, VoidBuffer()) {}
};

class IGPDatagram::ServerMsgInformIDDatagram : public IGPDatagram {
public:
    ServerMsgInformIDDatagram(int igpIdent) : IGPDatagram(ServerMsgInformID, VoidBuffer()) {}
};

};

#endif // _IGPDATAGRAM_H

