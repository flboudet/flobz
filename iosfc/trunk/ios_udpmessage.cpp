#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_udpmessage.h"

namespace ios_fc {

  UDPMessage::UDPMessage(int serialID, UDPMessageBox &owner)
    : StandardMessage(serialID), owner(owner)
  {
  }

  UDPMessage::~UDPMessage()
  {
  }

  void UDPMessage::sendBuffer(Buffer<char> out) const
  {
    owner.sendUDP(out, getSerialID(), isReliable());
  }

  UDPMessage::UDPMessage(const Buffer<char> raw, UDPMessageBox &owner)  throw(InvalidMessageException)
    : StandardMessage(raw), owner(owner)
  {
  }

};
