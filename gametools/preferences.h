/*
 *  preferences.h
 *  
 *
 *  Created by Guillaume Borios on 03/08/04.
 *  Copyright 2004 iOS. All rights reserved.
 *
 */

#ifndef PREF_H
#define PREF_H

/* Set preferences */
void SetBoolPreference(const char *name,int value);
void SetIntPreference (const char *name, int value);
void SetStrPreference (const char *name, const char *value);


/* Get preferences */
int  GetBoolPreference(const char *name, int defaut);
int  GetIntPreference (const char *name, int defaut);
void GetStrPreference (const char *name, char *out, const char *defaut, const int bufferSize = 256);

#endif
