#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_message.h"
#include "ios_udpmessage.h"

namespace ios_fc {

  static const String INTEGER = "INTEGER";
  static const String BOOLEAN = "BOOLEAN";
  static const String STRING  = "STRING";
  static const String INT_ARRAY  = "INT_ARRAY";
  static const String SERIAL_ID   = "SERIAL_ID";
  static const String IS_RELIABLE = "IS_RELIABLE";

  UDPMessage::UDPMessage(int serialID/*, const UDPMessageBox &owner*/)
  {
    addInt(SERIAL_ID, serialID);
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
    return getInt(SERIAL_ID);
  }
  
  void UDPMessage::send() const
  {
    Buffer<char> out = serialize();
    /* owner.send(out, getSerialID(), isReliable()); */
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
  
      
  const Buffer<char> UDPMessage::serialize() const
  {
    String out = "";
    for (int i=0;i<serialized.size();++i)
    {
      out += *serialized[i] + "\n";
    }
    return Buffer<char>((const char*)out, out.length());
  }

  UDPMessage::UDPMessage(const Buffer<char> raw/*, const UDPMessageBox &owner*/)
  {
    String sraw((const char *)raw);
    int    start = 0;
    int    end   = 0;
   
    while (true) { 
      while (sraw[end] && (sraw[end] != '\n'))
        end ++;

      String line = sraw.substring(start, end);

      if (line.length() <= 1) return;

      int itype = 0;
      int ival  = 0;

      while (line[itype] && (line[itype] != ':')) { itype++; ival++; }
      if (!line[itype]) return;
      
      ival++;
      while (line[ival] && (line[ival] != ':')) { ival++; }
      if (!line[ival]) return;

      String key   = line.substring(0,itype);
      String type  = line.substring(itype+1, ival);
      String value = line.substring(ival+1, line.length());

      if (type == INTEGER) addInt(key, atoi(value));
      else if (type == BOOLEAN) addBool(key, atoi(value));
      else if (type == STRING) addBool(key, value);
      /* TODO... le reste... */

      if (sraw[end] == 0) return;
      end   = end + 1;
      start = end;
    }
  }
  

};
