#include <CoreFoundation/CoreFoundation.h>
#include "CFPreferencesManager.h"

CFPreferencesManager::CFPreferencesManager()
{
}

CFPreferencesManager::~CFPreferencesManager()
{
}

bool CFPreferencesManager::getBoolPreference(const char *identifier, bool defaultVal) const
{
    bool ret = defaultVal;
    CFStringRef key = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());

    if (key != NULL)
    {
        Boolean keyExistsAndHasValidFormat = false;
        Boolean val = CFPreferencesGetAppBooleanValue(key, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
        if (keyExistsAndHasValidFormat == true) ret = (bool)(val);
        CFRelease(key);
    }
    return ret;
}

int CFPreferencesManager::getIntPreference(const char *identifier, int defaultVal) const
{
    int ret = defaultVal;
    CFStringRef key = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());

    if (key != NULL)
    {
        Boolean keyExistsAndHasValidFormat = false;
        CFIndex val = CFPreferencesGetAppIntegerValue(key, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
        if (keyExistsAndHasValidFormat == true) ret = (int)(val);
        CFRelease(key);
    }
    return ret;
}

std::string CFPreferencesManager::getStrPreference(const char *identifier, const char *defaultVal) const
{
    if (identifier==NULL) return "";
    std::string result;
    if (defaultVal != NULL)
    {
        result = defaultVal;
    }
    CFStringRef nom = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFStringRef value = (CFStringRef)CFPreferencesCopyAppValue(nom,kCFPreferencesCurrentApplication);
        if (value != NULL)
        {
            if (CFGetTypeID(value) == CFStringGetTypeID ())
            {
                char out[255];
                if (CFStringGetCString (value, out, 255, CFStringGetSystemEncoding()))
                    result = out;
            }
            CFRelease(value);
        }
        CFRelease(nom);
    }
    return result;
}

void CFPreferencesManager::setBoolPreference(const char *identifier, bool value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFPreferencesSetAppValue (nom,value?kCFBooleanTrue:kCFBooleanFalse,kCFPreferencesCurrentApplication);
        (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(nom);
    }
}

void CFPreferencesManager::setIntPreference(const char *identifier, int value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFNumberRef aValue = CFNumberCreate(NULL,kCFNumberIntType,&value);
        if (aValue != NULL)
        {
            CFPreferencesSetAppValue (nom,aValue,kCFPreferencesCurrentApplication);
            (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
            CFRelease(aValue);
        }
        CFRelease(nom);
    }
}

void CFPreferencesManager::setStrPreference(const char *identifier, const char *value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,identifier,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFStringRef val = CFStringCreateWithCString (NULL,value,CFStringGetSystemEncoding());
        if (val != NULL)
        {
            CFPreferencesSetAppValue (nom,val,kCFPreferencesCurrentApplication);
            (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
            CFRelease(val);
        }
        CFRelease(nom);
    }
}

