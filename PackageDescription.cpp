#include <iostream>
#include "PackageDescription.h"
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"

using namespace std;

void PackageDescription::end_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local)
{
    cout << "end_graphic" << endl;
    PackageDescription *packDesc = (PackageDescription *)(GSL_GET_USERDATA_PTR(gsl));
    const char * graphicKey  = (const char *) GSL_GLOBAL_PTR(gsl, "graphic.key");
    const char * graphicPath = (const char *) GSL_GLOBAL_PTR(gsl, "graphic.path");
    IosRect graphicRect = { GSL_GLOBAL_INT(gsl, "graphic.crop.x"),
                            GSL_GLOBAL_INT(gsl, "graphic.crop.y"),
                            GSL_GLOBAL_INT(gsl, "graphic.crop.w"),
                            GSL_GLOBAL_INT(gsl, "graphic.crop.h") };
    packDesc->m_cDC.declareCompositeSurface(graphicKey, graphicPath, graphicRect);
    cout << "Declare " << graphicKey << endl;
}

void PackageDescription::sbind(GoomSL *gsl)
{
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

