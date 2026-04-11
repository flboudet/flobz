#ifndef _IOS_MESSAGE_H
#define _IOS_MESSAGE_H

#include "ios_memory.h"
#include "ios_message.h"
#include <map>

namespace ios_fc {


class BaseMessage : public Message
{
  public:

    virtual ~BaseMessage();

    virtual void addInt       (const std::string&key, int value);
    virtual void addBool      (const std::string &key, bool value);
    virtual void addFloat     (const std::string &key, double value);
    virtual void addString    (const std::string &key, const std::string &value);
    virtual void addIntArray  (const std::string &key, const Buffer<int> &value);
    virtual void addCharArray (const std::string &key, const Buffer<char> &value);

    bool hasInt       (const std::string &key) const;
    bool hasBool      (const std::string &key) const;
    bool hasFloat     (const std::string &key) const;
    bool hasString    (const std::string &key) const;
    bool hasIntArray  (const std::string &key) const;
    bool hasCharArray (const std::string &key) const;

    bool hasInt       (const std::string &key, int value)    const;
    bool hasBool      (const std::string &key, bool value)   const;
    bool hasFloat     (const std::string &key, double value)  const;
    bool hasString    (const std::string &key, const std::string & value) const;

    int                getInt       (const std::string &key) const;
    bool               getBool      (const std::string &key) const;
    double             getFloat     (const std::string &key) const;
    const std::string  getString    (const std::string &key) const;
    const Buffer<int>  getIntArray  (const std::string &key) const;
    const Buffer<char> getCharArray (const std::string &key) const;

    virtual void addIntProperty   (const std::string &key, int value);
    virtual void addBoolProperty  (const std::string &key, bool value);

    bool hasIntProperty   (const std::string &key) const;
    bool hasBoolProperty  (const std::string &key) const;

    int  getIntProperty   (const std::string &key) const;
    bool getBoolProperty  (const std::string &key) const;

  protected:
    BaseMessage();

public:
    enum ValueType {
        INTEGER    = 1,
        BOOLEAN    = 2,
        STRING     = 3,
        INT_ARRAY  = 4,
        CHAR_ARRAY = 5,
        FLOAT      = 6
    };

    class ValueInterface
    {
    protected:
        ValueType type;
    public:
        ValueInterface() {}
        virtual ~ValueInterface() {}
        ValueInterface(ValueType t) : type(t) {}
        ValueType getType() const { return type; }
    };

private:
    std::map<std::string, ValueInterface> datas;
    std::map<std::string, int> intProperties;
};

}

#endif
