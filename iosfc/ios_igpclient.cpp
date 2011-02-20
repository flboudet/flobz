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

#include "ios_igpclient.h"
#include "ios_exception.h"
#include "ios_igpdatagram.h"
#include "ios_messagebox.h"
#include "ios_time.h"

namespace ios_fc {

IGPClient::IGPClient(MessageBox &mbox, bool identify) : mbox(mbox), enabled(false), igpLastKeepAliveDate(0.), igpKeepAliveInterval(2000.)
{
    mbox.addListener(this);
    if (identify) {
        IGPDatagram::ClientMsgAutoAssignIDDatagram datagram(mbox.createMessage());
        datagram.getMessage()->send();
        delete datagram.getMessage();
    }
}

IGPClient::IGPClient(MessageBox &mbox, int igpIdent) : mbox(mbox), enabled(false), igpLastKeepAliveDate(0.), igpKeepAliveInterval(2000.)
{
    mbox.addListener(this);
    IGPDatagram::ClientMsgAssignIDDatagram datagram(mbox.createMessage(), igpIdent);
    datagram.getMessage()->send();
    delete datagram.getMessage();
}

IGPClient::~IGPClient()
{
    mbox.removeListener(this);
    // remove all reference to this from the current pending ping transactions
    for (int i = 0 ; i < pendingPingTransactions.size() ; i++) {
        PingTransaction *currentTransaction = pendingPingTransactions[i];
        currentTransaction->m_owner = NULL;
    }
}

void IGPClient::sendMessage(int igpID, VoidBuffer message, bool reliable)
{
    IGPDatagram::ClientMsgToClientDatagram datagram(mbox.createMessage(), igpID, message, reliable);
    datagram.getMessage()->send();
    delete datagram.getMessage();
}

void IGPClient::idle()
{
    double time_ms = getTimeMs();
    if ((igpLastKeepAliveDate == 0.) || (time_ms - igpLastKeepAliveDate > igpKeepAliveInterval)) {
        IGPDatagram::ClientMsgKeepAliveDatagram datagram(mbox.createMessage());
        datagram.getMessage()->send();
        delete datagram.getMessage();
        igpLastKeepAliveDate = time_ms;
    }
    mbox.idle();
    // take care of the pending ping transactions
    for (int i = 0 ; i < pendingPingTransactions.size() ; i++) {
        PingTransaction *currentTransaction = pendingPingTransactions[i];
        currentTransaction->idle(time_ms);
    }
}

IGPClient::PingTransaction *IGPClient::ping(double timeoutMs, double interval)
{
    PingTransaction *newTransaction = new PingTransaction(getTimeMs(), timeoutMs, interval, this);
    pendingPingTransactions.add(newTransaction);
    return newTransaction;
}

void IGPClient::unregisterPingTransaction(IGPClient::PingTransaction *pingTransaction)
{
    pingTransaction->m_owner = NULL;
    pendingPingTransactions.remove(pingTransaction);
}

void IGPClient::onMessage(Message &rawMsg)
{
    if (!IGPDatagram::isIgpDatagram(rawMsg))
        return;
    IGPDatagram message(&rawMsg);
    switch (message.getMsgIdent()) {
        case IGPDatagram::ServerMsgInformID: {
            IGPDatagram::ServerMsgInformIDDatagram informIDMessage(message);
            igpIdent = informIDMessage.getIgpIdent();
            enabled = true;
            //printf("Obtenu info sur id: %d\n", informIDMessage.getIgpIdent());
            break;
        }
        case IGPDatagram::ServerMsgToClient: {
            IGPDatagram::ServerMsgToClientDatagram msgReceived(message);
            //printf("Notification clients\n");
            for (int i = 0, j = listeners.size() ; i < j ; i++) {
                IGPClientMessageListener *currentListener = listeners[i];
                currentListener->onMessage(msgReceived.getIgpMessage(), msgReceived.getIgpOriginIdent(), msgReceived.getIgpDestinationIdent());
            }
            break;
        }
        case IGPDatagram::IgpPing: {
            for (int i = 0 ; i < pendingPingTransactions.size() ; i++) {
                PingTransaction *currentTransaction = pendingPingTransactions[i];
                currentTransaction->m_completed = true;
                currentTransaction->m_success = true;
                currentTransaction->m_time = getTimeMs() - currentTransaction->m_initialTime;
                pendingPingTransactions.removeAt(i);
            }
            break;
        }
        default:
            break;
    }
}

void IGPClient::addListener(IGPClientMessageListener *newListener) {
    listeners.add(newListener);
}

void IGPClient::removeListener(IGPClientMessageListener *listener) {
    listeners.remove(listener);
}

void IGPClient::sendPingMessage(int sequenceNumber) const
{
    Message *pingMsg = mbox.createMessage();
    pingMsg->addInt(IGPDatagram::MSGIDENT, IGPDatagram::IgpPing);
    pingMsg->addInt("SN", sequenceNumber);
    pingMsg->send();
    delete pingMsg;
}


IGPClient::PingTransaction::PingTransaction(double initialTime, double timeoutMs, double interval, IGPClient *owner)
  : m_completed(false), m_success(false), m_time(0.), m_initialTime(initialTime),
    m_timeoutMs(initialTime + timeoutMs), m_interval(interval), m_owner(owner), m_sequenceNumber(0)
{
    m_owner->sendPingMessage(m_sequenceNumber++);
    m_nextRepeatTime = initialTime + interval;
}

IGPClient::PingTransaction::~PingTransaction()
{
    if (m_owner != NULL)
        m_owner->unregisterPingTransaction(this);
}

void IGPClient::PingTransaction::idle(double currentTime)
{
    if (m_timeoutMs < currentTime) {
        m_completed = true;
        m_success = false;
        m_owner->unregisterPingTransaction(this);
    }
    else if ((m_interval != 0.) && (m_nextRepeatTime < currentTime)) {
        m_owner->sendPingMessage(m_sequenceNumber++);
        m_nextRepeatTime = currentTime + m_interval;
    }
}

}
