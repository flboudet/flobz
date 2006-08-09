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

#include <stdio.h>
//#include <CoreFoundation/CoreFoundation.h>
#include "PuyoLocalizedDictionary.h"
#include "ios_memory.h"

static bool readLine(FILE *dictionaryFile, String &lineRead)
{
    bool result = true;
    char newChar[2];
    newChar[1] = 0;
    lineRead = "";
    do {
        if (fread(newChar, 1, 1, dictionaryFile) != 1)
            result = false;
        if (result && (newChar[0] != 10) && (newChar[0] != 13))
            lineRead += newChar;
    } while (result && (newChar[0] != 10) && (newChar[0] != 13));
    return result;
}

PuyoLocalizedDictionary::PuyoLocalizedDictionary(const char *dictionaryDirectory) : dictionary()
{
    // Retrieves the current locale
    //CFMutableArrayRef supportedLocalesArray = CFArrayCreateMutable(NULL, 0, NULL);
    //CFArrayAppendValue(supportedLocalesArray, CFSTR("French"));
    //CFArrayAppendValue(supportedLocalesArray, CFSTR("English"));
    //CFArrayRef chosenLocales = CFBundleCopyPreferredLocalizationsFromArray(supportedLocalesArray);
    //CFStringRef localeIdentifier = (CFStringRef)CFArrayGetValueAtIndex(chosenLocales, 0);
    //CFLocaleRef systemLocale = CFLocaleCopyCurrent();
    //CFStringRef localeIdentifier = (CFStringRef)CFLocaleGetValue(systemLocale, kCFLocaleLanguageCode);
    //CFStringRef localeIdentifier = CFLocaleGetIdentifier(systemLocale);
    //CFStringRef canonicalLocale = CFLocaleCreateCanonicalLanguageIdentifierFromString(NULL, localeIdentifier);
    //char locale[256];
    //CFStringGetCString(localeIdentifier, locale, 256, kCFStringEncodingUTF8);
    //locale[2] = 0;
    String locale("fr");
    String directoryName(dictionaryDirectory);
    
    printf("Locale lue:%s\n", (const char *)locale);
    
    // Read all the entries in the dictionary file
    FILE *dictionaryFile = fopen(directoryName + "/" + locale, "r");
    if (dictionaryFile == NULL)
        return;
    String keyString, valueString;
    bool fileOk;
    fileOk = readLine(dictionaryFile, keyString);
    while (fileOk) {
        fileOk = readLine(dictionaryFile, valueString);
        if (fileOk) {
            char *test = strdup(valueString);
            dictionary.put(keyString, test);
            do {
                fileOk = readLine(dictionaryFile, keyString);
            } while (fileOk && (keyString == ""));
        }
    }
    fclose(dictionaryFile);
}

const char * PuyoLocalizedDictionary::getLocalizedString(const char * originalString) const
{
    HashValue *result = dictionary.get(originalString);
    if (result != NULL) {
        return  (const char *)(result->ptr);
    }
    return originalString;
}

