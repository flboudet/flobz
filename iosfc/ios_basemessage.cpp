#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_basemessage.h"

namespace ios_fc {

template <typename T> class Value : public BaseMessage::ValueInterface
{
  public:
    Value (const T t, BaseMessage::ValueType type)
        : ValueInterface(type), t(t) {}
    virtual ~Value<T>() {}
    const T  getValue() const { return this->t; }
  private:
    const T t;
};

class ValueInt : public Value<int> {
  public:
    ValueInt(int i) : Value<int>(i, BaseMessage::INTEGER) {}
};

class ValueBool : public Value<bool> {
  public:
    ValueBool(int b) : Value<bool>(b, BaseMessage::BOOLEAN) {}
};

class ValueFloat : public Value<double> {
  public:
    ValueFloat(double f) : Value<double>(f, BaseMessage::FLOAT) {}
};

class ValueString : public Value<String> {
  public:
    ValueString(const String s) : Value<String>(s, BaseMessage::STRING) {}
};

class ValueIntArray : public Value<Buffer<int> > {
  public:
    ValueIntArray(const Buffer<int> array) : Value<Buffer<int> >(array, BaseMessage::INT_ARRAY) {}
};

class ValueCharArray : public Value<Buffer<char> > {
  public:
    ValueCharArray(const Buffer<char> array) : Value<Buffer<char> >(array, BaseMessage::CHAR_ARRAY) {}
};

/* class BaseMessage */

BaseMessage::BaseMessage()
{
}

BaseMessage::~BaseMessage()
{
}

void BaseMessage::addInt(const String &key, int value)
{
    datas[(const char *)key] = ValueInt(value);
}

void BaseMessage::addBool(const String &key, bool value)
{
    datas[(const char *)key] = ValueBool(value);
}

void BaseMessage::addFloat(const String &key, double value)
{
    datas[(const char *)key] = ValueFloat(value);
}

void BaseMessage::addString(const String &key, const String &value)
{
    datas[(const char *)key] = ValueString(value);
}

void BaseMessage::addIntArray(const String &key, const Buffer<int> &value)
{
    datas[(const char *)key] = ValueIntArray(value);
}

void BaseMessage::addCharArray(const String &key, const Buffer<char> &value)
{
    datas[(const char *)key] = ValueCharArray(value);
}

void BaseMessage::addIntProperty(const String &key, int value)
{
    intProperties[(const char *)key] = value;
}

void BaseMessage::addBoolProperty(const String &key, bool property)
{
    intProperties[(const char *)key] = (property ? 1 : 0);
}

static const BaseMessage::ValueInterface *getInterfaceAndCheckType(const std::map<std::string, BaseMessage::ValueInterface>  &datas,
                                                      const String key,
                                                      BaseMessage::ValueType type,
                                                      const String stype)
{
    auto hval = datas.find((const char *)key);
    if (hval == datas.end()) {
        throw BaseMessage::DataException(key + " does not exists");
    }

    if (hval->second.getType() != type) {
        throw BaseMessage::DataException(key + " is not " + stype);
    }
    return &(hval->second);
}

bool BaseMessage::hasInt(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}

bool BaseMessage::hasBool(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}

bool BaseMessage::hasFloat(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}

bool BaseMessage::hasString(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}

bool BaseMessage::hasIntArray(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}

bool BaseMessage::hasCharArray(const String &key) const
{
    return datas.find((const char *)key) != datas.end();
}


bool BaseMessage::hasInt       (const String key, int value)    const
{
    return hasInt(key) && (getInt(key) == value);
}

bool BaseMessage::hasFloat      (const String key, double value)   const
{
    return hasFloat(key) && (getFloat(key) == value);
}

bool BaseMessage::hasBool      (const String key, bool value)   const
{
    return hasBool(key) && (getBool(key) == value);
}

bool BaseMessage::hasString    (const String key, String value) const
{
    return hasString(key) && (getString(key) == value);
}


int BaseMessage::getInt      (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INTEGER, "Integer");

  const ValueInt* val_int = dynamic_cast<const ValueInt*>(val_interface);
  if (val_int == NULL)
    throw DataException(key + " is not an INTEGER");

  return val_int->getValue();
}

bool BaseMessage::getBool     (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, BOOLEAN, "Boolean");

  const ValueBool* val_bool = dynamic_cast<const ValueBool*>(val_interface);
  if (val_bool == NULL)
    throw DataException(key + " is not a BOOLEAN");

  return val_bool->getValue();
}

double BaseMessage::getFloat     (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, FLOAT, "Floatean");

  const ValueFloat* val_double = dynamic_cast<const ValueFloat*>(val_interface);
  if (val_double == NULL)
    throw DataException(key + " is not a FLOAT");

  return val_double->getValue();
}

const String BaseMessage::getString   (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, STRING, "String");

  const ValueString* val_string = dynamic_cast<const ValueString*>(val_interface);
  if (val_string == NULL)
    throw DataException(key + " is not a STRING");

  return val_string->getValue();
}

const Buffer<int> BaseMessage::getIntArray (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INT_ARRAY, "IntArray");

  const ValueIntArray* val_intarray = dynamic_cast<const ValueIntArray*>(val_interface);
  if (val_intarray == NULL)
    throw DataException(key + " is not an IntArray");

  return val_intarray->getValue();
}

const Buffer<char> BaseMessage::getCharArray (const String &key) const
{
  const ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, CHAR_ARRAY, "CharArray");

  const ValueCharArray* val_chararray = dynamic_cast<const ValueCharArray*>(val_interface);
  if (val_chararray == NULL)
    throw DataException(key + " is not a CharArray");

  return val_chararray->getValue();
}


int BaseMessage::getIntProperty(const String &key) const
{
    auto hval = intProperties.find((const char *)key);
    if (hval == intProperties.end()) {
        throw PropertyException(String("No such property '") + key + "'");
    }
    return hval->second;
}

bool BaseMessage::getBoolProperty(const String &key) const
{
    auto hval = intProperties.find((const char *)key);
    if (hval == intProperties.end()) {
        throw PropertyException(String("No such property '") + key + "'");
    }
    return (hval->second == 1);
}

bool BaseMessage::hasIntProperty    (const String &key) const
{
    auto hval = intProperties.find((const char *)key);
    return (hval != intProperties.end());
}

bool BaseMessage::hasBoolProperty   (const String &key) const
{
    auto hval = intProperties.find((const char *)key);
    return (hval != intProperties.end());
}

}
