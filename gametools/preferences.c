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

void SetBoolPreference(char * name, int value)
{
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    if (nom != NULL)
    {
        CFPreferencesSetAppValue (nom,value?CFSTR("YES"):CFSTR("NO"),kCFPreferencesCurrentApplication);
        (void)CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        CFRelease(nom);
    }
}

int GetBoolPreference(char * name, int defaut)
{
    int retour;
    
    CFStringRef nom = CFStringCreateWithCString (NULL,name,CFStringGetSystemEncoding());
    
    if (nom != NULL)
    {
        CFStringRef value = (CFStringRef)CFPreferencesCopyAppValue(nom,kCFPreferencesCurrentApplication);
    
        if ((value != NULL) && (CFGetTypeID(value) == CFGetTypeID(CFSTR("YES"))))
        {
            retour = (CFStringCompare(value, CFSTR("YES"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) ? 1 : 0;
            
            CFRelease(value);
        }
        else retour = defaut;
        
        CFRelease(nom);
    }
    else retour = defaut;
    
    
    return retour;
}

#else /* Not __APPLE__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static const char * prefsfile = ".flobopuyorc";
static const char * sep = "\n\r";
static const char * equ = "=%d";

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

void SetBoolPreference(char * name, int value)
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
    
    fprintf(prefs,"%s=%d\n",name,value);
    
    fclose(prefs);
}


int GetBoolPreference(char * name, int defaut)
{
    char * key, *copiedfile;
    
    if (file==NULL) fetchFile();
    if (file==NULL) return defaut;
    
    char * tmp = (char *)malloc(strlen(name)+strlen(equ)+1);
    strcpy(tmp,name);
    strcat(tmp,equ);

    copiedfile = strdup(file);
    
    for (key = strtok(copiedfile, sep); key; key = strtok(NULL, sep))
    {
        int j;

        if (sscanf(key,tmp,&j)!=0)
        {
            free(tmp);
            free(copiedfile);
            return j?true:false;
        }
    }
    free(copiedfile);
    free(tmp);
    
    return defaut;
}

#endif /* Not __APPLE__ */

