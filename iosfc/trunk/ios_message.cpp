#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_message.h"

namespace ios_fc {

enum ValueType {
  INTEGER = 1,
  BOOLEAN = 2,
  STRING  = 3,
  INT_ARRAY = 4
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

class ValueIntArray : public Value<Buffer<int> > {
  public:
    ValueIntArray(const Buffer<int> array) : Value<Buffer<int> >(array, INT_ARRAY) {}
};

/* class Message */

Message::Message()
{
}

Message::~Message()
{
  /* TODO: delete hashmap content */
}

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
  datas.put(key, new ValueIntArray(value));
}

void Message::addIntProperty   (const String key, const int value)
{
  intProperties.put(key, value);
}

void Message::addBoolProperty  (const String key, const bool property)
{
  intProperties.put(key, (int)property);
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
    throw DataException(key + " is not a BOOLEAN");

  return val_bool->getValue();
}

const String Message::getString   (const String key) const throw(DataException)
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, STRING, "String");

  ValueString* val_string = dynamic_cast<ValueString*>(val_interface);
  if (val_string == NULL)
    throw DataException(key + " is not a STRING");

  return val_string->getValue();
}

const Buffer<int> Message::getIntArray (const String key) const throw(DataException)
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INT_ARRAY, "IntArray");

  ValueIntArray* val_intarray = dynamic_cast<ValueIntArray*>(val_interface);
  if (val_intarray == NULL)
    throw DataException(key + " is not an IntArray");

  return val_intarray->getValue();
}


int Message::getIntProperty(const String key) const throw(PropertyException)
{
  HashValue *hval = intProperties.get(key);
  if (hval == NULL)
    throw PropertyException(String("No such property '") + key + "'");
  return hval->i;
}

bool         Message::getBoolProperty   (const String key) const throw(PropertyException)
{
  HashValue *hval = intProperties.get(key);
  if (hval == NULL)
    throw PropertyException(String("No such property '") + key + "'");
  return hval->i;
}

bool Message::hasIntProperty    (const String key) const
{
  return intProperties.get(key) != NULL;
}

bool Message::hasBoolProperty   (const String key) const
{
  return intProperties.get(key) != NULL;
}

};
