/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include "ios_fc.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

namespace ios_fc {

    Exception::Exception(const String exception)
    {
        message = exception;
    }

    Exception::~Exception() throw () {
    }

    const String Exception::getMessage() const {
        return message;
    }

    const char *Exception::what() const throw () {
        return message;
    }

    void Exception::printMessage() const {
        cerr << "Exception thrown: " << (const char*)message << endl;
    }

};

