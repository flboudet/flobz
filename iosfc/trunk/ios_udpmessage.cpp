#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_message.h"
#include "ios_udpmessage.h"

namespace ios_fc {

  UDPMessage::UDPMessage(int serialID/*, const UDPMessageBox &owner*/)
  {}
  UDPMessage::UDPMessage(const Buffer<char> serialized/*, const UDPMessageBox &owner*/)
  {}
  
  UDPMessage::~UDPMessage()
  {}

  bool UDPMessage::isReliable() const
  {
    return false;
  }
  
  int  UDPMessage::getSerialID() const
  {
    return 1;
  }
  
  void UDPMessage::send() const
  {
    serialize();
  }

  void UDPMessage::addInt(const String key, int value)
  {
    Message::addInt(key,value);
    serialized.add(new String(key + ":INTEGER:" + value));
  }
  
  void UDPMessage::addBool(const String key, bool value)
  {
    Message::addBool(key,value);
    serialized.add(new String(key + ":BOOL:" + value));
  }
  
  void UDPMessage::addString(const String key, const String value)
  {
    Message::addString(key,value);
    serialized.add(new String(key + ":STRING:" + value));
  }
  
  void UDPMessage::addIntArray(const String key, const Buffer<int> value)
  {
    Message::addIntArray(key,value);
  }
  
  void UDPMessage::addByteArray(const String key, const Buffer<char> value)
  {
    Message::addByteArray(key,value);
  }
      
  const Buffer<char> UDPMessage::serialize() const
  {
    AdvancedBuffer<char> out;
    for (int i=0;i<serialized.size();++i)
    {
      printf("%s\n", (const char*)*serialized[i]);
    }
    return out;
  }

};
