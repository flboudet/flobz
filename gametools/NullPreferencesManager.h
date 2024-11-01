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
    virtual bool getBoolPreference(const std::string &identifier, bool defaultVal) const { return defaultVal; }
    virtual int getIntPreference(const std::string &identifier, int defaultVal) const    { return defaultVal; }
    virtual std::string getStrPreference(const std::string &identifier, const std::string &defaultVal) const { return defaultVal; }
    // Setters
    virtual void setBoolPreference(const std::string &identifier, bool value) {}
    virtual void setIntPreference(const std::string &identifier, int value)   {}
    virtual void setStrPreference(const std::string &identifier, const std::string &value) {}
};

#endif
