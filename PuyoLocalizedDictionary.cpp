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
#include <CoreFoundation/CoreFoundation.h>
#include "PuyoLocalizedDictionary.h"
#include "ios_memory.h"

static bool readLine(FILE *dictionaryFile, String &lineRead)
{
    bool result = true;
    char newChar[2];
    newChar[1] = 0;
    String newLineRead;
    do {
        if (fread(newChar, 1, 1, dictionaryFile) != 1)
            result = false;
        if (result && (newChar[0] != 10) && (newChar[0] != 13))
            newLineRead += newChar;
    } while (result && (newChar[0] != 10) && (newChar[0] != 13));
    
    // Converting the escape sequences
    if (result) {
        const char *text = newLineRead;
        lineRead = "";
        char previousChar = text[0];
        for (int i = 0 ; i < strlen(text) ; i++) {
            char texti = text[i];
            switch (previousChar) {
                case '\\':
                    switch (texti) {
                        case 'n':
                            lineRead += '\n';
                            break;
                        default:
                            lineRead += texti;
                    }
                    break;
                default:
                    switch (texti) {
                        case '\\':
                            break;
                        default:
                            lineRead += texti;
                    }
            }
            previousChar = texti;
        }
    }
    return result;
}

PuyoLocalizedDictionary::PuyoLocalizedDictionary(const PuyoDataPathManager &datapathManager, const char *dictionaryDirectory, const char *dictionaryName) : dictionary(), datapathManager(datapathManager)
{
/* English is the default */
#define kPuyoDefaultPreferedLanguage "en"
/* We'll try at most 10 languages */
#define kPuyoMaxPreferedLanguage 10

    int i;
    char * PreferedLocales[kPuyoMaxPreferedLanguage];
    int PreferedLocalesCount = 0;

    /* First create the prefered languages list */
#ifdef MACOSX

    CFStringRef localeIdentifier;
    CFArrayRef prefArray;
    char canonicalLocale[32];

    prefArray = (CFArrayRef) CFPreferencesCopyValue(CFSTR("AppleLanguages"), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    PreferedLocalesCount = (prefArray ? CFArrayGetCount(prefArray) : 0);
    if (PreferedLocalesCount > kPuyoMaxPreferedLanguage-1) PreferedLocalesCount = kPuyoMaxPreferedLanguage-1;
    
    for (i = 0; i < PreferedLocalesCount; i++) {
        localeIdentifier = (CFStringRef)CFArrayGetValueAtIndex(prefArray, i);
        CFStringGetCString(localeIdentifier, canonicalLocale, 32, kCFStringEncodingUTF8);
        if (canonicalLocale[0] < 'a') canonicalLocale[0] += 'a'-'A';
        if (canonicalLocale[1] < 'a') canonicalLocale[1] += 'a'-'A';
        canonicalLocale[2] = 0;
        PreferedLocales[i] = strdup(canonicalLocale);
    }
    PreferedLocales[PreferedLocalesCount] = strdup(kPuyoDefaultPreferedLanguage);
    PreferedLocalesCount++;
    
#else
    
    char * my_lang = getenv("LANG");
    if ((my_lang != NULL) && (strlen(my_lang) >= 2)) {
      PreferedLocales[PreferedLocalesCount] = strdup(my_lang);
      PreferedLocales[PreferedLocalesCount][2] = 0;
      PreferedLocalesCount++;
    }
    PreferedLocales[PreferedLocalesCount] = strdup(kPuyoDefaultPreferedLanguage);
    PreferedLocalesCount++;

#endif /* MACOSX */

    /* Get the first matching dictionary */
    for (i = 0; i < PreferedLocalesCount; i++) {
        String locale(PreferedLocales[i]);
        String directoryName = FilePath::combine(dictionaryDirectory, locale);
        String dictFilePath = FilePath::combine(directoryName, dictionaryName) + ".dic";
    
        // Read all the entries in the dictionary file
        FILE *dictionaryFile = NULL;
        try { dictionaryFile = fopen(datapathManager.getPath(dictFilePath), "r"); } catch (Exception &e) { }
        if (dictionaryFile != NULL)
        {
            //printf("Locale lue:%s\n", (const char *)locale);
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
            break;
        }
    }
    
    /* clean up a bit before leaving */
    for (i = 0; i < PreferedLocalesCount; i++) {
        //fprintf(stderr,"Found prefered lang : %s\n",PreferedLocales[i]);
        free(PreferedLocales[i]);
    }

}

const char * PuyoLocalizedDictionary::getLocalizedString(const char * originalString) const
{
    HashValue *result = dictionary.get(originalString);
    if (result != NULL) {
        return  (const char *)(result->ptr);
    }
    return originalString;
}

