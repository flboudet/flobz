#ifndef _IOS_NEWMESSAGE_H
#define _IOS_NEWMESSAGE_H

#include "ios_memory.h"
#include "ios_hash.h"

namespace ios_fc {


class NewMessage
{
  public:

    class DataException : public Exception {
      public:
      DataException(const String name) : Exception(String("Error in Message Data '") + name + "'") {}
    };

    class PropertyException : public Exception {
      public:
      PropertyException(const String name) : Exception(String("Error in Message Property ") + name + "'") {}
    };

    virtual ~NewMessage() {}

    virtual VoidBuffer serialize() = 0;
    virtual void addInt       (const String &key, int value) = 0;
    virtual void addBool      (const String key, bool value) = 0;
    virtual void addFloat     (const String key, double value) = 0;
    virtual void addString    (const String &key, const String &value) = 0;
    virtual void addIntArray  (const String &key, const Buffer<int> &value) = 0;
    virtual void addCharArray (const String key, const Buffer<char> value) = 0;
#ifdef DISABLED
    bool hasInt       (const String key) const = 0;
    bool hasBool      (const String key) const = 0;
    bool hasFloat     (const String key) const = 0;
    bool hasString    (const String key) const = 0;
    bool hasIntArray  (const String key) const = 0;
    bool hasCharArray (const String key) const = 0;

    bool hasInt       (const String key, int value)    const = 0;
    bool hasBool      (const String key, bool value)   const = 0;
    bool hasFloat     (const String key, double value)  const = 0;
    bool hasString    (const String key, String value) const = 0;
#endif
    virtual int getInt       (const String &key) const = 0;
#ifdef DISABLED
    bool               getBool      (const String key) const = 0;
    double             getFloat     (const String key) const = 0;
    const String       getString    (const String key) const = 0;
    const Buffer<int>  getIntArray  (const String key) const = 0;
    const Buffer<char> getCharArray (const String key) const = 0;

    virtual void addIntProperty   (const String key, const int value) = 0;
    virtual void addBoolProperty  (const String key, const bool property) = 0;

    bool hasIntProperty   (const String key) const = 0;
    bool hasBoolProperty  (const String key) const = 0;

    int  getIntProperty   (const String key) const = 0;
    bool getBoolProperty  (const String key) const = 0;

    virtual void send() const = 0;

  protected:
    Message();

  private:
    HashMap datas;
    HashMap intProperties;
#endif
};

}

#endif

