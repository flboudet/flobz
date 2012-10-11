//
//  PreferencesManager.h
//  uothello_iphone
//
//  Created by Florent Boudet on 27/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NULL_PREFERENCES_MANAGER_H_
#define NULL_PREFERENCES_MANAGER_H_

#include "PreferencesManager.h"

class NullPreferencesManager : public PreferencesManager
{
public:
    // Getters
    virtual bool getBoolPreference(const char *identifier, bool defaultVal) const { return defaultVal; }
    virtual int getIntPreference(const char *identifier, int defaultVal) const    { return defaultVal; }
    virtual std::string getStrPreference(const char *identifier, const char *defaultVal) const { return defaultVal; }
    // Setters
    virtual void setBoolPreference(const char *identifier, bool value) {}
    virtual void setIntPreference(const char *identifier, int value)   {}
    virtual void setStrPreference(const char *identifier, const char *value) {}
};

#endif
