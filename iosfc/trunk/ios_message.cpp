#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_message.h"

namespace ios_fc {

enum ValueType {
  INTEGER = 1,
  BOOLEAN = 2,
  STRING  = 3,
  INT_ARRAY = 4,
  BYTE_ARRAY = 5
};
  
class ValueInterface
{
  protected:
    ValueType type;
  public:
    virtual ~ValueInterface() {}
    ValueInterface(ValueType t) : type(t) {}
    ValueType getType() const { return type; }
};
  
template <typename T> class Value : public ValueInterface
{
  public:
    Value (const T t, ValueType type) : t(t),ValueInterface(type) {}
    virtual ~Value<T>() {}
    const T  getValue() const { return this->t; }
  private:
    const T t;
};

class ValueInt : public Value<int> {
  public:
    ValueInt(int i) : Value<int>(i, INTEGER) {}
};

class ValueBool : public Value<bool> {
  public:
    ValueBool(int b) : Value<bool>(b, BOOLEAN) {}
};

class ValueString : public Value<String> {
  public:
    ValueString(const String s) : Value<String>(s, STRING) {}
};

/* class Message */

Message::Message()  {}
Message::~Message() {}

void Message::addInt       (const String key, int value)
{
  datas.put(key, new ValueInt(value));
}

void Message::addBool      (const String key, bool value)
{
  datas.put(key, new ValueBool(value));
}

void Message::addString    (const String key, const String value)
{
  datas.put(key, new ValueString(value));
}

void Message::addIntArray  (const String key, const Buffer<int> value)
{
  printf("TODO\n");
}

void Message::addByteArray (const String key, const Buffer<char> value)
{
  printf("TODO\n");
}

void Message::addIntProperty   (const String key, const int value)
{
  printf("TODO\n");
}

void Message::addBoolProperty  (const String key, const bool property)
{
  printf("TODO\n");
}

void Message::addStringProperty(const String key, const String value)
{
  printf("TODO\n");
}

static ValueInterface *getInterfaceAndCheckType(const HashMap &datas,
                                                const String key,
                                                ValueType type,
                                                const String stype) throw(Message::DataException)
{
  HashValue *hval = datas.get(key);
  if (hval == NULL)
    throw Message::DataException(key + " does not exists");

  ValueInterface *val_interface = static_cast<ValueInterface*>(hval->ptr);

  if (val_interface == NULL)
    throw Message::DataException(key + " has no value");

  if (val_interface->getType() != type)
    throw Message::DataException(key + " is not " + stype);

  return val_interface;
}

int Message::getInt      (const String key) const throw(DataException)
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INTEGER, "Integer");

  ValueInt* val_int = dynamic_cast<ValueInt*>(val_interface);
  if (val_int == NULL)
    throw DataException(key + " is not an INTEGER");

  return val_int->getValue();
}

bool Message::getBool     (const String key) const throw(DataException)
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, BOOLEAN, "Boolean");

  ValueBool* val_bool = dynamic_cast<ValueBool*>(val_interface);
  if (val_bool == NULL)
    throw DataException(key + " is not an INTEGER");

  return val_bool->getValue();
}

const String Message::getString   (const String key) const throw(DataException)
{
  printf("TODO\n");
}

const Buffer<int> Message::getIntArray (const String key) const throw(DataException)
{
  printf("TODO\n");
}

const Buffer<char> Message::getByteArray(const String key) const throw(DataException)
{
  printf("TODO\n");
}


int Message::getIntProperty(const String key) const throw(PropertyException)
{
  printf("TODO\n");
}

bool         Message::getBoolProperty   (const String key) const throw(PropertyException)
{
  printf("TODO\n");
}

const String Message::getStringProperty (const String key) const throw(PropertyException)
{
  printf("TODO\n");
}

};
