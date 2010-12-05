#ifndef _GTLOG_T_
#define _GTLOG_T_

#ifdef GTLOG_ENABLETRACES

#define GTLogTrace(txt) {        \
    Logger::instance.logln(txt); \
    }

#else

#define GTLogTrace(txt) { }

#endif


class LoggerImpl {
public:
    virtual void logln(const char *txt) const = 0;
};

class Logger {
public:
    void logln(const char *txt) const {
        m_implementation->logln(txt);
    }
    void setLoggerImpl(LoggerImpl *implementation);
    // This class is a singleton
    static Logger instance;
private:
    Logger();
    // Disabled copy constructor
    Logger(const Logger &l) {}
    LoggerImpl *m_implementation;
};

#endif // _GTLOG_T_
