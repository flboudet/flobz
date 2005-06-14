/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include "ios_exception.h"

#include <cstdlib>
#include <cstring>
using namespace std;

namespace ios_fc {
    
    const char *DBG_PRINT_PREFIX = "...";

    Exception::Exception(const char *exception)
      : message(strdup(exception))
      {}

    Exception::~Exception() throw ()
    {
        free(message);
    }

    Exception::Exception(const Exception &e)
    : message(strdup(e.message))
    {}

    const char *Exception::what() const throw () {
        return message;
    }

    void Exception::printMessage() const {
        fprintf(stderr, "Exception thrown: %s\n", message);
    }

}

