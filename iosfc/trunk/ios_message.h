#ifndef _IOS_MESSAGE_H
#define _IOS_MESSAGE_H

#include "ios_memory.h"
#include "ios_hash.h"

namespace ios_fc {


class Message
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

    virtual ~Message();
    
    virtual void addInt       (const String key, int value);
    virtual void addBool      (const String key, bool value);
    virtual void addString    (const String key, const String value);
    virtual void addIntArray  (const String key, const Buffer<int> value);
    virtual void addCharArray (const String key, const Buffer<char> value);

    bool hasInt       (const String key) const;
    bool hasBool      (const String key) const;
    bool hasString    (const String key) const;
    bool hasIntArray  (const String key) const;
    bool hasCharArray (const String key) const;

    int                getInt       (const String key) const throw(DataException);
    bool               getBool      (const String key) const throw(DataException);
    const String       getString    (const String key) const throw(DataException);
    const Buffer<int>  getIntArray  (const String key) const throw(DataException);
    const Buffer<char> getCharArray (const String key) const throw(DataException);

    virtual void addIntProperty   (const String key, const int value);
    virtual void addBoolProperty  (const String key, const bool property);

    bool hasIntProperty   (const String key) const;
    bool hasBoolProperty  (const String key) const;

    int  getIntProperty   (const String key) const throw(PropertyException);
    bool getBoolProperty  (const String key) const throw(PropertyException);

    virtual void send() const = 0;

  protected:
    Message();
    
  private:
    HashMap datas;
    HashMap intProperties;
};

};

#endif

