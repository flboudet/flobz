#include <stdlib.h>
#include <string.h>
#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "PuyoCommander.h"

#ifndef DATADIR
char *DATADIR = "data";
#endif

char *dataFolder = "data";

#ifdef MACOSX
const char *bundleDataPath = "/Contents/Resources/data";
void show(CFStringRef formatString, ...) {
    CFStringRef resultString;
    CFDataRef data;
    va_list argList;
    
    va_start(argList, formatString);
    resultString = CFStringCreateWithFormatAndArguments(NULL, NULL, formatString, argList);
    va_end(argList);
    
    data = CFStringCreateExternalRepresentation(NULL, resultString, CFStringGetSystemEncoding(), '?');
    
    if (data != NULL) {
        printf ("%.*s\n\n", (int)CFDataGetLength(data), CFDataGetBytePtr(data));
        CFRelease(data);
    }
    
    CFRelease(resultString);
}
#endif

bool fileExists(char *path)
{
    FILE *f;
    f = fopen(path, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}

#include <SDL/SDL_main.h>

int main(int argc, char *argv[])
{
    int i;
    
  if (!strcmp(argv[argc-1],"-h")) {
      printf("-win for windowed mode.\n");
      printf("-quiet to remove music.\n");
      printf("-nofx to remove sound FX.\n");
    return 0;
  }
    
#ifdef MACOSX

#ifndef DATADIR
    CFStringRef bundlePath = CFURLCopyFileSystemPath(CFBundleCopyBundleURL(CFBundleGetMainBundle()), kCFURLPOSIXPathStyle);
    int pathSize = (int)CFStringGetMaximumSizeForEncoding(CFStringGetLength(bundlePath), CFStringGetSystemEncoding()) + 1;
    DATADIR = (char *)malloc(pathSize + strlen(bundleDataPath));
    CFStringGetCString (bundlePath, DATADIR, pathSize, CFStringGetSystemEncoding());
    strcat(DATADIR, bundleDataPath);
    
    //show(CFSTR("test   : %@"), CFURLCopyFileSystemPath(CFBundleCopyBundleURL(CFBundleGetMainBundle()), kCFURLPOSIXPathStyle));
    //fprintf(stderr, "bundle=%d path=%s\n", CFBundleGetMainBundle(), DATADIR);
    //fprintf(stderr, "Attention ça va planter2\n");
    //CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle(),kCFStringEncodingUnicode);
    //DATADIR = strdup(CFStringGetCStringPtr(CFURLGetString( CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle())), kCFStringEncodingUnicode));
    //fprintf(stderr, "Resource path: %s\n", DATADIR);
#endif
#endif
 
    bool fs = true;
    bool snd = true;
    bool fx = true;
    
    for (i=1; i<argc; i++)
    {
        if (strcmp(argv[i],"-win") == 0) fs = false;
        if (strcmp(argv[i],"-quiet") == 0) snd = false;
        if (strcmp(argv[i],"-nofx") == 0) fx = false;
    }
    
    if (!fileExists("data/gfx/v0.png")) {
        dataFolder = DATADIR;
    }
    
    PuyoCommander commander( fs, snd, fx );

    
  commander.run();
  return 0;
}
