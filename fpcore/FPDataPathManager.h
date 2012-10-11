/* FloboPop
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

#ifndef _PUYODATAPATHMANAGER_H
#define _PUYODATAPATHMANAGER_H

#include "CompositeDrawContext.h"
#include "audio.h"
#include "DataPathManager.h"
#include "ios_filepath.h"

using namespace ios_fc;

class FPDataPathManager;

class FPDataPackage : public DataPackage {
public:
    FPDataPackage(FPDataPathManager *owner,
                  const char *packagePath,
                  int packageNumber);
    virtual bool hasFile(const char *shortPath) const;
    virtual std::string getPath(const char *shortPath) const;
    virtual std::string getName() const;
    virtual DataInputStream *openDataInputStream(const char *shortPath) const;
private:
    FPDataPathManager *m_owner;
    int m_packageNumber;
    std::string m_name;
};

class FPDataInputStream : public DataInputStream {
public:
    FPDataInputStream(const char *fname);
    virtual ~FPDataInputStream();
    virtual int streamRead(void *buffer, int size);
private:
    FILE *m_f;
};

class AbstractFPDataPathManager : public DataPathManager
{
public:
    virtual void registerDataPackages(CompositeDrawContext *cDC, Jukebox *jukebox = NULL) = 0;
};

class FPDataPathManager : public AbstractFPDataPathManager {
public:
    FPDataPathManager(String coreDataPath);
    virtual bool hasFile(String shortPath) const;
    virtual String getPath(String shortPath) const;
    virtual SelfVector<String> getEntriesAtPath(String shortPath) const;
    virtual DataInputStream *openDataInputStream(const char *shortPath) const;
    virtual void registerDataPackages(CompositeDrawContext *cDC, Jukebox *jukebox = NULL);
    // Own methods
    String getPathInPack(String shortPath, int packPathIndex) const;
    int getNumPacks() const { return m_dataPaths.size(); }
    void setMaxPackNumber(int maxPackNumber);
private:
    FPDataPathManager(FPDataPathManager &mgr) : m_coreDataPath(mgr.m_coreDataPath) {}
    FilePath m_coreDataPath;
    SelfVector<FilePath> m_dataPaths;
};

#endif // _PUYODATAPATHMANAGER_H

