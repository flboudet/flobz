#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_standardmessage.h"

namespace ios_fc {

  static const String INTEGER    = "I";
  static const String BOOLEAN    = "B";
  static const String STRING     = "S";
  static const String INT_ARRAY  = "A";
  static const String PARAM_INTEGER = "PI";
  static const String PARAM_BOOLEAN = "PB";
  
  static const String SERIAL_ID   = "SID";
  static const String IS_RELIABLE = "RELIABLE";

  StandardMessage::StandardMessage(int serialID)
  {
    addIntProperty(SERIAL_ID, serialID);
  }

  StandardMessage::~StandardMessage()
  {
    for (int i=0;i<serialized.size();++i)
      delete serialized[i];
  }

  bool StandardMessage::isReliable() const
  {
    if (hasBoolProperty(IS_RELIABLE))
      return getBoolProperty(IS_RELIABLE);
    return false;
  }
  
  int  StandardMessage::getSerialID() const
  {
    return getIntProperty(SERIAL_ID);
  }
  
  void StandardMessage::send() const
  {
    Buffer<char> out = serialize();
    sendBuffer(out);
  }

  void StandardMessage::addInt(const String key, int value)
  {
    Message::addInt(key,value);
    serialized.add(new String(key + ":" + INTEGER + ":" + value));
  }
  
  void StandardMessage::addBool(const String key, bool value)
  {
    Message::addBool(key,value);
    serialized.add(new String(key + ":" + BOOLEAN + ":" + value));
  }
  
  void StandardMessage::addString(const String key, const String value)
  {
    Message::addString(key,value);
    serialized.add(new String(key + ":" + STRING + ":" + value));
  }
  
  void StandardMessage::addIntArray(const String key, const Buffer<int> value)
  {
    Message::addIntArray(key,value);
    String *s = new String(key + ":" + INT_ARRAY + ":" + value.size());
    for (int i=0;i<value.size();++i)
    {
      s->operator+=(String(",") + value[i]);
    }
    serialized.add(s);
  }
  
  void StandardMessage::addIntProperty   (const String key, const int value)
  {
    Message::addIntProperty(key,value);
    serialized.add(new String(key + ":" + PARAM_INTEGER + ":" + value));
  }
  
  void StandardMessage::addBoolProperty  (const String key, const bool value)
  {
    Message::addBoolProperty(key,value);
    serialized.add(new String(key + ":" + PARAM_BOOLEAN + ":" + value));
  }
      
  const Buffer<char> StandardMessage::serialize() const
  {
    String out = "";
    for (int i=0;i<serialized.size();++i)
    {
      out += *serialized[i] + "\n";
    }
    return Buffer<char>((const char*)out, out.length());
  }

  StandardMessage::StandardMessage(const Buffer<char> raw)  throw(InvalidMessageException)
  {
    Buffer<char> tmp_buf = raw.dup();
    tmp_buf.grow(1);
    tmp_buf[raw.size()] = 0;
    String sraw((const char *)tmp_buf);
    int    start = 0;
    int    end   = 0;
   
    printf("****RAW MSG****\n%s\n********\n", (const char *)tmp_buf);
    
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
          while(value[index] && (value[index] != ',')) index++;
          if (value[index] == 0)
            throw InvalidMessageException();
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
  
  void StandardMessage::checkMessage() throw(InvalidMessageException)
  {
    if (!hasIntProperty(SERIAL_ID))
      throw InvalidMessageException();
  }

};
