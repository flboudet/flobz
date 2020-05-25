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

/* This class is not thread safe */
#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef WIN32
/* Stupid trick for WIN32 */
#ifdef DATADIR
#undef DATADIR
#endif
#include "windows.h"
#endif
#include "config.h"
#include "GTLog.h"
#include "LocalizedDictionary.h"
#include "ios_memory.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <stdint.h>

static bool readLine(DataInputStream *dictionaryFile, String &lineRead)
{
    bool result = true;
    char newChar[2];
    newChar[1] = 0;
    String newLineRead;
    do {
        if (dictionaryFile->streamRead(newChar, 1) != 1)
            result = false;
        if (result && (newChar[0] != 10) && (newChar[0] != 13))
            newLineRead += newChar;
    } while (result && (newChar[0] != 10) && (newChar[0] != 13));

    // Converting the escape sequences
    if (result) {
        const char *text = newLineRead;
        lineRead = "";
        char previousChar = text[0];
        for (unsigned int i = 0 ; i < strlen(text) ; i++) {
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

/* English is the default */
#define kDefaultPreferedLanguage "en"
/* Additionnaly we'll try at most 10 user languages */
#ifndef kMaxPreferedLanguage
#define kMaxPreferedLanguage 10
#endif

char *PreferedLocales[kMaxPreferedLanguage+1]; // +1 for the default one
int   PreferedLocalesCount = 0;
static bool  systemInitiated = false;

void Locales_Init()
{
  int i;

  if (!systemInitiated) {

#ifdef DEBUG
    fprintf(stdout,"Languages detection...\n");
#endif

#ifdef MACOSX

    CFStringRef localeIdentifier;
    CFArrayRef prefArray;
    char canonicalLocale[32];

    prefArray = (CFArrayRef) CFPreferencesCopyValue(CFSTR("AppleLanguages"), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    PreferedLocalesCount = (prefArray ? CFArrayGetCount(prefArray) : 0);
    if (PreferedLocalesCount > kMaxPreferedLanguage) PreferedLocalesCount = kMaxPreferedLanguage;

    for (i = 0; i < PreferedLocalesCount; i++) {
        localeIdentifier = (CFStringRef)CFArrayGetValueAtIndex(prefArray, i);
        CFStringGetCString(localeIdentifier, canonicalLocale, 32, kCFStringEncodingUTF8);
        if (canonicalLocale[0] < 'a') canonicalLocale[0] += 'a'-'A';
        if (canonicalLocale[1] < 'a') canonicalLocale[1] += 'a'-'A';
        canonicalLocale[2] = 0;
        PreferedLocales[i] = strdup(canonicalLocale);
    }

#else

#ifdef WIN32

#define WinKnownLangsNb 18
    static const char * WinKnownLangsNames[WinKnownLangsNb] = {"fr","en","ja","de","es","it","nl","sv","da","pt","fi","no","ru","ar","el","he","ca","zh"};
    static const WORD WinKnownLangsCodes[WinKnownLangsNb] = {LANG_FRENCH, LANG_ENGLISH, LANG_JAPANESE, LANG_GERMAN, LANG_SPANISH, LANG_ITALIAN, LANG_DUTCH, LANG_SWEDISH, LANG_DANISH, LANG_PORTUGUESE, LANG_FINNISH, LANG_NORWEGIAN, LANG_RUSSIAN, LANG_ARABIC, LANG_GREEK, LANG_HEBREW, LANG_CATALAN, LANG_CHINESE};

    WORD winLang;

    winLang = PRIMARYLANGID(GetUserDefaultLangID());
    for (i=0; i<WinKnownLangsNb; i++)
    {
      if (WinKnownLangsCodes[i] == winLang)
      {
        PreferedLocales[PreferedLocalesCount] = strdup(WinKnownLangsNames[i]);
        PreferedLocalesCount++;
        break;
      }
    }

    winLang = PRIMARYLANGID(GetSystemDefaultLangID());
    for (i=0; i<WinKnownLangsNb; i++)
    {
      if (WinKnownLangsCodes[i] == winLang)
      {
        PreferedLocales[PreferedLocalesCount] = strdup(WinKnownLangsNames[i]);
        PreferedLocalesCount++;
        break;
      }
    }

#endif /* WIN32 */

    char * my_lang = getenv("LANG");
    if ((my_lang != NULL) && (strlen(my_lang) >= 2)) {
      PreferedLocales[PreferedLocalesCount] = strdup(my_lang);
      PreferedLocales[PreferedLocalesCount][2] = 0;
      PreferedLocalesCount++;
    }

#endif /* MACOSX */

    /* Finally set the default language (the reason for the +1 in PreferedLocales declaration) */
    PreferedLocales[PreferedLocalesCount] = strdup(kDefaultPreferedLanguage);
    PreferedLocalesCount++;

#ifdef DEBUG
    for (i = 0; i < PreferedLocalesCount; i++)
      fprintf(stdout,"User prefered language %d : %s\n",i,PreferedLocales[i]);
#endif

    systemInitiated = true;
  }
}


/*************************** CUSTOM HASHMAP STUFF *********************************/
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

// by Paul Hsieh
// http://www.azillionmonkeys.com/qed/hash.html
struct SuperFastHashString {
    size_t operator()(std::string datas) const {
        const char * data = datas.c_str();
        uint32_t len = datas.size();
        uint32_t hash = len, tmp;
        int rem;

        if (len <= 0 || data == NULL) return 0;

        rem = len & 3;
        len >>= 2;

        /* Main loop */
        for (;len > 0; len--) {
            hash  += get16bits (data);
            tmp    = (get16bits (data+2) << 11) ^ hash;
            hash   = (hash << 16) ^ tmp;
            data  += 2*sizeof (uint16_t);
            hash  += hash >> 11;
        }

        /* Handle end cases */
        switch (rem) {
            case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
            case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
            case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash;
    }
};

struct EqualString {
    bool operator()(const std::string s1, const std::string s2) const {
        return (s1 == s2);
    }
};


/*************************************************************************************/
LocalizedDictionary::str_dictionnary_cache LocalizedDictionary::dictionaries;

LocalizedDictionary::LocalizedDictionary(const DataPathManager &datapathManager, const char *dictionaryDirectory, const char *dictionaryName) : dictionary(NULL), datapathManager(datapathManager)
{
  signed int i;

  /* First create the prefered languages list whenever needed */
  Locales_Init();

  std::string stdName(FilePath::combine(dictionaryDirectory, dictionaryName));
  std::shared_ptr<str_dictionnary> myDictEntry;
  auto found = dictionaries.find(stdName);
  if (found == dictionaries.end()) {
      // Dictionary is not cached, create it
      myDictEntry = std::make_shared<str_dictionnary>();
      dictionaries[stdName] = myDictEntry;

      /* Get the first matching dictionary */
      bool found = false;
      for (i = PreferedLocalesCount - 1; i >= 0 ; i--) {

          /* try to open the dictionary for the selected locale */
          String locale(PreferedLocales[i]);
          String directoryName = FilePath::combine(dictionaryDirectory, locale);
          String dictFilePath = FilePath::combine(directoryName, dictionaryName) + ".dic";
          DataInputStream *dictionaryStream = NULL;
          if (datapathManager.hasDataInputStream(dictFilePath))
              dictionaryStream = datapathManager.openDataInputStream(dictFilePath);
          if (dictionaryStream != NULL)
          {
              /* Read all the entries in the dictionary file */
              String keyString, valueString;
              bool fileOk;
              fileOk = readLine(dictionaryStream, keyString);
              while (fileOk) {
                  fileOk = readLine(dictionaryStream, valueString);
                  if (fileOk) {
                      std::string key((const char *)keyString);
                      (*myDictEntry)[key] = valueString;
                      do {
                          fileOk = readLine(dictionaryStream, keyString);
                      } while (fileOk && (keyString == ""));
                  }
              }
              delete dictionaryStream;
//#ifdef DEBUG
//            fprintf(stdout,"Found dictionary %s\n",(const char *)datapathManager.getPath(dictFilePath));
//#endif
            found = true;
        }
    }
    // Should we look for any eligible dictionnary now?
    // By now we don't bother since english (en) should be there or we return the original strings anyway.
//#ifdef DEBUG
//    if (!found) fprintf(stdout,"No dictionary found in %s for %s\n",(const char *)datapathManager.getPath(dictionaryDirectory),dictionaryName);
//#endif
  }

  dictionary = myDictEntry;
  //fprintf(stderr,"-----Refcount++ = %d (%s)\n",myDictEntry->refcount,(const char *)stdName);
}

LocalizedDictionary::~LocalizedDictionary()
{
}

const char * LocalizedDictionary::getLocalizedString(const char * originalString, bool copyIfNotThere)
{
    auto result = dictionary->find(originalString);
    if (result != dictionary->end()) {
        return result->second.c_str();
    }
    else if (copyIfNotThere) {
        (*dictionary)[originalString] = originalString;
        return (*dictionary)[originalString].c_str();
	}
    return originalString;
}
