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

void FastMessage::addString(const String &key, const String &value)
{
    size_t valueSize = value.size();
    void *valueBuffer = append_key_reserve_data(INTEGER, key, valueSize+2);
    copy_uint16(valueBuffer, valueSize);
    Memory::memcpy((char *)valueBuffer + 2, value, valueSize);
}

void FastMessage::addIntArray(const String &key, const Buffer<int> &value)
{
    size_t keySize = key.size();
    m_headerRecords[m_headerRecordsCount++] = HeaderRecord(INTEGER, m_dataOffset,
                                            m_dataOffset+keySize);
    Memory::memcpy((char *)m_data+m_dataOffset, (const char *)key, keySize);
    size_t valueSize = value.size();
    m_dataOffset += keySize;
    for (int i = 0 ; i < valueSize ; i++) {
        int32_t curValue = value[i];
        copy_uint32((char *)m_data + m_dataOffset, curValue);
        m_dataOffset += 4;
    }
}

int FastMessage::getInt(const String &key) const
{
    return get_uint16(get_data_for_key(key));
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


