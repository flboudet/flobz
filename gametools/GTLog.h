#ifndef _GTLOG_T_
#define _GTLOG_T_

// From GC common/util.h
#ifdef __GNUC__
# define HAVE_PRINTF_ATTR 1
# define PRINTF(FMT,X) __attribute__ (( __format__ ( __printf__, FMT, X)))
# define   likely(x)  __builtin_expect((x),1)
# define unlikely(x)  __builtin_expect((x),0)
#else
# define HAVE_PRINTF_ATTR 0
# define PRINTF(FMT,X)
# define   likely(x)  (x)
# define unlikely(x)  (x)
#endif

#define GTLOG_ENABLETRACES 1

#ifdef GTLOG_ENABLETRACES

#define GTLogTrace(__fmt, ...) do { \
    char GTLogTraceArray[512]; \
    snprintf(GTLogTraceArray, 512, "%s:" __fmt, __FUNCTION__, ## __VA_ARGS__); \
    Logger::instance.logln(GTLogTraceArray); \
} while(0)

#else

#define GTLogTrace(__fmt, ...) { }

#endif

#define GTCheckInterval(value, min, max, errorMessage) { \
    if (value < min || value > max) { \
        GTLogTrace("%s: %d", errorMessage, (int)value); \
}}

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
