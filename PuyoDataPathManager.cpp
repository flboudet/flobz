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

#include "PuyoDataPathManager.h"

PuyoDataPathManager::PuyoDataPathManager(String coreDataPath) : m_coreDataPath(coreDataPath)
{
    SelfVector<String> dataFiles = m_coreDataPath.listFiles();
    SelfVector<String> wellFormattedNames;
    AdvancedBuffer<int> wellFormattedNumbers;
    for (int i = 0 ; i < dataFiles.size() ; i++) {
        String currentFile = dataFiles[i];
        if (currentFile[currentFile.length() - 4] == '.') {
            wellFormattedNames.add(currentFile);
            wellFormattedNumbers.add(atoi(currentFile.substring(currentFile.length() - 3)));
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

String PuyoDataPathManager::getPath(String shortPath) const
{
    for (int i = 0 ; i < m_dataPaths.size() ; i++) {
        FilePath testPath(m_dataPaths[i].combine(shortPath));
        if (testPath.exists())
            return testPath.getPathString();
    }
    throw Exception(String("File ") + shortPath + " not found !");
}

int main()
{
    PuyoDataPathManager test("data/");
    printf("Trouve:%s\n", (const char *)test.getPath("gfx/radada.jpeg"));
}
