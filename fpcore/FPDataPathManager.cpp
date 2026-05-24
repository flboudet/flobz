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
    _f = fopen(fname.c_str(), "r");
}

FPDataInputStream::~FPDataInputStream()
{
    fclose(_f);
}

int FPDataInputStream::streamRead(void *buffer, int size)
{
    return fread(buffer, 1, size, _f);
}


FPDataPackage::FPDataPackage(FPDataPathManager *owner,
                             const std::string &packagePath,
                             int packageNumber)
    : _owner(owner), _packageNumber(packageNumber)
{
    std::stringstream sstream;
    sstream << packagePath;
    _name = sstream.str();
}

std::string FPDataPackage::getPath(const std::string &shortPath) const
{
    std::string ret = (_owner->getPathInPack(shortPath, _packageNumber));
    if (ret != "")
        return ret;
    else
        throw std::runtime_error((std::string("File ") + shortPath + " not found !").c_str());
}

bool FPDataPackage::hasFile(const std::string &shortPath) const
{
    std::string ret(_owner->getPathInPack(shortPath, _packageNumber));
    return (ret != "");
}

std::string FPDataPackage::getName() const
{
    return _name;
}

DataInputStream *FPDataPackage::openDataInputStream(const std::string &shortPath) const
{
    std::string path = getPath(shortPath);
    return new FPDataInputStream(path.c_str());
}

FPDataPathManager::FPDataPathManager(const std::string &coreDataPath)
    : _coreDataPath(coreDataPath)
{
    std::vector<std::string> dataFiles = _coreDataPath.listFiles();
    std::vector<std::string> wellFormattedNames;
    AdvancedBuffer<int> wellFormattedNumbers;
    for (auto const &currentFile : dataFiles) {
        int len = currentFile.length();
        if ((len > 3) && (currentFile[len - 4] == '.')
                && isnum(currentFile[len - 3])
                && isnum(currentFile[len - 2])
                && isnum(currentFile[len - 1])) {
            wellFormattedNames.push_back(currentFile);
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
            _dataPaths.emplace_back(new FilePath(_coreDataPath.combine(wellFormattedNames[biggestFileIndex])));
            wellFormattedNumbers.removeAt(biggestFileIndex);
            wellFormattedNames.erase(wellFormattedNames.begin() + biggestFileIndex);
        }
    }
}

void FPDataPathManager::registerDataPackages(CompositeDrawContext *cDC, Jukebox *jukebox)
{
    // Now iterate through the datapaths to build PackageDescription
    int packageIndex = 0;
    for (auto const &path : _dataPaths) {
        FPDataPackage currentPackage(this, path->getPathString(), packageIndex++);
        PackageDescription packDesc(*this, currentPackage, cDC, jukebox);
    }
}

bool FPDataPathManager::hasFile(const std::string & shortPath) const
{
    for (auto const &path : _dataPaths) {
        FilePath testPath(path->combine(shortPath));
        if (testPath.exists())
            return true;
    }
    return false;
}

std::string FPDataPathManager::getPath(const std::string &shortPath) const
{
    for (auto const &path : _dataPaths) {
        FilePath testPath(path->combine(shortPath));
        if (testPath.exists())
            return testPath.getPathString();
    }
    return ""; // File not found
}

std::string FPDataPathManager::getPathInPack(const std::string & shortPath, int packPathIndex) const
{
    FilePath testPath(_dataPaths[packPathIndex]->combine(shortPath));
    if (testPath.exists())
        return testPath.getPathString();
    else
        return "";
    // throw std::runtime_error(String("File ") + shortPath + " not found !");
}

std::vector<std::string> FPDataPathManager::getEntriesAtPath(const std::string &shortPath) const
{
    std::vector<std::string> result;
    if (hasFile(shortPath)) {
        FilePath rshortPath(shortPath);
        for (auto const &path : _dataPaths) {
            FilePath testPath(path->combine(shortPath));
            if (testPath.exists()) {
                std::vector<std::string> existingFilesInPack = testPath.listFiles();
                for (auto const &fileName : existingFilesInPack) {
                    result.push_back(rshortPath.combine(fileName));
                }
            }
        }
    }
    return result;
}

void FPDataPathManager::setMaxPackNumber(int maxPackNumber)
{
    for (auto it = _dataPaths.begin(); it != _dataPaths.end(); ) {
        const std::string &currentFile = (*it)->getPathString();
        int currentNumber = atoi(currentFile.substr(currentFile.length() - 3).c_str());
        if (currentNumber > maxPackNumber)
            it = _dataPaths.erase(it);
        else
            ++it;
    }
}

DataInputStream *FPDataPathManager::openDataInputStream(const std::string &shortPath) const
{
    std::string path = getPath(shortPath);
    return new FPDataInputStream(path);
}
