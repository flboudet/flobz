#include <iostream>
#include "GTLog.h"

Logger Logger::instance;

class DefaultLoggerImpl : public LoggerImpl {
public:
    virtual void logln(const char *txt) const {
        std::cout << txt << std::endl;
    }
};

static DefaultLoggerImpl s_defaultImpl;

Logger::Logger()
{
    setLoggerImpl(&s_defaultImpl);
}

void Logger::setLoggerImpl(LoggerImpl *implementation)
{
    m_implementation = implementation;
}

