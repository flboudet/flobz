//
//  GSLFileAccessWrapper.h
//  flobopop
//
//  Created by Florent Boudet on 06/04/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#ifndef _GSL_FILEACCESSWRAPPER_H
#define _GSL_FILEACCESSWRAPPER_H
#include "goomsl.h"
#include "DataPathManager.h"

void GSLFA_setupWrapper(GoomSL *gsl, const DataPathManager *dataPathManager, const DataProvider *target = NULL);

#endif // _GSL_FILEACCESSWRAPPER_H
