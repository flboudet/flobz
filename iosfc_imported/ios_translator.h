#ifndef _IOS_GETTEXT
#define _IOS_GETTEXT

#include "ios_memory.h"

namespace ios_fc
{
  class TranslationDictionary {
    public:
      TranslationDictionary(InputStream *resourceFile);
      String translate(const String text) const;
  };

  class TranslateFunction {
    public:
      // Translate the string
      String operator()(const String from) const;
      
      // Adds a dictionary with a lower priority than the existing ones
      void addDictionary(TranslationDictionary *dict) const;

      // Clear the dictionary list and delete them.
      void deleteDictionaries();

    private:
      Vector<TranslationDictionary*> dictonaries;
  };
};

#endif
