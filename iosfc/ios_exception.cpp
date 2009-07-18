/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include "ios_exception.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include <signal.h>

using namespace std;

namespace ios_fc {

    std::string get_stack_trace()
    {
        const size_t max_depth = 100;
        size_t stack_depth;
        void *stack_addrs[max_depth];
        char **stack_strings;
        std::stringstream out;
#ifndef WIN32
        stack_depth = backtrace(stack_addrs, max_depth);
        stack_strings = backtrace_symbols(stack_addrs, stack_depth);
#endif
        out << "Call stack:\n";

        for (size_t i = 1; i < stack_depth; i++) {
            size_t sz = 200; // just a guess, template names will go much wider
            char *function = static_cast<char*>(malloc(sz));
            char *begin = 0, *end = 0;
            // find the parentheses and address offset surrounding the mangled name
            for (char *j = stack_strings[i]; *j; ++j) {
                if (*j == '(') {
                    begin = j;
                }
                else if (*j == '+') {
                    end = j;
                }
            }
            if (begin && end) {
                *begin++ = '\0';
                *end = '\0';
                // found our mangled name, now in [begin, end)

                int status;
                char *ret = abi::__cxa_demangle(begin, function, &sz, &status);
                if (ret) {
                    // return value may be a realloc() of the input
                    function = ret;
                }
                else {
                    // demangling failed, just pretend it's a C function with no args
                    std::strncpy(function, begin, sz);
                    std::strncat(function, "()", sz);
                    function[sz-1] = '\0';
                }
                out << "    " << stack_strings[i] << ":" << function << "\n";
            }
            else
            {
                // didn't find the mangled name, just print the whole line
                out << "    " << stack_strings[i] << "\n";
            }
            free(function);
        }
        free(stack_strings); // malloc()ed by backtrace_symbols
        return out.str();
    }


    const char *DBG_PRINT_PREFIX = "...";

    Exception::Exception(const char *exception)
      : message(strdup(exception))
    {
        stack = get_stack_trace();
    }

    Exception::~Exception() throw ()
    {
        free(message);
    }

    Exception::Exception(const Exception &e)
      : message(strdup(e.message))
    {
        std::stringstream newstack;
        newstack << get_stack_trace();
        newstack << "  Which was caused by:\n";
        newstack << e.stack;
        stack = newstack.str();
    }

    const char *Exception::what() const throw () {
        return message;
    }

    const char *Exception::getStackTrace() const
    {
        return stack.c_str();
    }

    void Exception::printMessage() const {
        fprintf(stderr, "Exception thrown: %s\n", message);
        fprintf(stderr, "From:\n%s\n", stack.c_str());
    }

#ifndef WIN32
    void signal_handler(int sig) {
        static volatile unsigned long _new = 0;
        if (_new)
            return;
        ++_new;
        switch (sig)
        {
            case SIGSEGV:
            case SIGFPE:
            case SIGBUS:
            case SIGILL:
                fprintf(stderr, "Caught signal %d from %s:%d\n", sig, __FILE__, __LINE__);
                {
                std::string st = get_stack_trace();
                fprintf(stderr, "%s\n", st.c_str());
                }
                abort();
                break;
            case SIGTERM:
                exit(255);
                break;
            case SIGQUIT:
            case SIGINT:
                exit(255);
                break;
        }
    }
#endif

    void catch_signals() {
#ifndef WIN32
        /* Invalid memory reference */
        signal (SIGSEGV, signal_handler);
        /* Interrupt from keyboard */
        signal (SIGINT, signal_handler);
        /* Quit from keyboard */
        signal (SIGQUIT, signal_handler);
        /* Floating point exception */
        signal (SIGFPE, signal_handler);
        /* Bus error (bad memory access) */
        signal (SIGBUS, signal_handler);
        /* Termination signal */
        signal (SIGTERM, signal_handler);
        /* Illegal Instruction */
        signal (SIGILL, signal_handler);
#endif
    }

    void reportCrash(const std::string &error) {
        fprintf(stderr, "WOOPS! :-(\n%s\n", error.c_str());
        fprintf(stderr, "\nPlease report the bug.\n");
        fprintf(stderr, "  - To the dedicated forum thread: http://www.fovea.cc/forum/viewtopic.php?id=5&t_id=7\n");
        fprintf(stderr, "  - By email: woops@fovea.cc\n");
        fprintf(stderr, "\nWe will fix this as quickly as possible. Thanks for your help!\n");
    }
}

