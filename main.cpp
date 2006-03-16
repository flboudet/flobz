#include <stdlib.h>
#include <string.h>
#include "IosImgProcess.h"
#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include "SDL_main.h"
#else
#include <SDL/SDL_main.h>
#endif

#include "PuyoCommander.h"
#include "ios_exception.h"
#include "ios_memory.h"

#ifndef DATADIR
char *DATADIR = "data";
#endif

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


static void displayExceptionMessage(const char * msg)
{
  cout << "Sorry, an unhandled exception occured. Giving up." << endl;
  cout << msg << endl;
  cout << "You can report this problem to ios@ios-software.com" << endl;
}

int main(int argc, char *argv[])
{
    int i;
    
  if (!strcmp(argv[argc-1],"-h")) {
      printf("-win for windowed mode.\n");
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
    
    for (i=1; i<argc; i++)
    {
        if (strcmp(argv[i],"-win") == 0) fs = false;
    }
    
    if (!fileExists("data")) setDataFolder(DATADIR);
    else setDataFolder("data");
    //printf("Datafolder set to %s\n", getDataFolder());
    
    PuyoCommander commander( fs );
    
    try { commander.run(); }
    catch (Exception e) { displayExceptionMessage(e.what()); }
    catch (char * str) { displayExceptionMessage(str); }
    catch (String str) { displayExceptionMessage(str); }
    catch (...) { displayExceptionMessage("The exception is unknown."); }
    return 0;
}
