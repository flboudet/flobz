//
//  GSLFileAccessWrapper.cpp
//  flobopop
//
//  Created by Florent Boudet on 06/04/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "GSLFileAccessWrapper.h"
#include "GTLog.h"

static goomsl_file GSLFA_openFile(GoomSL *gsl, const char *file_name)
{
    DataProvider *dp = NULL;
    if (strncmp("@/", file_name, 2) == 0) {
        // Package mode
        dp = (DataProvider *)gsl_get_userdata(gsl, "GSLFA.Target");
        file_name += 1;
    }
    else {
        // Datapath Manager mode
        dp = (DataPathManager *)gsl_get_userdata(gsl, "GSLFA.DataPathManager");
    }
    if (dp->hasDataInputStream(file_name))
        return (goomsl_file)(dp->openDataInputStream(file_name));
    else
        return NULL;
}
static void GSLFA_closeFile(GoomSL *gsl, goomsl_file file)
{
    DataInputStream *f = (DataInputStream *)file;
    if (f)
        delete f;
}
static int GSLFA_readFile(GoomSL *gsl, void *buffer, goomsl_file file, int read_size)
{
    DataInputStream *f = (DataInputStream *)file;
    if (f)
        return f->streamRead(buffer, read_size);
    else
        return 0;
}

void GSLFA_setupWrapper(GoomSL *gsl, const DataPathManager *dataPathManager, const DataProvider *target)
{
    GTLogTrace("++");
    gsl_set_userdata(gsl, "GSLFA.DataPathManager", dataPathManager);
    gsl_set_userdata(gsl, "GSLFA.Target", target);
    gsl_bind_file_functions(gsl, GSLFA_openFile, GSLFA_closeFile, GSLFA_readFile);
    GTLogTrace("--");
}
