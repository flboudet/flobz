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
    PeerAddress(PeerAddressImpl *impl) : impl(impl) { impl->incrementUsage(); }
    PeerAddress(const PeerAddress &a) : impl(a.impl) { impl->incrementUsage(); }
    virtual ~PeerAddress() { impl->decrementUsage(); }
    PeerAddress & operator = (const PeerAddress &a) {
        if (impl != NULL) impl->decrementUsage();
        this->impl = a.impl;
        impl->incrementUsage();
        return *this;
    }
    bool operator == (const PeerAddress &a) const {
        return (*impl == *(a.impl));
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
};

}

#endif // _IOS_DIRIGEABLE_H