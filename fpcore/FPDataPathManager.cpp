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

#include <iostream>
#include <sstream>
#include "FPDataPathManager.h"
#include "PackageDescription.h"

using namespace std;

#define isnum(X) ((X>='0') && (X<='9'))


FPDataInputStream::FPDataInputStream(const std::string &fname)
{
    m_f = fopen(fname.c_str(), "r");
}

FPDataInputStream::~FPDataInputStream()
{
    fclose(m_f);
}

int FPDataInputStream::streamRead(void *buffer, int size)
{
    return fread(buffer, 1, size, m_f);
}


FPDataPackage::FPDataPackage(FPDataPathManager *owner,
                             const std::string &packagePath,
                             int packageNumber)
    : m_owner(owner), m_packageNumber(packageNumber)
{
    std::stringstream sstream;
    sstream << packagePath;
    m_name = sstream.str();
}

std::string FPDataPackage::getPath(const std::string &shortPath) const
{
    std::string ret = (m_owner->getPathInPack(shortPath, m_packageNumber));
    if (ret != "")
        return ret;
    else
        throw std::runtime_error((std::string("File ") + shortPath + " not found !").c_str());
}

bool FPDataPackage::hasFile(const std::string &shortPath) const
{
    std::string ret(m_owner->getPathInPack(shortPath, m_packageNumber));
    return (ret != "");
}

std::string FPDataPackage::getName() const
{
    return m_name;
}

DataInputStream *FPDataPackage::openDataInputStream(const std::string &shortPath) const
{
    std::string path = getPath(shortPath);
    return new FPDataInputStream(path.c_str());
}

FPDataPathManager::FPDataPathManager(const std::string &coreDataPath)
    : m_coreDataPath(coreDataPath)
{
    SelfVector<std::string> dataFiles = m_coreDataPath.listFiles();
    SelfVector<std::string> wellFormattedNames;
    AdvancedBuffer<int> wellFormattedNumbers;
    for (int i = 0 ; i < dataFiles.size() ; i++) {
        std::string currentFile = dataFiles[i];
        int len = currentFile.length();
        if ((len > 3) && (currentFile[len - 4] == '.')
                && isnum(currentFile[len - 3])
                && isnum(currentFile[len - 2])
                && isnum(currentFile[len - 1])) {
            wellFormattedNames.add(currentFile);
            wellFormattedNumbers.add(atoi(currentFile.substr(currentFile.length() - 3).c_str()));
        }

    }
    // Now let's sort the names found
    while (wellFormattedNames.size() > 0) {
        int biggestFileIndex = -1;
        int biggestFileNumber = -1;
        for (int i = 0 ; i < wellFormattedNumbers.size() ; i++) {
            if (wellFormattedNumbers[i] > biggestFileNumber) {
                biggestFileNumber = wellFormattedNumbers[i];
                biggestFileIndex = i;
            }
        }
        if (biggestFileIndex != -1) {
            m_dataPaths.add(new FilePath(m_coreDataPath.combine(wellFormattedNames[biggestFileIndex])));
            wellFormattedNumbers.removeAt(biggestFileIndex);
            wellFormattedNames.removeAt(biggestFileIndex);
        }
    }
}

void FPDataPathManager::registerDataPackages(CompositeDrawContext *cDC, Jukebox *jukebox)
{
    // Now iterate through the datapaths to build PackageDescription
    for (int i = 0 ; i < m_dataPaths.size() ; i++) {
        FPDataPackage currentPackage(this, m_dataPaths[i].getPathString(), i);
        PackageDescription packDesc(*this, currentPackage, cDC, jukebox);
    }
}

bool FPDataPathManager::hasFile(const std::string & shortPath) const
{
    for (int i = 0 ; i < m_dataPaths.size() ; i++) {
        FilePath testPath(m_dataPaths[i].combine(shortPath));
        if (testPath.exists())
            return true;
    }
    return false;
}

std::string FPDataPathManager::getPath(const std::string &shortPath) const
{
    for (int i = 0 ; i < m_dataPaths.size() ; i++) {
        FilePath testPath(m_dataPaths[i].combine(shortPath));
        if (testPath.exists())
            return testPath.getPathString();
    }
    return ""; // File not found
}

std::string FPDataPathManager::getPathInPack(const std::string & shortPath, int packPathIndex) const
{
    FilePath testPath(m_dataPaths[packPathIndex].combine(shortPath));
    if (testPath.exists())
        return testPath.getPathString();
    else
        return "";
    // throw std::runtime_error(String("File ") + shortPath + " not found !");
}

SelfVector<std::string> FPDataPathManager::getEntriesAtPath(const std::string &shortPath) const
{
    SelfVector<std::string> result;
    if (hasFile(shortPath)) {
        FilePath rshortPath(shortPath);
        for (int i = 0 ; i < m_dataPaths.size() ; i++) {
            FilePath testPath(m_dataPaths[i].combine(shortPath));
            if (testPath.exists()) {
                SelfVector<std::string> existingFilesInPack = testPath.listFiles();
                for (int j = 0 ; j < existingFilesInPack.size() ; j++)
                {
                    result.add(rshortPath.combine(existingFilesInPack[j]));
                }
            }
        }
    }
    return result;
}

void FPDataPathManager::setMaxPackNumber(int maxPackNumber)
{
    for (int i = m_dataPaths.size() - 1 ; i >= 0 ; i--) {
        const std::string &currentFile = m_dataPaths[i].getPathString();
        int currentNumber = atoi(currentFile.substr(currentFile.length() - 3).c_str());
        if (currentNumber > maxPackNumber)
            m_dataPaths.removeAtKeepOrder(i);
    }
}

DataInputStream *FPDataPathManager::openDataInputStream(const std::string &shortPath) const
{
    std::string path = getPath(shortPath);
    return new FPDataInputStream(path);
}
