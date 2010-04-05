#ifndef _IOS_MESSAGE_H
#define _IOS_MESSAGE_H

#include "ios_memory.h"
#include "ios_newmessage.h"
#include "ios_hash.h"

namespace ios_fc {


class BaseMessage : public NewMessage
{
  public:

    virtual ~BaseMessage();

    virtual void addInt       (const String &key, int value);
    virtual void addBool      (const String &key, bool value);
    virtual void addFloat     (const String &key, double value);
    virtual void addString    (const String &key, const String &value);
    virtual void addIntArray  (const String &key, const Buffer<int> &value);
    virtual void addCharArray (const String &key, const Buffer<char> &value);

    bool hasInt       (const String &key) const;
    bool hasBool      (const String &key) const;
    bool hasFloat     (const String &key) const;
    bool hasString    (const String &key) const;
    bool hasIntArray  (const String &key) const;
    bool hasCharArray (const String &key) const;

    bool hasInt       (const String key, int value)    const;
    bool hasBool      (const String key, bool value)   const;
    bool hasFloat     (const String key, double value)  const;
    bool hasString    (const String key, String value) const;

    int                getInt       (const String &key) const;
    bool               getBool      (const String &key) const;
    double             getFloat     (const String &key) const;
    const String       getString    (const String &key) const;
    const Buffer<int>  getIntArray  (const String &key) const;
    const Buffer<char> getCharArray (const String &key) const;

    virtual void addIntProperty   (const String &key, int value);
    virtual void addBoolProperty  (const String &key, bool value);

    bool hasIntProperty   (const String &key) const;
    bool hasBoolProperty  (const String &key) const;

    int  getIntProperty   (const String &key) const;
    bool getBoolProperty  (const String &key) const;

    virtual void send() = 0;

  protected:
    BaseMessage();

  private:
    HashMap datas;
    HashMap intProperties;
};

}

#endif

