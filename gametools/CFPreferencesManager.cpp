#include <CoreFoundation/CoreFoundation.h>
#include "CFPreferencesManager.h"

bool CFPreferencesManager::getBoolPreference(const char *identifier, bool defaultVal) const
{
    bool ret = defaut;
    CFStringRef key = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());

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
    int ret = defaut;
    CFStringRef key = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());

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
    if ((out==NULL) || (name==NULL)) return;

    if (defaut != NULL)
    {
        strncpy(out,defaut,bufferSize-1);
        out[bufferSize-1]=0;
    }
    else out[0]=0;

    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());

    if (nom != NULL)
    {
        CFStringRef value = (CFStringRef)CFPreferencesCopyAppValue(nom,kCFPreferencesCurrentApplication);

        if (value != NULL)
        {
            if (CFGetTypeID(value) == CFStringGetTypeID ())
            {
                if ((!CFStringGetCString (value, out, bufferSize, CFStringGetSystemEncoding())) && (defaut != NULL))
                    strcpy(out,defaut);
            }
            CFRelease(value);
        }
        CFRelease(nom);
    }
}

void CFPreferencesManager::setBoolPreference(const char *identifier, bool value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFPreferencesSetAppValue (nom,value?kCFBooleanTrue:kCFBooleanFalse,kCFPreferencesCurrentApplication);
        (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(nom);
    }
}

void CFPreferencesManager::setIntPreference(const char *identifier, int value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
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
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
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

