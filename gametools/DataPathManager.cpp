#include "DataPathManager.h"

int BufferedStream::getc() {
    if (m_index < m_buffer.size())
        return m_buffer[m_index++];
    else
        return EOF;
}

int BufferedStream::streamRead(void *buffer, int size) {
    if (m_index + size > m_buffer.size())
        size = m_buffer.size() - m_index;
    ::memcpy(buffer, m_buffer + m_index, size);
    m_index += size;
    return size;
}

#ifdef DISABLED
int BufferedStream::streamSkip(int size) {
    if (m_index + size > m_buffer.size())
        size = m_buffer.size() - m_index;
    m_index += size;
    return size;
}
#endif

char *BufferedStream::gets(char *buf, int bsize)
{
    int i;
    int c;
    int done = 0;
    if (buf == 0 || bsize <= 0)
        return 0;
    for (i = 0; !done && i < bsize - 1; i++) {
        c = getc();
        if (c == EOF) {
            done = 1;
            i--;
        } else {
            buf[i] = c;
            if (c == '\n')
                done = 1;
        }
    }
    buf[i] = '\0';
    if (i == 0)
        return 0;
    else
        return buf;
}

void BufferedStream::load() {
    int loadBlockSize = 1024;
    int numRead   = 0;
    int totalRead = 0;
    while(1) {
        if (m_buffer.size() < totalRead + loadBlockSize) {
            loadBlockSize *= 2;
            m_buffer.realloc(totalRead + loadBlockSize);
        }
        numRead    = m_stream.streamRead(m_buffer + totalRead, loadBlockSize);
        totalRead += numRead;
        if (numRead < loadBlockSize)
            break;
    }
    m_buffer.realloc(totalRead);
}


