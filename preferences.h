/*
 *  preferences.h
 *  
 *
 *  Created by Guillaume Borios on 03/08/04.
 *  Copyright 2004 iOS. All rights reserved.
 *
 */


/* Set preferences */
void SetBoolPreference(char*name,int value);
void SetIntPreference(char*name,int value);


/* Get preferences */
int GetBoolPreference(char * name, int defaut);
int GetIntPreference(char * name, int defaut);

