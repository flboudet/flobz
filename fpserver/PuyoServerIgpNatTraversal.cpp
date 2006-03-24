/* -*-mode:c; c-style:k&r; c-basic-offset:4; indent-tab-mode: nil; -*- */
#include "ios_igpmessage.h"
#include "ios_udpmessage.h"
#include "PuyoServerIgpNatTraversal.h"
#include "PuyoIgpDefs.h"

void PuyoIgpNatTraversal::onMessage(Message &msg)
{
    //printf("Message recu!\n");
    switch (msg.getInt("CMD")) {
    case PUYO_IGP_NAT_TRAVERSAL: {
	Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(msg);
	PeerAddress address = dirMsg.getPeerAddress();
	IgpMessage::IgpPeerAddressImpl *impl =
	    static_cast<IgpMessage::IgpPeerAddressImpl *>(address.getImpl());
	printf("Demande de peeraddress udp du peer igp %d\n", impl->getIgpIdent());
	UDPPeerAddress udpAddress = igpListener.getPeerAddress(impl->getIgpIdent());
	printf("Peer:  %s:%d\n", (const char *)(udpAddress.getSocketAddress().asString()), udpAddress.getPortNum());
	printf("Local address:  %s:%d\n", (const char *)(msg.getString("LSOCKADDR")), msg.getInt("LPORTNUM"));
	break;
    }
    default:
	break;
    }
}

void PuyoIgpNatTraversal::idle()
{
}
