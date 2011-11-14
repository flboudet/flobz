#ifndef _POSIXPREFERENCESMANAGER_H_
#define _POSIXPREFERENCESMANAGER_H_

#include "PreferencesManager.h"

/**
 * Good Ol' flat file storage
 * As initially implemented in the old preferences.cpp by Gyom
 * We should probably have one that uses FileStream one day
 */
class PosixPreferencesManager : public PreferencesManager
{
public:
    PosixPreferencesManager(const char *fileName);
    virtual ~PosixPreferencesManager();
public:
    // Getters
    virtual bool getBoolPreference(const char *identifier, bool defaultVal) const;
    virtual int getIntPreference(const char *identifier, int defaultVal) const;
    virtual std::string getStrPreference(const char *identifier, const char *defaultVal) const;
    // Setters
    virtual void setBoolPreference(const char *identifier, bool value);
    virtual void setIntPreference(const char *identifier, int value);
    virtual void setStrPreference(const char *identifier, const char *value);
private:
    void fetchFile() const;
    void storeFile();
private:
    static const char * m_sep;
    std::string m_fileName;
    mutable char *m_fileContent;
};

#endif // _POSIXPREFERENCESMANAGER_H_

