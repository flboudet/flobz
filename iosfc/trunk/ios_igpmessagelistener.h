/**
 * iosfc::IGPMessageListener: a class designed to help the implementation of an IGP server
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

#ifndef _IGPMESSAGELISTENER_H
#define _IGPMESSAGELISTENER_H

#include "ios_selector.h"
#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_igpdatagram.h"
#include "ios_dirigeable.h"

namespace ios_fc {

class IgpMessageListener : public MessageListener, public SessionListener {
public:
    IgpMessageListener(MessageBox &mbox) : mbox(mbox), currentAutoIgpIdent(firstAutoIgpIdent) {}
    void onMessage(Message &message);
    void onPeerConnect(const PeerAddress &address);
    void onPeerDisconnect(const PeerAddress &address);
private:
    int getUniqueIGPId();
    bool igpIdValidAndUnique(int igpIdent);
    class PeerRecord;
    PeerRecord *getPeer(int igpIdent) const;
    Message *createMessage() { return mbox.createMessage(); }
    
    static const int firstAutoIgpIdent;
    MessageBox &mbox;
    int currentAutoIgpIdent;
    PeerRecord *findPeer(PeerAddress address);
    AdvancedBuffer<PeerRecord *> knownPeers;
};

class IgpMessageListener::PeerRecord {
public:
    PeerRecord(PeerAddress address, IgpMessageListener *pool) : address(address), pool(pool), valid(false) {
        pool->knownPeers.add(this);
    }
    ~PeerRecord() {
        pool->knownPeers.remove(this);
    }
    inline PeerAddress getAddress() const { return address; }
    void datagramReceived(IGPDatagram &message);
    inline int getIgpIdent() const { return igpID; }
private:
    void sendIGPIdent();
    PeerAddress address;
    IgpMessageListener *pool;
    int igpID;
    bool valid;
};

}

#endif //_IGPMESSAGELISTENER_H

