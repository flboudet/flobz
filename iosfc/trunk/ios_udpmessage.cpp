#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_message.h"
#include "ios_udpmessage.h"

namespace ios_fc {

  static const String INTEGER    = "I";
  static const String BOOLEAN    = "B";
  static const String STRING     = "S";
  static const String INT_ARRAY  = "A";
  static const String PARAM_INTEGER = "PI";
  static const String PARAM_BOOLEAN = "PB";
  
  static const String SERIAL_ID   = "SID";
  static const String IS_RELIABLE = "RELIABLE";

  UDPMessage::UDPMessage(int serialID, UDPMessageBox &owner) : owner(owner)
  {
    addIntProperty(SERIAL_ID, serialID);
  }

  UDPMessage::~UDPMessage()
  {
    for (int i=0;i<serialized.size();++i)
      delete serialized[i];
  }

  bool UDPMessage::isReliable() const
  {
    if (hasBoolProperty(IS_RELIABLE))
      return getBoolProperty(IS_RELIABLE);
    return false;
  }
  
  int  UDPMessage::getSerialID() const
  {
    return getIntProperty(SERIAL_ID);
  }
  
  void UDPMessage::send() const
  {
    Buffer<char> out = serialize();
    owner.sendUDP(out, getSerialID(), isReliable());
  }

  void UDPMessage::addInt(const String key, int value)
  {
    Message::addInt(key,value);
    serialized.add(new String(key + ":" + INTEGER + ":" + value));
  }
  
  void UDPMessage::addBool(const String key, bool value)
  {
    Message::addBool(key,value);
    serialized.add(new String(key + ":" + BOOLEAN + ":" + value));
  }
  
  void UDPMessage::addString(const String key, const String value)
  {
    Message::addString(key,value);
    serialized.add(new String(key + ":" + STRING + ":" + value));
  }
  
  void UDPMessage::addIntArray(const String key, const Buffer<int> value)
  {
    Message::addIntArray(key,value);
    String *s = new String(key + ":" + INT_ARRAY + ":" + value.size());
    for (int i=0;i<value.size();++i)
    {
      s->operator+=(String(",") + value[i]);
    }
    serialized.add(s);
  }
  
  void UDPMessage::addIntProperty   (const String key, const int value)
  {
    Message::addIntProperty(key,value);
    serialized.add(new String(key + ":" + PARAM_INTEGER + ":" + value));
  }
  
  void UDPMessage::addBoolProperty  (const String key, const bool value)
  {
    Message::addBoolProperty(key,value);
    serialized.add(new String(key + ":" + PARAM_BOOLEAN + ":" + value));
  }
      
  const Buffer<char> UDPMessage::serialize() const
  {
    String out = "";
    for (int i=0;i<serialized.size();++i)
    {
      out += *serialized[i] + "\n";
    }
    return Buffer<char>((const char*)out, out.length());
  }

  UDPMessage::UDPMessage(const Buffer<char> raw, UDPMessageBox &owner)  throw(InvalidMessageException)
    : owner(owner)
  {
    String sraw((const char *)raw);
    int    start = 0;
    int    end   = 0;
   
    while (true) { 
      while (sraw[end] && (sraw[end] != '\n'))
        end ++;

      String line = sraw.substring(start, end);

      if (line.length() <= 1) { checkMessage(); return; }

      int itype = 0;
      int ival  = 0;

      while (line[itype] && (line[itype] != ':')) { itype++; ival++; }
      if (!line[itype]) { checkMessage(); return; }
      
      ival++;
      while (line[ival] && (line[ival] != ':')) { ival++; }
      if (!line[ival]) { checkMessage(); return; }

      String key   = line.substring(0,itype);
      String type  = line.substring(itype+1, ival);
      String value = line.substring(ival+1, line.length());

      if (type == INTEGER) addInt(key, atoi(value));
      else if (type == BOOLEAN) addBool(key, atoi(value));
      else if (type == STRING)  addString(key, value);
      else if (type == PARAM_INTEGER) addIntProperty(key, atoi(value));
      else if (type == PARAM_BOOLEAN) addBoolProperty(key, atoi(value));
      else if (type == INT_ARRAY) {
        Buffer<int> buffer(atoi(value));
        int index = 0;
        for (int i=0; i<buffer.size(); ++i) {
          while(value[index] != ',') index++;
          index++;
          buffer[i] = atoi(value.substring(index));
        }
        addIntArray(key, buffer);
      }

      if (sraw[end] == 0) { checkMessage(); return; }
      end   = end + 1;
      start = end;
    }
  }
  
  void UDPMessage::checkMessage() throw(InvalidMessageException)
  {
    if (!hasIntProperty(SERIAL_ID))
      throw InvalidMessageException();
  }

};
