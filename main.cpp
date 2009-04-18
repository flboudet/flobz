#include <stdlib.h>
#include <string.h>
#include "IosImgProcess.h"
#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include "SDL_main.h"
#else
#include <SDL_main.h>
#endif

#include "PuyoCommander.h"
#include "ios_exception.h"
#include "ios_memory.h"
#include "ios_filepath.h"

#ifndef DATADIR
char *DATADIR = "data";
#endif

using namespace ios_fc;

/*
#ifdef MACOSX
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
*/

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

static void help()
{
    cout << "usage: ./flobopuyo [options]" << endl;
    cout << endl;
    cout << "options are:" << endl;
    cout << "  -win           enable windowed mode" << endl;
    cout << "  -maxpack {n}   use data pack n (0..2)" << endl;
    cout << "  -gsl {file}    launch a gsl script" << endl;
    cout << endl;
    cout << "(c)2007, by iOS-Software (G.Borios, F.Boudet, J.C.Hoelt)." << endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    int i;
    String dataDir;
    int maxPackNumber = -1;

#ifdef MACOSX

#ifndef DATADIR
    const char *bundleDataPath = "/Contents/Resources/data";
    CFURLRef bundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
    CFRelease(bundleURL);
    int pathSize = (int)CFStringGetMaximumSizeForEncoding(CFStringGetLength(bundlePath), kCFStringEncodingUTF8) + 1;
    DATADIR = (char *)malloc(pathSize + strlen(bundleDataPath));
    CFStringGetCString (bundlePath, DATADIR, pathSize, kCFStringEncodingUTF8);
    strcat(DATADIR, bundleDataPath);
    CFRelease(bundlePath);
#endif

#endif

    bool fs = true;
    const char *gsl_screen = NULL;

    for (i=1; i<argc; i++)
    {
        // Help
        if ((strcmp(argv[i],"--help") == 0) || (strcmp(argv[i],"-h") == 0))
            help();
        // Fullscreen option
        if (strcmp(argv[i],"-win") == 0) fs = false;
        // Max data pack option
        if (strcmp(argv[i], "-maxpack") == 0) {
            if (++i < argc)
                maxPackNumber = atoi(argv[i]);
        }
        if (strcmp(argv[i], "-gsl") == 0) {
            if (++i < argc)
                gsl_screen = argv[i];
        }
    }

    if (!FilePath(DATADIR).exists())
        dataDir = "data";
    else
        dataDir = DATADIR;

    PuyoMain fp(dataDir, fs, maxPackNumber);
    try {
        if (gsl_screen != NULL)
            fp.debug_gsl(gsl_screen);
        else
            fp.run();
    }
    catch (Exception e) { displayExceptionMessage(e.what()); }
    catch (char * str) { displayExceptionMessage(str); }
    catch (String str) { displayExceptionMessage(str); }
    catch (...) { displayExceptionMessage("The exception is unknown."); }
    return 0;
}
