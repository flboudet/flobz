#ifndef _GTLOG_T_
#define _GTLOG_T_

#define GTLOG_ENABLETRACES 1

#ifdef GTLOG_ENABLETRACES

#define GTLogTrace(txt) { \
    char *GTLogTraceArray; \
    GTLogTraceArray = (char*)malloc(strlen(txt) + strlen(__FUNCTION__) + 3); \
    sprintf(GTLogTraceArray, "%s: %s", __FUNCTION__, txt); \
    Logger::instance.logln(txt); \
    free(GTLogTraceArray); \
}

#else

#define GTLogTrace(txt) { }

#endif

#define GTCheckInterval(value, min, max, errorMessage) { \
    if (value < min || value > max) \
        GTLogTrace(errorMessage); \
}

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
