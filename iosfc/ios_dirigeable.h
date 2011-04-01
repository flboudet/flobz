/**
 * Contains classes for management of destinations in messages
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

#ifndef _IOS_DIRIGEABLE_H
#define _IOS_DIRIGEABLE_H

namespace ios_fc {

class PeerAddressImpl {
public:
    PeerAddressImpl() : usage(0) {}
    virtual ~PeerAddressImpl() {}
    virtual bool operator == (const PeerAddressImpl &) const = 0;
    virtual bool operator < (const PeerAddressImpl &a) const = 0;
    inline int getUsage() const { return usage; }
    inline void incrementUsage() { usage++; }
    inline void decrementUsage() {
        usage--;
        if (usage < 1) {
            delete this;
        }
    }
private:
    int usage;
};

class PeerAddress {
public:
    PeerAddress() : impl(NULL) {}
    PeerAddress(PeerAddressImpl *impl) : impl(impl) { if (impl) impl->incrementUsage(); }
    PeerAddress(const PeerAddress &a) : impl(a.impl) { if (impl) impl->incrementUsage(); }
    virtual ~PeerAddress() { if (impl) impl->decrementUsage(); }
    PeerAddress & operator = (const PeerAddress &a) {
        if (impl != NULL) impl->decrementUsage();
        this->impl = a.impl;
        if (impl) impl->incrementUsage();
        return *this;
    }
    inline bool operator == (const PeerAddress &a) const {
        if (impl == a.impl) return true;
        if (impl == NULL) return false;
        if (a.impl == NULL) return false;
        return (*impl == *(a.impl));
    } 
    inline bool operator < (const PeerAddress &a) const {
        if (impl == a.impl) return false;
        if (impl == NULL) return true;
        if (a.impl == NULL) return false;
        return (*impl < *(a.impl));
        return false;
    }
    inline PeerAddressImpl *getImpl() const { return impl; }
private:
    PeerAddressImpl *impl;
};

class Dirigeable {
public:
    virtual PeerAddress getPeerAddress() = 0;
    virtual PeerAddress getBroadcastAddress() = 0;
    virtual void setPeerAddress(PeerAddress) = 0;
    virtual void addPeerAddress(const String key, const PeerAddress &value) = 0;
    virtual PeerAddress getPeerAddress(const String key) = 0;
    virtual ~Dirigeable() {};
};

class SessionListener {
public:
    virtual void onPeerConnect(const PeerAddress &address) = 0;
    virtual void onPeerDisconnect(const PeerAddress &address) = 0;
    virtual ~SessionListener() {};
};

class SessionManager {
public:
    virtual void addSessionListener(SessionListener *) = 0;
    virtual void removeSessionListener(SessionListener *) = 0;
    virtual ~SessionManager() {};
};

}

#endif // _IOS_DIRIGEABLE_H
