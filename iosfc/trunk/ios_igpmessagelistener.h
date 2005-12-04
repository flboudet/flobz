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

class IgpPeer;

class IgpMessageListener : public MessageListener, public SessionListener {
public:
    IgpMessageListener(MessageBox &mbox) : mbox(mbox), currentAutoIgpIdent(firstAutoIgpIdent) {}
    void onMessage(Message &message);
    void onPeerConnect(const PeerAddress &address);
    void onPeerDisconnect(const PeerAddress &address);
    void sendMessageToAddress(IgpPeer *peer, VoidBuffer igpMessage, int destIgpIdent, bool reliable);
    
    void addPeer(IgpPeer *peer) { knownPeers.add(peer); }
    void removePeer(IgpPeer *peer) { knownPeers.remove(peer); }
    // should be private
    Message *createMessage() { return mbox.createMessage(); }
    int getUniqueIGPId();
    bool igpIdValidAndUnique(int igpIdent);
private:
    AdvancedBuffer<IgpPeer *> knownPeers;
    class NetworkIgpPeer;
    IgpPeer *getPeer(int igpIdent) const;
    
    static const int firstAutoIgpIdent;
    MessageBox &mbox;
    int currentAutoIgpIdent;
    NetworkIgpPeer *findPeer(PeerAddress address);
};

class IgpPeer {
public:
    IgpPeer() : pool(NULL), igpID(-1) {}
    IgpPeer(IgpMessageListener *pool) : pool(NULL), igpID(-1) { registerSelf(pool); }
    virtual ~IgpPeer() {
        if (pool != NULL)
            pool->removePeer(this);
    }
    virtual void messageReceived(VoidBuffer message, int origIgpIdent, bool reliable) = 0;
    inline int getIgpIdent() const { return igpID; }
    inline void setIgpIdent(int igpId) { this->igpID = igpId; }
protected:
    void registerSelf(IgpMessageListener *pool)
    {
        this->pool = pool;
        pool->addPeer(this);
    }
    inline Message *createMessage() { return pool->createMessage(); }
    inline int getUniqueIGPId() { return  pool->getUniqueIGPId(); }
    inline int igpIdValidAndUnique(int igpId) { return pool->igpIdValidAndUnique(igpId); }
    inline void sendMessageToAddress(VoidBuffer igpMessage, int destIgpIdent, bool reliable)
        { pool->sendMessageToAddress(this, igpMessage, destIgpIdent, reliable); }
private:
    IgpMessageListener *pool;
    int igpID;
    //bool valid;
};

class IgpMessageListener::NetworkIgpPeer : public IgpPeer {
public:
    NetworkIgpPeer(PeerAddress address, IgpMessageListener *pool) : address(address) {
        registerSelf(pool);
    }
    ~NetworkIgpPeer() {}
    inline PeerAddress getAddress() const { return address; }
    void messageReceived(VoidBuffer message, int origIgpIdent, bool reliable);
    void datagramFromMyself(IGPDatagram &message);
private:
    void sendIGPIdent();
    PeerAddress address;
    int igpID;
};

class IgpVirtualPeer : public IgpPeer {
public:
    IgpVirtualPeer(IgpMessageListener *pool, int igpIdent) : IgpPeer(pool) {
        if (igpIdValidAndUnique(igpIdent))
            setIgpIdent(igpIdent);
        else throw Exception("IGP ident already registered!");
    }
    IgpVirtualPeer(IgpMessageListener *pool) : IgpPeer(pool) {
        setIgpIdent(getUniqueIGPId());
    }
};

}

#endif //_IGPMESSAGELISTENER_H

