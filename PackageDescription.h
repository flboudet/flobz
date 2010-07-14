#ifndef _PACKAGEDESCRIPTION_H_
#define _PACKAGEDESCRIPTION_H_

#include "CompositeDrawContext.h"
#include "DataPathManager.h"
#include "goomsl/goomsl.h"

class PackageDescription
{
public:
    static void end_graphic(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void sbind(GoomSL *gsl);
public:
    PackageDescription(DataPathManager &dataPathManager,
                       DataPackage     &package,
                       CompositeDrawContext &cDC);
private:
    CompositeDrawContext &m_cDC;
};

#endif // _PACKAGEDESCRIPTION_H_

