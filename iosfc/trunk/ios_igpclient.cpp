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
#include "ios_udpmessagebox.h"

namespace ios_fc {

IGPClient::IGPClient(String hostName, int portID) : enabled(false), mbox(new UDPMessageBox(hostName, 0, portID)), igpKeepAliveCounter(0)
{
    mbox->addListener(this);
    IGPDatagram::ClientMsgAutoAssignIDDatagram datagram(mbox->createMessage());
    datagram.getMessage()->send();
}

IGPClient::IGPClient(String hostName, int portID, int igpIdent) : enabled(false), mbox(new UDPMessageBox(hostName, 0, portID)), igpKeepAliveCounter(0)
{
    mbox->addListener(this);
    IGPDatagram::ClientMsgAssignIDDatagram datagram(mbox->createMessage(), igpIdent);
    datagram.getMessage()->send();
}

IGPClient::~IGPClient()
{
    delete mbox;
}

void IGPClient::sendMessage(int igpID, VoidBuffer message, bool reliable)
{
    IGPDatagram::ClientMsgToClientDatagram datagram(mbox->createMessage(), igpID, message, reliable);
    datagram.getMessage()->send();
}

void IGPClient::idle()
{
    igpKeepAliveCounter++;
    if (igpKeepAliveCounter == 100) {
        IGPDatagram::ClientMsgKeepAliveDatagram datagram(mbox->createMessage());
        datagram.getMessage()->send();
        igpKeepAliveCounter = 0;
    }
    mbox->idle();
}

void IGPClient::onMessage(Message &rawMsg)
{
    IGPDatagram message(&rawMsg);
    switch (message.getMsgIdent()) {
        case IGPDatagram::ServerMsgInformID: {
            IGPDatagram::ServerMsgInformIDDatagram informIDMessage(message);
            igpIdent = informIDMessage.getIgpIdent();
            enabled = true;
            printf("Obtenu info sur id: %d\n", informIDMessage.getIgpIdent());
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

}
