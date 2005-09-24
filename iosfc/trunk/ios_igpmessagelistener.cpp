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
 
 #include "ios_igpmessagelistener.h"
 #include <stdio.h>
 
namespace ios_fc {

const int IgpMessageListener::firstAutoIgpIdent = 32768;

void IgpMessageListener::NetworkIgpPeer::sendIGPIdent()
{
    IGPDatagram::ServerMsgInformIDDatagram reply(createMessage(), getIgpIdent());
    Dirigeable *msg = dynamic_cast<Dirigeable *>(reply.getMessage());
    if (msg != NULL) {
        //printf("setpeeraddress\n");
        msg->setPeerAddress(this->address);
        //printf("setpeeraddress done\n");
        reply.getMessage()->send();
        //printf("envoye!\n");
    }
    else {
        printf("msg NULL!\n");
    }
}

void IgpMessageListener::NetworkIgpPeer::messageReceived(VoidBuffer message, int origIgpIdent, bool reliable)
{
    IGPDatagram::ServerMsgToClientDatagram msgToSend(createMessage(),
                                                     origIgpIdent, getIgpIdent(),
                                                     message,
                                                     reliable);
    Dirigeable *msg = dynamic_cast<Dirigeable *>(msgToSend.getMessage());
    if (msg != NULL) {
        msg->setPeerAddress(getAddress());
        msgToSend.getMessage()->send();
    }
}

void IgpMessageListener::NetworkIgpPeer::datagramFromMyself(IGPDatagram &message)
{
    switch (message.getMsgIdent()) {
    case IGPDatagram::ClientMsgAutoAssignID:
        setIgpIdent(getUniqueIGPId());
        printf("Auto-assign ID:%d\n", getIgpIdent());
        //setValid(true);
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgAssignID: {
        IGPDatagram::ClientMsgAssignIDDatagram msgReceived(message);
        printf("Assign ID\n");
        if (igpIdValidAndUnique(msgReceived.getIgpIdent())) {
            //setValid(true);
            setIgpIdent(msgReceived.getIgpIdent());
            sendIGPIdent();
        }
        else {
            printf("Adresse igp invalide:%d\n", msgReceived.getIgpIdent());
        }
        break;
    }
    case IGPDatagram::ClientMsgGetID:
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgToClient: {
        IGPDatagram::ClientMsgToClientDatagram msgReceived(message);
        bool reliable = false;
        Message *rawmsg = msgReceived.getMessage();
        //
        if (rawmsg->hasBoolProperty("RELIABLE"))
            reliable = rawmsg->getBoolProperty("RELIABLE");
        sendMessageToAddress(msgReceived.getIgpMessage(), msgReceived.getIgpIdent(), reliable);
        break;
    }
    default:
        break;
	}
}

IgpMessageListener::NetworkIgpPeer *IgpMessageListener::findPeer(PeerAddress address)
{
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        NetworkIgpPeer *networkKnownPeer = dynamic_cast<NetworkIgpPeer *>(knownPeers[i]);
        if (networkKnownPeer != NULL) {
            if (networkKnownPeer->getAddress() == address) {
                //printf("Trouve !\n");
                return networkKnownPeer;
            }
        }
    }
    return NULL;
}

void IgpMessageListener::onMessage(Message &data)
{
    //printf("Cool, un nouveau message!\n");
    Dirigeable &dir = dynamic_cast<Dirigeable &>(data);
    
    PeerAddress msgAddress = dir.getPeerAddress();
    NetworkIgpPeer *currentPeer = findPeer(msgAddress);
    if (currentPeer == NULL) {
        currentPeer = new NetworkIgpPeer(msgAddress, this);
        printf("Nouveau peer !\n");
    }
    
    IGPDatagram message(&data);
    currentPeer->datagramFromMyself(message);
}

int IgpMessageListener::getUniqueIGPId()
{
    return currentAutoIgpIdent++;
}

bool IgpMessageListener::igpIdValidAndUnique(int igpIdent)
{
    if ((igpIdent <= 0) || (igpIdent >= firstAutoIgpIdent))
        return false;
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        if (igpIdent == knownPeers[i]->getIgpIdent())
            return false;
    }
    return true;
}

IgpPeer *IgpMessageListener::getPeer(int igpIdent) const
{
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        if (igpIdent == knownPeers[i]->getIgpIdent())
            return knownPeers[i];
    }
    return NULL;
}

void IgpMessageListener::onPeerConnect(const PeerAddress &address)
{
    printf("Ca me fait une belle jambe!\n");
}

void IgpMessageListener::onPeerDisconnect(const PeerAddress &address)
{
    printf("Cool, une deconnection IGP!\n");
    NetworkIgpPeer *disconnectedPeer = findPeer(address);
    delete disconnectedPeer;
}

void IgpMessageListener::sendMessageToAddress(IgpPeer *peer, VoidBuffer igpMessage, int destIgpIdent, bool reliable)
{
    IgpPeer *destPeer = getPeer(destIgpIdent);
    if (destPeer != NULL) {
        destPeer->messageReceived(igpMessage, peer->getIgpIdent(), reliable);
    }
    else {
        printf("DEST not found!!!\n");
    }
}

}

