#include "PosixPreferencesManager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

const char * PosixPreferencesManager::m_sep = "\n\r";

PosixPreferencesManager::PosixPreferencesManager(const char *fileName)
    : m_fileName(fileName), m_fileContent(NULL)
{
}

PosixPreferencesManager::~PosixPreferencesManager()
{
    if (m_fileContent != NULL)
        free(m_fileContent);
}

bool PosixPreferencesManager::getBoolPreference(const char *identifier, bool defaultVal) const
{
    return getIntPreference(identifier, defaultVal ? 1 : 0) ? true : false;
}

int PosixPreferencesManager::getIntPreference(const char *identifier, int defaultVal) const
{
    std::string sresult = getStrPreference(identifier, "");
    if (sresult == "")
        return defaultVal;
    return atoi(sresult.c_str());
}

void PosixPreferencesManager::setBoolPreference(const char *identifier, bool value)
{
    setIntPreference(identifier, (int)value);
}

void PosixPreferencesManager::setIntPreference(const char *identifier, int value)
{
    char var[256];
    sprintf(var, "%d", value);
    setStrPreference(identifier, var);
}

std::string PosixPreferencesManager::getStrPreference(const char *identifier, const char *defaultVal) const
{
    char * key, *copiedfile;
    int tmplen;
    std::string result;
    if (identifier == NULL)
        return result;
    if (defaultVal != NULL)
        result = defaultVal;
    fetchFile();
    if (m_fileContent == NULL)
        return result;
    char tmp[256];
    sprintf(tmp,"%s=", identifier);
    tmplen = strlen(tmp);
    copiedfile = strdup(m_fileContent);
    if (copiedfile == NULL) return result;

    for (key = strtok(copiedfile, m_sep); key; key = strtok(NULL, m_sep))
    {
        if (strncmp(key, tmp, tmplen) == 0)
        {
            char outBuffer[255];
            strncpy(outBuffer, key+tmplen, 254);
            outBuffer[254] = 0;
            result = outBuffer;
            break;
        }
    }
    free(copiedfile);
    return result;
}

void PosixPreferencesManager::setStrPreference(const char *identifier, const char *value)
{
    char * key;
    char * prefs;
    // if no name given, ignore
    if (identifier == NULL)
        return;
    // if no value given, use an empty string
    if (value == NULL) value = "";
    // Read current value to check if file update is really needed
    // If equal, ignore request
    std::string oldValue = getStrPreference(identifier, "core.preferences.fakeoldvalue");
    if (!strcmp(oldValue.c_str(),value) && strlen(oldValue.c_str()) == strlen(value)) {
        return;
    }
    // Not equal... we should update the memory image and the file
    // No memory image available, return...
    if (m_fileContent == NULL) return;
    // Allocate a new mem image or die
    prefs = (char *)calloc(1, strlen(m_fileContent) + strlen(identifier) + strlen(value) + strlen("=\n") + 1);
    if (prefs == NULL)
        return;
    // Copy the old image to the new, updating the right line...
    int l = 0;
    for (key = strtok(m_fileContent, m_sep); key; key = strtok(NULL, m_sep))
    {
        if (strstr(key, identifier) != key)
        {
          sprintf(prefs+l,"%s\n",key);
          l += strlen(key)+1;
        }
    }
    sprintf(prefs+l,"%s=%s\n", identifier,value);
    free(m_fileContent);
    m_fileContent = prefs;
    // Finally try to store the file
    storeFile();
}

void PosixPreferencesManager::fetchFile() const
{
    FILE * prefs;
    struct stat myStat;
    // If we already have loaded the file, ignore the request
    if (m_fileContent != NULL)
        return;
    // Try to read the file
    prefs = fopen(m_fileName.c_str(), "r");
    // If we fail, return a crafted file content
    if (prefs == NULL) {
        m_fileContent = (char *)calloc(1, 1);
        return;
    }
    // Check the file size and try to read it
    if (stat(m_fileName.c_str(), &myStat) == 0)
    {
        m_fileContent = (char *)malloc((size_t)(myStat.st_size)+1);
        int l = fread(m_fileContent, 1,(size_t)(myStat.st_size), prefs);
        m_fileContent[l]=0;
    }
    // If we fail, return a crafted file content
    else
    {
        m_fileContent = (char *)calloc(1, 1);
        return;
    }
    // Close the file
    fclose(prefs);
}

void PosixPreferencesManager::storeFile()
{
    if (m_fileContent == NULL)
        return;
    // Try to open the file to write it
    FILE * prefs = fopen(m_fileName.c_str(), "w");
    if (prefs == NULL) return;
    // Store the image and close the file
    fprintf(prefs,"%s", m_fileContent);
    fclose(prefs);
}
