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
    virtual bool getBoolPreference(const std::string &identifier, bool defaultVal) const = 0;
    virtual int getIntPreference(const std::string &identifier, int defaultVal) const = 0;
    virtual std::string getStrPreference(const std::string &identifier, const std::string &defaultVal) const = 0;
    // Setters
    virtual void setBoolPreference(const std::string &identifier, bool value) = 0;
    virtual void setIntPreference(const std::string &identifier, int value) = 0;
    virtual void setStrPreference(const std::string &identifier, const std::string &value) = 0;
};


#endif // _PREFERENCES_MANAGER_H_

