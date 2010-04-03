#include "ios_fastmessage.h"
#include <iostream>

using namespace std;

namespace ios_fc {

FastMessage::FastMessage()
    : m_headerRecordsCount(0), m_dataOffset(0)
{
    m_header = Memory::malloc(1024);
    m_data = Memory::malloc(1024);
    m_headerRecords = (HeaderRecord *)((char *)m_header + 2);
}

FastMessage::FastMessage(VoidBuffer &serializedData)
{
    size_t headerSize = get_uint16(serializedData);
    size_t dataSize = serializedData.size() - headerSize;

    m_header = Memory::malloc(headerSize);
    m_data = Memory::malloc(dataSize);

    Memory::memcpy(m_header, serializedData, headerSize);
    Memory::memcpy(m_data, (char *)serializedData + headerSize, dataSize);
    m_headerRecords = (HeaderRecord *)((char *)m_header + 2);

    m_headerRecordsCount = (headerSize-2) / sizeof(HeaderRecord);
}

void FastMessage::addInt(const String &key, int value)
{
    copy_uint32(append_key_reserve_data(INTEGER, key, 4), value);
}

void FastMessage::addBool(const String key, bool value)
{
    copy_char8(append_key_reserve_data(INTEGER, key, 1), value?0xFF:0x00);
}

void FastMessage::addFloat(const String key, double value)
{
    copy_float64(append_key_reserve_data(INTEGER, key, 8), value);
}

void FastMessage::addString(const String &key, const String &value)
{
    size_t valueSize = value.size();
    void *valueBuffer = append_key_reserve_data(STRING, key, valueSize+2);
    copy_uint16(valueBuffer, valueSize);
    Memory::memcpy((char *)valueBuffer + 2, value, valueSize);
}

void FastMessage::addIntArray(const String &key, const Buffer<int> &value)
{
    size_t valueSize = value.size();
    void *valueBuffer = append_key_reserve_data(INTEGER_ARRAY, key, valueSize*4 + 2);
    copy_uint16(valueBuffer, valueSize);
    for (int i = 0 ; i < valueSize ; i++) {
        copy_uint32(valueBuffer, value[i]);
        valueBuffer = (char *)valueBuffer + 4;
    }
}

void FastMessage::addCharArray(const String key, const Buffer<char> value)
{
    size_t valueSize = value.size();
    void *valueBuffer = append_key_reserve_data(INTEGER_ARRAY, key, valueSize + 2);
    copy_uint16(valueBuffer, valueSize);
    for (int i = 0 ; i < valueSize ; i++) {
        copy_char8(valueBuffer, value[i]);
        valueBuffer = (char *)valueBuffer + 1;
    }
}

int FastMessage::getInt(const String &key) const
{
    return get_uint32(get_data_for_key(key));
}

bool FastMessage::getBool(const String &key) const
{
    return (get_char8(get_data_for_key(key)) != 0);
}

double FastMessage::getFloat(const String &key) const
{
    return get_float64(get_data_for_key(key));
}

const String FastMessage::getString(const String &key) const
{
    void *stringAddress = get_data_for_key(key);
    size_t stringSize = get_uint16(stringAddress);
    char buf[stringSize+1];
    Memory::memcpy(buf, (char *)stringAddress+2, stringSize);
    buf[stringSize] = 0;
    return buf;
}

const Buffer<int> FastMessage::getIntArray(const String &key) const
{
    void *address = get_data_for_key(key);
    size_t size = get_uint16(address);
    Buffer<int> result(size);
    uint32_t *currentValue = (uint32_t *)((char *)address + 2);
    for (int i = 0 ; i < size ; i++) {
        result[i] = get_uint32(currentValue++);
    }
    return result;
}

const Buffer<char> FastMessage::getCharArray(const String &key) const
{
    void *address = get_data_for_key(key);
    size_t size = get_uint16(address);
    Buffer<char> result((char *)address + 2, size);
    return result;
}

VoidBuffer FastMessage::serialize()
{
    size_t headerOffset = 2 + m_headerRecordsCount * sizeof(HeaderRecord);
    copy_uint16(m_header, headerOffset);
    VoidBuffer result(m_header, headerOffset + m_dataOffset);
    Memory::memcpy((char *)result+headerOffset, m_data, m_dataOffset);
    return result;
}

}


