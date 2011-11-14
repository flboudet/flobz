//
//  PreferencesManager.h
//  uothello_iphone
//
//  Created by Florent Boudet on 27/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _PREFERENCES_MANAGER_H_
#define _PREFERENCES_MANAGER_H_

#include <string>

class PreferencesManager
{
public:
    virtual ~PreferencesManager() {}
    // Getters
    virtual bool getBoolPreference(const char *identifier, bool defaultVal) const = 0;
    virtual int getIntPreference(const char *identifier, int defaultVal) const = 0;
    virtual std::string getStrPreference(const char *identifier, const char *defaultVal) const = 0;
    // Setters
    virtual void setBoolPreference(const char *identifier, bool value) = 0;
    virtual void setIntPreference(const char *identifier, int value) = 0;
    virtual void setStrPreference(const char *identifier, const char *value) = 0;
};


#endif // _PREFERENCES_MANAGER_H_

