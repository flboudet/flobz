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
    virtual ~IGPClientMessageListener() {};
};

class IGPClient : public MessageListener {
public:
    class PingTransaction;
    IGPClient(MessageBox &mbox, bool identify = true);
    IGPClient(MessageBox &mbox, int igpIdent);
    virtual ~IGPClient();
    void sendMessage(int igpID, VoidBuffer message, bool reliable);
    void idle();
    void addListener(IGPClientMessageListener *);
    void removeListener(IGPClientMessageListener *);
    void onMessage(Message &);
    int getIgpIdent() const { return igpIdent; }
    bool isEnabled() const { return enabled; }
    PingTransaction *ping(double timeoutMs, double interval=0.);
private:
    void sendPingMessage(int sequenceNumber) const;
    void unregisterPingTransaction(PingTransaction *pingTransaction);
    MessageBox &mbox;
    bool enabled;
    int igpIdent;
    AdvancedBuffer<IGPClientMessageListener*> listeners;
    AdvancedBuffer<PingTransaction *> pendingPingTransactions;
    double igpLastKeepAliveDate;
    double igpKeepAliveInterval;
};

class IGPClient::PingTransaction {
public:
    virtual ~PingTransaction();
    bool completed() { return m_completed; }
    bool success()   { return m_success;   }
    double time()    { return m_time;      }
private:
    PingTransaction(double initialTime, double timeoutMs, double interval, IGPClient *owner);
    void idle(double currentTime);
    bool m_completed, m_success;
    double m_time, m_initialTime, m_timeoutMs, m_interval, m_nextRepeatTime;
    IGPClient *m_owner;
    int m_sequenceNumber;
    friend class IGPClient;
};

}

#endif // _IGPCLIENT_H
