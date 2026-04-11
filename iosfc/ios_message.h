#ifndef _IOS_NEWMESSAGE_H
#define _IOS_NEWMESSAGE_H

#include "ios_memory.h"
#include "ios_hash.h"
#include <string>
#include <stdexcept>

namespace ios_fc {


class Message
{
public:

    class DataException : public std::runtime_error {
    public:
        DataException(const std::string &name) : std::runtime_error(std::string("Error in Message Data '") + name.c_str() + "'") {}
    };

    class PropertyException : public std::runtime_error {
    public:
        PropertyException(const std::string &name) : std::runtime_error(std::string("Error in Message Property ") + name.c_str() + "'") {}
    };

    class InvalidMessageException : public std::runtime_error {
    public:
        InvalidMessageException() : std::runtime_error("Invalid Message") {}
    };

    virtual ~Message() {}

    virtual VoidBuffer serialize() = 0;
    virtual void addInt       (const std::string &key, int value) = 0;
    virtual void addBool      (const std::string &key, bool value) = 0;
    virtual void addFloat     (const std::string &key, double value) = 0;
    virtual void addString    (const std::string &key, const std::string &value) = 0;
    virtual void addIntArray  (const std::string &key, const Buffer<int> &value) = 0;
    virtual void addCharArray (const std::string &key, const Buffer<char> &value) = 0;

    virtual bool hasInt       (const std::string &key) const = 0;
    virtual bool hasBool      (const std::string &key) const = 0;
    virtual bool hasFloat     (const std::string &key) const = 0;
    virtual bool hasString    (const std::string &key) const = 0;
    virtual bool hasIntArray  (const std::string &key) const = 0;
    virtual bool hasCharArray (const std::string &key) const = 0;
#ifdef DISABLED
    bool hasInt       (const std::string key, int value)    const = 0;
    bool hasBool      (const std::string key, bool value)   const = 0;
    bool hasFloat     (const std::string key, double value)  const = 0;
    bool hasString    (const std::string key, const std::string value) const = 0;
#endif
    virtual int                getInt       (const std::string &key) const = 0;
    virtual bool               getBool      (const std::string &key) const = 0;
    virtual double             getFloat     (const std::string &key) const = 0;
    virtual const std::string  getString    (const std::string &key) const = 0;
    virtual const Buffer<int>  getIntArray  (const std::string &key) const = 0;
    virtual const Buffer<char> getCharArray (const std::string &key) const = 0;

    virtual void addIntProperty (const std::string &key, int value) = 0;
    virtual void addBoolProperty(const std::string &key, bool value) = 0;

    virtual bool hasIntProperty   (const std::string &key) const = 0;
    virtual bool hasBoolProperty  (const std::string &key) const = 0;

    virtual int  getIntProperty   (const std::string &key) const = 0;
    virtual bool getBoolProperty  (const std::string &key) const = 0;

    virtual void send() {}
};

}

#endif

