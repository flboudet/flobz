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

namespace ios_fc {

IGPClient::IGPClient(String hostName, int portID) : enabled(false), clientSocket(hostName, portID)
{
    IGPDatagram::ClientMsgAutoAssignIDDatagram datagram;
    clientSocket.getOutputStream()->streamWrite(datagram.serialize());
}

IGPClient::IGPClient(String hostName, int portID, int igpIdent) : enabled(false), clientSocket(hostName, portID)
{
    IGPDatagram::ClientMsgAssignIDDatagram datagram(igpIdent);
    clientSocket.getOutputStream()->streamWrite(datagram.serialize());
}

void IGPClient::sendMessage(int igpID, VoidBuffer message)
{
    IGPDatagram::ClientMsgToClientDatagram datagram(igpID, message);
    clientSocket.getOutputStream()->streamWrite(datagram.serialize());
}

void IGPClient::idle()
{
    InputStream *input = clientSocket.getInputStream();
    /*int available = input->streamAvailable();
    if (available > 0) {
        VoidBuffer data(available);
        input->streamRead(data);
        IGPDatagram message(data);
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
            printf("Notification clients\n");
            for (int i = 0, j = listeners.size() ; i < j ; i++) {
                IGPClientMessageListener *currentListener = listeners[i];
                currentListener->onMessage(msgReceived.getMessage(), msgReceived.getIgpOriginIdent(), msgReceived.getIgpDestinationIdent());
            }
            break;
        }
        default:
            break;
        }
    }*/
    int available = input->streamAvailable();
    while (available > 0) {
        printf("***Disponible:%d (%d)***\n", available, available-8);
        IGPDatagram message(input);
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
                currentListener->onMessage(msgReceived.getMessage(), msgReceived.getIgpOriginIdent(), msgReceived.getIgpDestinationIdent());
            }
            break;
        }
        default:
            break;
        }
        available = input->streamAvailable();
    }
}

void IGPClient::addListener(IGPClientMessageListener *newListener) {
    listeners.add(newListener);
}

void IGPClient::removeListener(IGPClientMessageListener *listener) {
    listeners.remove(listener);
}

};
