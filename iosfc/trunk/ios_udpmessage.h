#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

namespace ios_fc {

  class UDPMessage : public Message {
    public:
      UDPMessage(int serialID/*, const UDPMessageBox &owner*/) {}
      UDPMessage(const Buffer<char> serialized/*, const UDPMessageBox &owner*/) {}
      bool isReliable() const  {return false;}
      int  getSerialID() const {return 1;}
      void send() const {}
  };

};

#endif
