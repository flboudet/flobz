/*
 *  preferences.c
 *  
 *
 *  Created by Guillaume Borios on 03/08/04.
 *  Copyright 2004 iOS. All rights reserved.
 *
 */

#include "preferences.h"


#ifdef __APPLE__

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

void SetBoolPreference(const char * name, int value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFPreferencesSetAppValue (nom,value?CFSTR("YES"):CFSTR("NO"),kCFPreferencesCurrentApplication);
        (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(nom);
    }
}

int GetIntPreference(const char * name, int defaut)
{
    int retour = defaut;
    
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    
    if (nom != NULL)
    {
        CFNumberRef value = (CFNumberRef)CFPreferencesCopyAppValue(nom,kCFPreferencesCurrentApplication);
        
        if (value != NULL)
        {
           if (CFGetTypeID(value) == CFNumberGetTypeID ())
           {
            if (CFNumberGetValue (value, kCFNumberIntType,&retour) == false)
                retour = defaut;
           }
            CFRelease(value);
        }        
        CFRelease(nom);
    }
    
    return retour;
}

int GetBoolPreference(const char * name, int defaut)
{
    int retour = defaut;
    
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    
    if (nom != NULL)
    {
        CFStringRef value = (CFStringRef)CFPreferencesCopyAppValue(nom,kCFPreferencesCurrentApplication);
    
        if (value != NULL)
        {
            if (CFGetTypeID(value) == CFStringGetTypeID ())
            {
                retour = (CFStringCompare(value, CFSTR("YES"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) ? 1 : 0;
            }
            CFRelease(value);
        }
        CFRelease(nom);
    }
    
    
    return retour;
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
    
    if (defaut != NULL) strncpy(out,defaut,bufferSize);
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

static void fetchFile(void)
{
    FILE * prefs;
    struct stat myStat;
    
    if (home == NULL)
    {
#ifndef _WIN32
        char * h = getenv("HOME");
#else
        char * h = ".";
#endif
        if (h==NULL) return;
        home = (char *)malloc(strlen(h)+strlen(prefsfile)+2);
        strcpy(home,h);
        strcat(home,"/");
        strcat(home,prefsfile);
    }
    
    if (file != NULL)
    {
        free(file);
        file = NULL;
    }
    
    prefs = fopen(home, "r");
    if (prefs == NULL) return;

    if (stat(home,&myStat) == 0)
    {
        file = (char *)malloc((size_t)(myStat.st_size)+1);
        int l = fread(file, 1,(size_t)(myStat.st_size), prefs);
        file[l]=0;
    }
    
    fclose(prefs);
}


void SetBoolPreference(const char *name, int value)
{
    SetIntPreference(name, value);
}

void SetIntPreference(const char *name, int value)
{
  char var[256];
  sprintf(var,"%d",value);
  SetStrPreference(name,var);
}


int GetBoolPreference(const char *name, int defaut)
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
    FILE * prefs;
    
    fetchFile();

    if (home == NULL) return;
    
    prefs = fopen(home, "w+");
    if (prefs == NULL) return;
    
    if (file != NULL)
    {
        for (key = strtok(file, sep); key; key = strtok(NULL, sep))
        {
            if (strstr(key,name)==NULL)
            {
                fprintf(prefs,"%s\n",key);
            }
        }
        free(file);
        file = NULL;
    }
    
    fprintf(prefs,"%s=%s\n",name,value);
    
    fclose(prefs);
}


void GetStrPreference (const char *name, char *out, const char *defaut, const int bufferSize)
{
    char * key, *copiedfile;
    int tmplen;

    if ((out==NULL) || (name==NULL)) return;
    
    if (defaut != NULL) strncpy(out,defaut,bufferSize);
    else out[0]=0;
    
    if (file==NULL) fetchFile();
    if (file==NULL) return;
    
    char tmp[256];
    sprintf(tmp,"%s=",name);
    tmplen = strlen(tmp);

    copiedfile = strdup(file);
    
    for (key = strtok(copiedfile, sep); key; key = strtok(NULL, sep))
    {
        if (strncmp(key, tmp, tmplen) == 0)
        {
            strncpy(out, key+tmplen, bufferSize);
            free(copiedfile);
            return;
        }
    }
    free(copiedfile);
    
    return;
}


#endif /* Not __APPLE__ */

