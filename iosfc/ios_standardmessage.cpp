#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_standardmessage.h"

namespace ios_fc {

  static const std::string _INTEGER    = "I";
  static const std::string _BOOLEAN    = "B";
  static const std::string _FLOAT      = "F";
  static const std::string _STRING     = "S";
  static const std::string _INT_ARRAY  = "A";
  static const std::string _CHAR_ARRAY  = "C";
  static const std::string _PARAM_INTEGER = "PI";
  static const std::string _PARAM_BOOLEAN = "PB";

  static const std::string _SERIAL_ID   = "SID";
  static const std::string _IS_RELIABLE = "RELIABLE";

  StandardMessage::StandardMessage(int serialID)
    : serialized()
  {
    addIntProperty(_SERIAL_ID, serialID);
  }

  StandardMessage::~StandardMessage()
  {
    for (int i=0;i<serialized.size();++i)
      delete serialized[i];
  }

  void StandardMessage::addInt(const std::string &key, int value)
  {
    BaseMessage::addInt(key,value);
    serialized.add(new std::string(key + ":" + _INTEGER + ":" + std::to_string(value)));
  }

  void StandardMessage::addBool(const std::string &key, bool value)
  {
    BaseMessage::addBool(key,value);
    serialized.add(new std::string(key + ":" + _BOOLEAN + ":" + (value ? "1" : "0")));
  }

  void StandardMessage::addFloat(const std::string &key, double value)
  {
    BaseMessage::addFloat(key,value);
    serialized.add(new std::string(key + ":" + _FLOAT + ":" + std::to_string(value)));
  }

  void StandardMessage::addString(const std::string &key, const std::string &value)
  {
    BaseMessage::addString(key,value);
    serialized.add(new std::string(key + ":" + _STRING + ":" + value));
  }

  void StandardMessage::addIntArray(const std::string &key, const Buffer<int> &value)
  {
    BaseMessage::addIntArray(key,value);
    std::string *s = new std::string(key + ":" + _INT_ARRAY + ":" + std::to_string(value.size()));
    for (int i=0;i<value.size();++i) {
      s->operator+=(std::string(",") + std::to_string(value[i]));
    }
    serialized.add(s);
  }

  void StandardMessage::addCharArray(const std::string &key, const Buffer<char> &value)
  {
    BaseMessage::addCharArray(key,value);
    std::string *s = new std::string(key + ":" + _CHAR_ARRAY + ":" + std::to_string(value.size()) + ",");
    for (int i=0;i<value.size();++i)
    {
      static const char *hex16[16] =
        { "0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f" };
      s->operator+=(hex16[(value[i] >> 4) & 0x0f]);
      s->operator+=(hex16[value[i] & 0x0f]);
    }
    serialized.add(s);
  }

  void StandardMessage::addIntProperty   (const std::string &key, int value)
  {
    BaseMessage::addIntProperty(key,value);
    serialized.add(new std::string(key + ":" + _PARAM_INTEGER + ":" + std::to_string(value)));
  }

  void StandardMessage::addBoolProperty  (const std::string &key, bool value)
  {
    BaseMessage::addBoolProperty(key,value);
    serialized.add(new std::string(key + ":" + _PARAM_BOOLEAN + ":" + (value ? "1" : "0")));
  }

  VoidBuffer StandardMessage::serialize()
  {
    std::string out = "";
    for (int i=0;i<serialized.size();++i)
    {
      out += *serialized[i] + "\n";
    }
    return VoidBuffer(out.c_str(), out.length());
  }

  StandardMessage::StandardMessage(const Buffer<char> raw)
    : serialized()
  {
    Buffer<char> tmp_buf = raw.dup();
    tmp_buf.grow(1);
    tmp_buf[raw.size()] = 0;
    std::string sraw(tmp_buf.ptr());
    int    start = 0;
    int    end   = 0;

    //printf("****RAW MSG****\n%s\n********\n", (const char *)tmp_buf);

    while (true) {
      while (sraw[end] && (sraw[end] != '\n'))
        end ++;

      std::string line = sraw.substr(start, end - start);

      if (line.length() <= 1) { checkMessage(); return; }

      int itype = 0;
      int ival  = 0;

      while (line[itype] && (line[itype] != ':')) { itype++; ival++; }
      if (!line[itype]) { checkMessage(); return; }

      ival++;
      while (line[ival] && (line[ival] != ':')) { ival++; }
      if (!line[ival]) { checkMessage(); return; }

      std::string key   = line.substr(0, itype);
      std::string type  = line.substr(itype+1, ival - itype - 1);
      std::string value = line.substr(ival+1, line.length() - ival - 1);

      if (type == _INTEGER)
          addInt(key, atoi(value.c_str()));
      else if (type == _BOOLEAN) addBool(key, atoi(value.c_str()));
      else if (type == _FLOAT) addFloat(key, (double)atof(value.c_str()));
      else if (type == _STRING)  addString(key, value);
      else if (type == _PARAM_INTEGER) addIntProperty(key, atoi(value.c_str()));
      else if (type == _PARAM_BOOLEAN) addBoolProperty(key, atoi(value.c_str()));
      else if (type == _INT_ARRAY) {
        Buffer<int> buffer(atoi(value.c_str()));
        int index = 0;
        for (int i=0; i<buffer.size(); ++i) {
          while(value[index] && (value[index] != ',')) index++;
          if (value[index] == 0)
            throw InvalidMessageException();
          index++;
          buffer[i] = atoi(value.substr(index).c_str());
        }
        addIntArray(key, buffer);
      }
      else if (type == _CHAR_ARRAY) {
        Buffer<char> buffer(atoi(value.c_str()));
        int index = 0;
        while(value[index] && (value[index] != ',')) index++;
        if (value[index] == 0)
            throw InvalidMessageException();
        if (value.size() - index != buffer.size() * 2 + 1)
            throw InvalidMessageException();
        for (int i=0; i<buffer.size(); i++) {
          char c1, c2;
          c1 = value[++index];
          c2 = value[++index];
          if ((c1 >= '0') && (c1 <= '9')) c1 -= '0';
          else c1 -= ('a' - 10);
          if ((c2 >= '0') && (c2 <= '9')) c2 -= '0';
          else c2 -= ('a' - 10);
          buffer[i] = (c1 << 4) | c2;
        }
        addCharArray(key, buffer);
      }

      if (sraw[end] == 0) { checkMessage(); return; }
      end   = end + 1;
      start = end;
    }
  }

  void StandardMessage::checkMessage()
  {
    if (!hasIntProperty(_SERIAL_ID))
      throw InvalidMessageException();
  }

}
