#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_basemessage.h"

namespace ios_fc {

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
    virtual ~ValueInterface() {}
    ValueInterface(ValueType t) : type(t) {}
    ValueType getType() const { return type; }
};

template <typename T> class Value : public ValueInterface
{
  public:
    Value (const T t, ValueType type) : ValueInterface(type), t(t) {}
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

class ValueFloat : public Value<double> {
  public:
    ValueFloat(double f) : Value<double>(f, FLOAT) {}
};

class ValueString : public Value<String> {
  public:
    ValueString(const String s) : Value<String>(s, STRING) {}
};

class ValueIntArray : public Value<Buffer<int> > {
  public:
    ValueIntArray(const Buffer<int> array) : Value<Buffer<int> >(array, INT_ARRAY) {}
};

class ValueCharArray : public Value<Buffer<char> > {
  public:
    ValueCharArray(const Buffer<char> array) : Value<Buffer<char> >(array, CHAR_ARRAY) {}
};

/* class BaseMessage */

BaseMessage::BaseMessage() : datas(), intProperties()
{
}

class DeleteAction : public HashMapAction {
  public:
    void action(HashValue *value) { delete static_cast<ValueInterface*>(value->ptr); }
};

BaseMessage::~BaseMessage()
{
  DeleteAction act;
  datas.foreach(&act);
}

void BaseMessage::addInt       (const String &key, int value)
{
  datas.put(key, new ValueInt(value));
}

void BaseMessage::addBool      (const String &key, bool value)
{
  datas.put(key, new ValueBool(value));
}

void BaseMessage::addFloat      (const String &key, double value)
{
  datas.put(key, new ValueFloat(value));
}

void BaseMessage::addString    (const String &key, const String &value)
{
  datas.put(key, new ValueString(value));
}

void BaseMessage::addIntArray  (const String &key, const Buffer<int> &value)
{
  datas.put(key, new ValueIntArray(value));
}

void BaseMessage::addCharArray  (const String &key, const Buffer<char> &value)
{
  datas.put(key, new ValueCharArray(value));
}

void BaseMessage::addIntProperty   (const String &key, int value)
{
  intProperties.put(key, value);
}

void BaseMessage::addBoolProperty  (const String &key, bool property)
{
  intProperties.put(key, (int)property);
}

static ValueInterface *getInterfaceAndCheckType(const HashMap &datas,
                                                const String key,
                                                ValueType type,
                                                const String stype)
{
  HashValue *hval = datas.get(key);
  if (hval == NULL)
    throw BaseMessage::DataException(key + " does not exists");

  ValueInterface *val_interface = static_cast<ValueInterface*>(hval->ptr);

  if (val_interface == NULL)
    throw BaseMessage::DataException(key + " has no value");

  if (val_interface->getType() != type)
    throw BaseMessage::DataException(key + " is not " + stype);

  return val_interface;
}

bool BaseMessage::hasInt   (const String &key) const
{
  return datas.get(key) != NULL;
}

bool BaseMessage::hasBool      (const String &key) const
{
  return datas.get(key) != NULL;
}

bool BaseMessage::hasFloat      (const String &key) const
{
  return datas.get(key) != NULL;
}

bool BaseMessage::hasString    (const String &key) const
{
  return datas.get(key) != NULL;
}

bool BaseMessage::hasIntArray  (const String &key) const
{
  return datas.get(key) != NULL;
}

bool BaseMessage::hasCharArray  (const String &key) const
{
  return datas.get(key) != NULL;
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
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INTEGER, "Integer");

  ValueInt* val_int = dynamic_cast<ValueInt*>(val_interface);
  if (val_int == NULL)
    throw DataException(key + " is not an INTEGER");

  return val_int->getValue();
}

bool BaseMessage::getBool     (const String &key) const
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, BOOLEAN, "Boolean");

  ValueBool* val_bool = dynamic_cast<ValueBool*>(val_interface);
  if (val_bool == NULL)
    throw DataException(key + " is not a BOOLEAN");

  return val_bool->getValue();
}

double BaseMessage::getFloat     (const String &key) const
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, FLOAT, "Floatean");

  ValueFloat* val_double = dynamic_cast<ValueFloat*>(val_interface);
  if (val_double == NULL)
    throw DataException(key + " is not a FLOAT");

  return val_double->getValue();
}

const String BaseMessage::getString   (const String &key) const
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, STRING, "String");

  ValueString* val_string = dynamic_cast<ValueString*>(val_interface);
  if (val_string == NULL)
    throw DataException(key + " is not a STRING");

  return val_string->getValue();
}

const Buffer<int> BaseMessage::getIntArray (const String &key) const
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, INT_ARRAY, "IntArray");

  ValueIntArray* val_intarray = dynamic_cast<ValueIntArray*>(val_interface);
  if (val_intarray == NULL)
    throw DataException(key + " is not an IntArray");

  return val_intarray->getValue();
}

const Buffer<char> BaseMessage::getCharArray (const String &key) const
{
  ValueInterface *val_interface = getInterfaceAndCheckType(datas, key, CHAR_ARRAY, "CharArray");

  ValueCharArray* val_chararray = dynamic_cast<ValueCharArray*>(val_interface);
  if (val_chararray == NULL)
    throw DataException(key + " is not a CharArray");

  return val_chararray->getValue();
}


int BaseMessage::getIntProperty(const String &key) const
{
  HashValue *hval = intProperties.get(key);
  if (hval == NULL)
    throw PropertyException(String("No such property '") + key + "'");
  return hval->i;
}

bool         BaseMessage::getBoolProperty   (const String &key) const
{
  HashValue *hval = intProperties.get(key);
  if (hval == NULL)
    throw PropertyException(String("No such property '") + key + "'");
  return hval->i;
}

bool BaseMessage::hasIntProperty    (const String &key) const
{
  return intProperties.get(key) != NULL;
}

bool BaseMessage::hasBoolProperty   (const String &key) const
{
  return intProperties.get(key) != NULL;
}

}

