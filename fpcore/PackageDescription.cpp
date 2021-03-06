#include <iostream>
#include "GTLog.h"
#include "PackageDescription.h"
#include "goomsl.h"
#include "GSLFileAccessWrapper.h"

using namespace std;

void PackageDescription::gsl_strcat(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    static bool firstTime = true;
    const char *a = (const char *)GSL_LOCAL_PTR(gsl, local, "a");
    const char *b = (const char *)GSL_LOCAL_PTR  (gsl, local, "b");
    int *globalPtrReturn = (int*)goom_hash_get(gsl_globals(gsl), "strcat")->ptr;
    if ((!firstTime) && (gsl_get_ptr(gsl, *globalPtrReturn))) {
        gsl_free_ptr(gsl, *globalPtrReturn);
        firstTime = false;
    }
    int newPtrId = gsl_malloc(gsl, strlen(a)+strlen(b)+1); // allocate a new pointer (should we allow realloc?)
    char *returnPtr = (char*)gsl_get_ptr(gsl, newPtrId);
    strcpy(returnPtr, a);
    strcat(returnPtr, b);
    *globalPtrReturn = newPtrId;
}

void PackageDescription::start_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)gsl_get_userdata(gsl, "PackageDescription");
    const char * path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    packageDesc->m_graphicBeingDefinedPath = path;
}
void PackageDescription::define_crop(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)gsl_get_userdata(gsl, "PackageDescription");
    const char * key = (const char *)GSL_LOCAL_PTR(gsl, local, "key");
    IosRect graphicRect = { GSL_LOCAL_INT(gsl, local, "x"),
                            GSL_LOCAL_INT(gsl, local, "y"),
                            GSL_LOCAL_INT(gsl, local, "w"),
                            GSL_LOCAL_INT(gsl, local, "h") };
    packageDesc->m_cDC->declareCompositeSurface(key,
               packageDesc->m_graphicBeingDefinedPath.c_str(), graphicRect);
}
void PackageDescription::define_crop_x(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)gsl_get_userdata(gsl, "PackageDescription");
    const char * key = (const char *)GSL_LOCAL_PTR(gsl, local, "key");
    IosRect graphicRect = { GSL_LOCAL_INT(gsl, local, "x"),
                            GSL_LOCAL_INT(gsl, local, "y"),
                            GSL_LOCAL_INT(gsl, local, "w"),
                            GSL_LOCAL_INT(gsl, local, "h") };
    packageDesc->m_cDC->declareCompositeSurface(key,
               packageDesc->m_graphicBeingDefinedPath.c_str(), graphicRect,
                                                GSL_LOCAL_INT(gsl, local, "ww"),
                                                GSL_LOCAL_INT(gsl, local, "hh"));
}
void PackageDescription::end_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
}

void PackageDescription::start_music(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)gsl_get_userdata(gsl, "PackageDescription");
    const char * path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    packageDesc->m_musicBeingDefinedPath = path;
}
void PackageDescription::define_track(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)gsl_get_userdata(gsl, "PackageDescription");
    const char * key = (const char *)GSL_LOCAL_PTR(gsl, local, "key");
    int position = GSL_LOCAL_INT(gsl, local, "pos");
    if (packageDesc->m_jukebox != NULL)
        packageDesc->m_jukebox->registerTrack(key, packageDesc->m_musicBeingDefinedPath.c_str(), position);
}
void PackageDescription::end_music(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
}

void PackageDescription::sbind(GoomSL *gsl)
{
    gsl_bind_function(gsl, "strcat", gsl_strcat);
    gsl_bind_function(gsl, "start_graphic", start_graphic);
    gsl_bind_function(gsl, "define_crop",   define_crop);
    gsl_bind_function(gsl, "define_crop_x", define_crop_x);
    gsl_bind_function(gsl, "end_graphic",   end_graphic);

    gsl_bind_function(gsl, "start_music",  start_music);
    gsl_bind_function(gsl, "define_track", define_track);
    gsl_bind_function(gsl, "end_music",    end_music);
}

PackageDescription::PackageDescription(DataPathManager &dataPathManager,
                                       DataPackage     &package,
                                       CompositeDrawContext *cDC,
                                       Jukebox *jukebox)
    : m_cDC(cDC), m_jukebox(jukebox)
{
    GTLogTrace("++");
    if (package.hasFile("Description.gsl")) {
        /*
           catch (ios_fc::Exception e) {
           GTLogTrace("Warning: No description file in package %s", package.getName().c_str());
           return;
           }
           catch (...) {
           GTLogTrace("Error while checking for a file in package %s", package.getName().c_str());
           return;
           } */
        GoomSL * gsl = gsl_new();
        if (!gsl) return;
        GSLFA_setupWrapper(gsl, &dataPathManager, &package);
        GTLogTrace("Wrapper Set");
        gsl_set_userdata(gsl, "PackageDescription", this);
        gsl_push_file(gsl, "/lib/packagelib.gsl");
        gsl_push_file(gsl, "@/Description.gsl");
        gsl_compile(gsl);
        sbind(gsl);
        gsl_execute(gsl);
        gsl_free(gsl);
    }
    else {
        GTLogTrace("Warning: No description file in package %s", package.getName().c_str());
    }
    GTLogTrace("--");
}

PackageDescription::PackageDescription(GoomSL *gsl,
                                       CompositeDrawContext *cDC,
                                       Jukebox *jukebox)
    : m_cDC(cDC), m_jukebox(jukebox)
{
    gsl_set_userdata(gsl, "PackageDescription", this);
    sbind(gsl);
}
