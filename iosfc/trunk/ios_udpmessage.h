#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

#include "ios_standardmessage.h"
#include "ios_udpmessagebox.h"

namespace ios_fc {

  class UDPMessage : public StandardMessage {
    public:
      UDPMessage(int serialID, UDPMessageBox &owner);
      UDPMessage(const Buffer<char> serialized, UDPMessageBox &owner) throw(InvalidMessageException);
      virtual ~UDPMessage();
      void sendBuffer(Buffer<char> out) const;
      
    private:
      UDPMessageBox &owner;
  };

};

#endif
