#ifndef _CFPREFERENCESMANAGER_H_
#define _CFPREFERENCESMANAGER_H_

#include "PreferencesManager.h"

/**
 * PreferencesManager for the macs and ios devices
 */
class CFPreferencesManager : public PreferencesManager
{
public:
    CFPreferencesManager();
    virtual ~CFPreferencesManager();
public:
    // Getters
    virtual bool getBoolPreference(const char *identifier, bool defaultVal) const;
    virtual int getIntPreference(const char *identifier, int defaultVal) const;
    virtual std::string getStrPreference(const char *identifier, const char *defaultVal) const;
    // Setters
    virtual void setBoolPreference(const char *identifier, bool value);
    virtual void setIntPreference(const char *identifier, int value);
    virtual void setStrPreference(const char *identifier, const char *value);
};

#endif // _CFPREFERENCESMANAGER_H_

