#ifndef _IOS_FASTMESSAGE_H_
#define _IOS_FASTMESSAGE_H_

#include <iostream>
#include <stdint.h>
#include "ios_newmessage.h"

namespace ios_fc {

enum ValueType {
    INTEGER=1,
    BOOL=2,
    FLOAT=3,
    STRING=4,
    INTEGER_ARRAY = 5,
    CHAR_ARRAY = 6,
    PROP_INTEGER=11,
    PROP_BOOL=12
};

struct HeaderRecord {
    HeaderRecord(uint16_t  type,
                 uint16_t nameOffset,
                 uint16_t valOffset)
        : type(type),
          nameOffset(nameOffset),
          valOffset(valOffset) {}
    uint16_t type;
    uint16_t nameOffset;
    uint16_t valOffset;
};

class FastMessage : public NewMessage
{
public:
    FastMessage();
    FastMessage(VoidBuffer &serializedData);

    virtual VoidBuffer serialize();
    virtual void addInt(const String &key, int value);
    virtual void addBool(const String &key, bool value);
    virtual void addFloat(const String &key, double value);
    virtual void addString(const String &key, const String &value);
    virtual void addIntArray(const String &key, const Buffer<int> &value);
    virtual void addCharArray(const String &key, const Buffer<char> &value);

    virtual int    getInt(const String &key) const;
    virtual bool   getBool(const String &key) const;
    virtual double getFloat(const String &key) const;
    virtual const String       getString    (const String &key) const;
    virtual const Buffer<int>  getIntArray  (const String &key) const;
    virtual const Buffer<char> getCharArray (const String &key) const;

    virtual bool hasInt       (const String &key) const;
    virtual bool hasBool      (const String &key) const;
    virtual bool hasFloat     (const String &key) const;
    virtual bool hasString    (const String &key) const;
    virtual bool hasIntArray  (const String &key) const;
    virtual bool hasCharArray (const String &key) const;

    virtual void addIntProperty (const String &key, int value);
    virtual void addBoolProperty(const String &key, bool value);

    virtual bool hasIntProperty   (const String &key) const;
    virtual bool hasBoolProperty  (const String &key) const;

    virtual int  getIntProperty   (const String &key) const;
    virtual bool getBoolProperty  (const String &key) const;
private:
    inline int8_t get_char8(void *src) const {
        return *(int8_t *)src;
    }
    inline uint16_t get_uint16(void *src) const {
        return *(uint16_t *)src;
    }
    inline uint32_t get_uint32(void *src) const {
        return *(uint32_t *)src;
    }
    inline double get_float64(void *src) const {
        return *(double *)src;
    }
    inline void copy_char8(void *dest, int8_t src) {
        *((int8_t *)dest) = src;
    }
    inline void copy_uint16(void *dest, uint16_t src) {
        *((uint16_t *)dest) = src;
    }
    inline void copy_uint32(void *dest, uint32_t src) {
        *((uint32_t *)dest) = src;
    }
    inline void copy_float64(void *dest, double src) {
        *((double *)dest) = src;
    }
    inline void * append_key_reserve_data(uint16_t type, const char *key, size_t dataSize) {
        // TODO: check buffer overflow
        size_t keySize = strlen(key);
        m_headerRecords[m_headerRecordsCount++]
            = HeaderRecord(type, m_dataOffset, m_dataOffset+keySize);
        Memory::memcpy((char *)m_data+m_dataOffset, (const char *)key, keySize);
        m_dataOffset += (keySize + dataSize);
        return (char *)m_data + m_dataOffset - dataSize;
    }
    inline void * get_data_for_key(const char *key, uint16_t type, bool throwNotFound = true) const {
        for (int i = 0 ; i < m_headerRecordsCount ; i++) {
            HeaderRecord &currentRecord = m_headerRecords[i];
            if (currentRecord.type != type)
                continue;
            size_t nameLength = currentRecord.valOffset
                                  - currentRecord.nameOffset;
            size_t keyLength = strlen(key);
            if (nameLength != keyLength)
                continue;
            char *address = (char *)m_data + currentRecord.nameOffset;
            if (strncmp(address, key, nameLength) == 0)
                return (char *)m_data + currentRecord.valOffset;
        }
        if (throwNotFound)
            throw DataException(String("Data not found: ") + key);
        return NULL;
    }
private:
    void         *m_header;
    HeaderRecord *m_headerRecords;
    void         *m_data;
    size_t m_headerRecordsCount;
    size_t m_dataOffset;
};

}

#endif // _IOS_FASTMESSAGE_H_

