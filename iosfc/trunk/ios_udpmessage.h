#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

#include "ios_standardmessage.h"
#include "ios_udpmessagebox.h"
#include "ios_dirigeable.h"

namespace ios_fc {
    
    class UDPMessage : public StandardMessage, public Dirigeable {
    public:
        UDPMessage(int serialID, UDPMessageBox &owner);
        UDPMessage(const Buffer<char> serialized, UDPMessageBox &owner) throw(InvalidMessageException);
        virtual ~UDPMessage();
        void sendBuffer(Buffer<char> out) const;
        // Dirigeable implementation
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
    private:
        UDPMessageBox &owner;
    };
    
};

#endif
