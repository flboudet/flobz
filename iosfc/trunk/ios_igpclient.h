/**
 * iosfc::IGPClient: a reusable class to communicate with an IGP server
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

#ifndef _IGPCLIENT_H
#define _IGPCLIENT_H

#include "ios_messagebox.h"
#include "ios_memory.h"

namespace ios_fc {

class IGPClientMessageListener {
public:
    virtual void onMessage(VoidBuffer message, int igpOriginIdent, int igpDestinationIdent) = 0;
};

class IGPClient : public MessageListener {
public:
    IGPClient(String hostName, int portID);
    IGPClient(String hostName, int portID, int igpIdent);
    virtual ~IGPClient();
    void sendMessage(int igpID, VoidBuffer message, bool reliable);
    void idle();
    void addListener(IGPClientMessageListener *);
    void removeListener(IGPClientMessageListener *);
    void onMessage(Message &);
private:
    bool enabled;
    int igpIdent;
    MessageBox *mbox;
    AdvancedBuffer<IGPClientMessageListener*> listeners;
    double igpLastKeepAliveDate;
    double igpKeepAliveInterval;
};

}

#endif // _IGPCLIENT_H
