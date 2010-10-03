#include <iostream>
#include "PackageDescription.h"
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"

using namespace std;

void PackageDescription::start_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)GSL_GET_USERDATA_PTR(gsl);
    const char * path = (const char *)GSL_LOCAL_PTR(gsl, local, "path");
    packageDesc->m_graphicBeingDefinedPath = path;
}
void PackageDescription::define_crop(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    PackageDescription *packageDesc = (PackageDescription *)GSL_GET_USERDATA_PTR(gsl);
    const char * key = (const char *)GSL_LOCAL_PTR(gsl, local, "key");
    IosRect graphicRect = { GSL_LOCAL_INT(gsl, local, "x"),
                            GSL_LOCAL_INT(gsl, local, "y"),
                            GSL_LOCAL_INT(gsl, local, "w"),
                            GSL_LOCAL_INT(gsl, local, "h") };
    packageDesc->m_cDC.declareCompositeSurface(key,
               packageDesc->m_graphicBeingDefinedPath.c_str(), graphicRect);
}
void PackageDescription::end_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
}

void PackageDescription::sbind(GoomSL *gsl)
{
    gsl_bind_function(gsl, "start_graphic", start_graphic);
    gsl_bind_function(gsl, "define_crop",   define_crop);
    gsl_bind_function(gsl, "end_graphic",   end_graphic);
}

PackageDescription::PackageDescription(DataPathManager &dataPathManager,
                                       DataPackage     &package,
                                       CompositeDrawContext &cDC)
    : m_cDC(cDC)
{
    cout << "PackageDescription" << endl;
    String libPath = dataPathManager.getPath("lib/packagelib.gsl");
    String scriptPath;
    try {
        scriptPath = package.getPath("Description.gsl").c_str();
    }
    catch (...) {
        cerr << "Warning: No description file in package " << package.getName() << endl;
        return;
    }
    GoomSL * gsl = gsl_new();
    if (!gsl) return;
    GSL_SET_USERDATA_PTR(gsl, this);
//dataPathManager.getPath("Description.gsl");
    char * fbuffer = gsl_init_buffer((const char *)libPath);
    gsl_append_file_to_buffer((const char *)scriptPath, &fbuffer);
    gsl_compile(gsl,fbuffer);
    sbind(gsl);
    gsl_execute(gsl);
    gsl_free(gsl);
    free(fbuffer);
    cout << "PackageDescription end" << endl;
}

