/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#ifndef _DATAPATHMANAGER_H
#define _DATAPATHMANAGER_H

#include "ios_filepath.h"

using namespace ios_fc;

class DataInputStream {
public:
    virtual ~DataInputStream() {}
    virtual int streamRead(void *buffer, int size) = 0;
};

class DataProvider {
public:
    virtual DataInputStream *openDataInputStream(const char *shortPath) const = 0;
};

class DataPackage : public DataProvider {
public:
    virtual std::string getPath(const char *shortPath) const = 0;
    virtual std::string getName() const = 0;
};

class DataPathManager : public DataProvider {
public:
    virtual String getPath(String shortPath) const = 0;
    virtual SelfVector<String> getEntriesAtPath(String shortPath) const = 0;
};

#endif // _DATAPATHMANAGER_H
