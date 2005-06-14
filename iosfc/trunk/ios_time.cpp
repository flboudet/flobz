#ifndef WIN32

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

namespace ios_fc
{

#if _POSIX_TIMERS > 0

static int checkIfLower(struct timespec *min, clockid_t type, const char *stype)
{
    struct timespec ts;
    if (::clock_getres(type, &ts) < 0) {
        printf("%s not supported\n", stype);
        return 0;
    }

    printf("    %s Resolution: %d sec %d nano\n", stype, (int)ts.tv_sec, (int)ts.tv_nsec);

    if (min->tv_sec > ts.tv_sec) {
        *min = ts;
        return 1;
    }

    if (min->tv_sec == ts.tv_sec)
        if (min->tv_nsec > ts.tv_nsec) {
            *min = ts;
            return 1;
        }

    return 0;
}

clockid_t selectClock(void)
{
    struct timespec min; min.tv_sec = 999;
    clockid_t ret = 0;
    const char *selected = NULL;

    printf("Checking clocks\n");

#define DO_CHK(type) \
    if (checkIfLower(&min, type, #type)) { \
        ret = type; \
        selected = #type; \
    }

    DO_CHK(CLOCK_REALTIME);

#if 0
    Commented out since it s not reliable clock... set a way to use them for benchmarks.

#ifdef _POSIX_MONOTONIC_CLOCK
    DO_CHK(CLOCK_MONOTONIC);
#endif


    clockid_t tmp;
#ifdef _POSIX_CPUTIME
    if (clock_getcpuclockid(0,&tmp) != ENOENT)
        DO_CHK(CLOCK_PROCESS_CPUTIME_ID);
#endif
    
#ifdef _POSIX_THREAD_CPUTIME
    if (clock_getcpuclockid(0,&tmp) != ENOENT)
        DO_CHK(CLOCK_THREAD_CPUTIME_ID);
#endif

#endif

#undef  DO_CHK
        
    printf("Selected: %s\n", selected);

    return ret;
}

double getTimeMs()
{
    static struct timespec t0;
    static clockid_t clk;

    static int firstTime = 1;
    if (firstTime)
    {
        clk = selectClock();
        clock_gettime(clk, &t0);
        firstTime = 0;
    }
        
    struct timespec ts;
    clock_gettime(clk, &ts);
    
    return 1000. * (ts.tv_sec - t0.tv_sec) + 0.000001 * ts.tv_nsec;
}

#else // !_POSIX_TIMERS

double getTimeMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_usec / 1000.0) + ((double)tv.tv_sec * 1000.0);
}

#endif // _POSIX_TIMERS

}

#else // WIN32

#error TODO

#endif // WIN32
