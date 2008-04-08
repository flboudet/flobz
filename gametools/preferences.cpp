/*
 *  preferences.c
 *  
 *
 *  Created by Guillaume Borios on 03/08/04.
 *  Copyright 2004 iOS. All rights reserved.
 *
 */

#include "preferences.h"


#ifndef __APPLE__

#include <CoreFoundation/CoreFoundation.h>

void SetIntPreference(const char * name, int value)
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

void SetBoolPreference(const char * name, bool value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFPreferencesSetAppValue (nom,value?kCFBooleanTrue:kCFBooleanFalse,kCFPreferencesCurrentApplication);
        (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(nom);
    }
}

int GetIntPreference(const char * name, int defaut)
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

bool GetBoolPreference(const char * name, bool defaut)
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

void SetStrPreference (const char *name, const char *value)
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

void GetStrPreference (const char *name, char *out, const char *defaut, const int bufferSize)
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

#else /* Not __APPLE__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static const char * prefsfile = ".flobopuyorc";
static const char * sep = "\n\r";

static char * file = NULL;
static char * home = NULL;

static bool getPrefsPath(void)
{
    if (home == NULL)
    {
#ifndef _WIN32
        char * h = getenv("HOME");
#else
        char * h = ".";
#endif
        if (h==NULL) return false;
        home = (char *)malloc(strlen(h)+strlen(prefsfile)+2);
        if (home == NULL) return false;
        strcpy(home,h);
        strcat(home,"/");
        strcat(home,prefsfile);
    }
    return true;
}

static void fetchFile(void)
{
    FILE * prefs;
    struct stat myStat;
    
    // If we already have loaded the file, ignore the request
    if (file != NULL) return;
    
    // Set up a fake file if the file path cannot be determined
    if (getPrefsPath() == false)
    {
      file = (char *)malloc((size_t)1);
      file[0]=0;
      return;
    }

    // Else try to read the file
    prefs = fopen(home, "r");

    // Set up a fake file if the real one cannot be read
    if (prefs == NULL)
    {
      file = (char *)malloc((size_t)1);
      file[0]=0;
      return;
    }

    // Check the file size and try to read it or set up a fake file if problem
    if (stat(home,&myStat) == 0)
    {
        file = (char *)malloc((size_t)(myStat.st_size)+1);
        int l = fread(file, 1,(size_t)(myStat.st_size), prefs);
        file[l]=0;
    }
    else
    {
      file = (char *)malloc((size_t)1);
      file[0]=0;
    }

    // Close the file
    fclose(prefs);
}

static void storeFile(void)
{    
    // Ensure we know where to write
    getPrefsPath();

    // Ignore the request if the file path cannot be determined or the memory image doesn't exist
    if ( (file == NULL) || (home == NULL) )
    {
      return;
    }

    // Try to open the file to write it
    FILE * prefs = fopen(home, "w");
    if (prefs == NULL) return;

    // Store the image and close the file
    //fprintf(stderr,"Writing to %s\n%s",home,file);
    fprintf(prefs,"%s",file);
    fclose(prefs);
}

void SetBoolPreference(const char * name, bool value)
{
    SetIntPreference(name, (int)value);
}

void SetIntPreference(const char *name, int value)
{
  char var[256];
  sprintf(var,"%d",value);
  SetStrPreference(name,var);
}


bool GetBoolPreference(const char * name, bool defaut)
{
    return GetIntPreference(name,defaut)?true:false;
}

int GetIntPreference(const char *name, int defaut)
{
  char var[256];
  var[0] = 0;
  GetStrPreference(name,var,NULL,256);
  if (var[0]) return atoi(var);
  return defaut;
}

void SetStrPreference (const char *name, const char *value)
{
    char * key;
    char * prefs;
    
    // if no name given, ignore
    if (name == NULL) return;
    
    // if no value given, use an empty string
    if (value == NULL) value = "";
    
    // Read current value to check if file update is really needed
    int valueLen = strlen(value);
    char * oldValue = (char *)malloc(valueLen+2);
    if (oldValue != NULL)
    {
     // If equal, ignore request
      GetStrPreference(name, oldValue, "core.preferences.fakeoldvalue", valueLen+2);
      if (!strcmp(oldValue,value) && strlen(oldValue) == strlen(value)) return;
    }

    // Not equal... we should update the memory image and the file

    // No memory image available, return...
    if (file == NULL) return;

    // Allocate a new mem image or die
    prefs = (char*)malloc(strlen(file) + strlen(name) + strlen(value) + strlen("=\n") + 1);
    if (prefs == NULL) return;
    
    // Copy the old image to the new, updating the right line...
    prefs[0]=0;
    int l = 0;
    for (key = strtok(file, sep); key; key = strtok(NULL, sep))
    {
        if (strstr(key,name) != key)
        {
          sprintf(prefs+l,"%s\n",key);
          l += strlen(key)+1;
        }
    }
    sprintf(prefs+l,"%s=%s\n",name,value);
    free(file);
    file = prefs;

    // Finally try to store the file
    storeFile();
}


void GetStrPreference (const char *name, char *out, const char *defaut, const int bufferSize)
{
    char * key, *copiedfile;
    int tmplen;

    if ((out==NULL) || (name==NULL)) return;
    
    if (defaut != NULL)
    {
        strncpy(out,defaut,bufferSize-1);
        out[bufferSize-1]=0;
    }
    else out[0]=0;
    
    fetchFile();
    if (file==NULL) return;
    
    char tmp[256];
    sprintf(tmp,"%s=",name);
    tmplen = strlen(tmp);

    copiedfile = strdup(file);
    if (copiedfile == NULL) return;

    for (key = strtok(copiedfile, sep); key; key = strtok(NULL, sep))
    {
        if (strncmp(key, tmp, tmplen) == 0)
        {
            strncpy(out, key+tmplen, bufferSize-1);
            out[bufferSize-1] = 0;
            break;
        }
    }
    free(copiedfile);
    
    return;
}


#endif /* Not __APPLE__ */

