#ifndef _PACKAGEDESCRIPTION_H_
#define _PACKAGEDESCRIPTION_H_

#include "CompositeDrawContext.h"
#include "DataPathManager.h"
#include "audio.h"
#include "goomsl.h"

class PackageDescription
{
public:
    static void gsl_strcat(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void start_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void define_crop(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void define_crop_x(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void start_music(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void define_track(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void end_music(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void sbind(GoomSL *gsl);
public:
    PackageDescription(DataPathManager &dataPathManager,
                       DataPackage     &package,
                       CompositeDrawContext *cDC,
                       Jukebox *jukebox = NULL);
    PackageDescription(GoomSL *gsl,
                       CompositeDrawContext *cDC,
                       Jukebox *jukebox = NULL);
private:
    CompositeDrawContext *m_cDC;
    Jukebox *m_jukebox;
    std::string m_graphicBeingDefinedPath;
    std::string m_musicBeingDefinedPath;
};

#endif // _PACKAGEDESCRIPTION_H_

