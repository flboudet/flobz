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
#include "PuyoLocalizedDictionary.h"
#include "ios_memory.h"
#include <stdio.h>

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

/* English is the default */
#define kPuyoDefaultPreferedLanguage "en"
/* Additionnaly we'll try at most 10 user languages */
#define kPuyoMaxPreferedLanguage 10

char *PreferedLocales[kPuyoMaxPreferedLanguage+1]; // +1 for the default one
int   PreferedLocalesCount = 0;
static bool  systemInitiated = false;

typedef struct {
  HashMap * dictionary;
  int refcount;
} dictionaryEntry;

static HashMap dictionaries;

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
    if (PreferedLocalesCount > kPuyoMaxPreferedLanguage) PreferedLocalesCount = kPuyoMaxPreferedLanguage;
    
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
    PreferedLocales[PreferedLocalesCount] = strdup(kPuyoDefaultPreferedLanguage);
    PreferedLocalesCount++;

#ifdef DEBUG
    for (i = 0; i < PreferedLocalesCount; i++)
      fprintf(stdout,"User prefered language %d : %s\n",i,PreferedLocales[i]);
#endif
    
    systemInitiated = true;
  }
}

PuyoLocalizedDictionary::PuyoLocalizedDictionary(const PuyoDataPathManager &datapathManager, const char *dictionaryDirectory, const char *dictionaryName) : dictionary(), datapathManager(datapathManager)
{
  signed int i;

  /* First create the prefered languages list whenever needed */
  Locales_Init();

  stdName = FilePath::combine(dictionaryDirectory, dictionaryName);
  HashValue * result = dictionaries.get((const char *)stdName);
  dictionaryEntry * myDictEntry = NULL;
  
  if (result != NULL) {
    myDictEntry = (dictionaryEntry *)(result->ptr);
  }

  if (myDictEntry == NULL)
  {
    myDictEntry = (dictionaryEntry *)malloc(sizeof(dictionaryEntry));
    myDictEntry->dictionary = new HashMap();
    myDictEntry->refcount=0;
    dictionaries.put((const char *)stdName,(void *)myDictEntry);
    
    /* Get the first matching dictionary */
    bool found = false;
    for (i = PreferedLocalesCount - 1; i >= 0 ; i--) {
    
        /* try to open the dictionary for the selected locale */
        String locale(PreferedLocales[i]);
        String directoryName = FilePath::combine(dictionaryDirectory, locale);
        String dictFilePath = FilePath::combine(directoryName, dictionaryName) + ".dic";
        FILE *dictionaryFile = NULL;

        try { dictionaryFile = fopen(datapathManager.getPath(dictFilePath), "r"); } catch (Exception &e) { }
        
        if (dictionaryFile != NULL)
        {
            /* Read all the entries in the dictionary file */
            String keyString, valueString;
            bool fileOk;
            fileOk = readLine(dictionaryFile, keyString);
            while (fileOk) {
                fileOk = readLine(dictionaryFile, valueString);
                if (fileOk) {
                    HashValue * old = myDictEntry->dictionary->get((const char *)valueString);
                    if (result != NULL) free(old->ptr);
                    char * newstring = strdup(valueString);
                    myDictEntry->dictionary->put(keyString, newstring);
                    do {
                        fileOk = readLine(dictionaryFile, keyString);
                    } while (fileOk && (keyString == ""));
                }
            }
            fclose(dictionaryFile);
            fprintf(stdout,"Found dictionary %s\n",(const char *)datapathManager.getPath(dictFilePath));
            found = true;
        }
    }
    // Should we look for any eligible dictionnary now?
    // By now we don't bother since english (en) should be there or we return the original strings anyway.
#ifdef DEBUG
    if (!found) fprintf(stdout,"No dictionary found in %s for %s\n",(const char *)datapathManager.getPath(dictionaryDirectory),dictionaryName);
#endif
  }
  
  myDictEntry->refcount++;
  dictionary = myDictEntry->dictionary;
  //fprintf(stderr,"-----Refcount++ = %d (%s)\n",myDictEntry->refcount,(const char *)stdName);
}

class HashActionValueFree : public HashMapAction {
	public:
		void action(HashValue * value);
};

void HashActionValueFree::action(HashValue * value) {
	free(value->ptr);
}

PuyoLocalizedDictionary::~PuyoLocalizedDictionary()
{
  HashValue * result = dictionaries.get((const char *)stdName);
  dictionaryEntry * myDictEntry = NULL;
  
  if (result != NULL) {
    myDictEntry = (dictionaryEntry *)(result->ptr);
  }

  if (myDictEntry != NULL)
  {
    myDictEntry->refcount--;
    //fprintf(stderr,"-----Refcount-- = %d (%s)\n",myDictEntry->refcount,(const char *)stdName);
    if (myDictEntry->refcount <= 0) {
      //fprintf(stderr,"-----Destroying %s.\n",(const char *)stdName);
      dictionaries.remove((const char *)stdName);

	  HashActionValueFree myDeleteAction;
	  myDictEntry->dictionary->foreach(&myDeleteAction);

      delete myDictEntry->dictionary;

      free(myDictEntry);
        /* clean up a bit before leaving
        fprintf(stderr,"-----Languages cleanup...\n");
        for (int i = 0; i < PreferedLocalesCount; i++) {
            free(PreferedLocales[i]);
        }
        */
    }
  }
  else
  {
    fprintf(stderr,"FATAL ERROR dictionary %s destroyed too early.\n",(const char *)stdName);
    exit(-1);
  }
}

const char * PuyoLocalizedDictionary::getLocalizedString(const char * originalString, bool copyIfNotThere)
{
    HashValue *result = dictionary->get(originalString);
    if (result != NULL) {
        return  (const char *)(result->ptr);
    } else if (copyIfNotThere)
	{
	  const char * A = strdup(originalString);
      dictionary->put(originalString, (void *)A);
	  return A;
	}
    return originalString;
}

