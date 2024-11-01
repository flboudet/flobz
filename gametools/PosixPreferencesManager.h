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
    PosixPreferencesManager(const std::string &fileName);
    virtual ~PosixPreferencesManager();
public:
    // Getters
    virtual bool getBoolPreference(const std::string &identifier, bool defaultVal) const;
    virtual int getIntPreference(const std::string &identifier, int defaultVal) const;
    virtual std::string getStrPreference(const std::string &identifier, const std::string &defaultVal) const;
    // Setters
    virtual void setBoolPreference(const std::string &identifier, bool value);
    virtual void setIntPreference(const std::string &identifier, int value);
    virtual void setStrPreference(const std::string &identifier, const std::string &value);
private:
    void fetchFile() const;
    void storeFile();
private:
    static const std::string m_sep;
    std::string m_fileName;
    mutable char *m_fileContent;
};

#endif // _POSIXPREFERENCESMANAGER_H_

