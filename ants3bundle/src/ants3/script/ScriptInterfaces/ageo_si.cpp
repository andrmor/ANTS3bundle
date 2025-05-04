#include "ageo_si.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "afiletools.h"
#include "avector.h"

#include <QDebug>

#include <array>

AGeo_SI::AGeo_SI() :
    AScriptInterface(), GeoHub(AGeometryHub::getInstance())
{
    Description = "Geometry configurator. Based on CERN ROOT TGeoManager";

    QString s = "It is filled with the given material (material index iMat)\n"
                "and positioned inside 'container' object\n"
                "at coordinates (x,y,z) and orientation angles (phi,thetha,psi)\n"
                "in the local frame of the container;\n"
                "Requires updateGeometry() to take effect!";

    Help["box"] = "Add a box 'name' with the sizes (sizeX,sizeY,sizeZ).\n" + s;
    Help["cylinder"] = "Add a cylinder 'name' with the given diameter and height.\n" + s;
    Help["tube"] = "Add a tube 'name' with the given outer and inner diameters and height.\n" + s;
    Help["cone"] =  "Add a cone 'name' with the given top and bottom diameters and height.\n" +s;
    Help["polygone"] =  "Add a polygon 'name' with the given number of edges, top and bottom diameters of the inscribed "
                        "circles and height.\n" +s;
    Help["sphere"] = "Add a sphere 'name' with the given diameter.\n" + s;
    Help["srb8"] = "Add a TGeoArb8 object with name 'name' and the given height and two arrays,\n"
                   "containing X and Y coordinates of the nodes.\n"+ s;
    Help["customTGeo"] = "Add an object with name 'name' and the shape generated using the CERN ROOT geometry system.\n" + s +
                   "\nFor example, to generate a box of (10,10,10) half sizes use the generation string \"TGeoBBox(10, 10, 10)\".\n"
                   "To generate a composite object, first create logical volumes (using TGeo command or \"Box\" etc), and then "
                   "create the composite using, e.g., the generation string \"TGeoCompositeShape( name1 + name2 )\". Note that the logical volume is removed "
                   "from the generation list after it was used by composite object generator!";

    Help["setLineProperties"] = "Set color, width and style of the line for visualisation of the object \"name\".";
    Help["clearWorld"] = "Remove all objects and prototypes leaving only World";

    Help["clearHosted"] = "Remove all objects hosted inside the given Object.\nRequires updateGeometry().";
    Help["removeWithHosted"] = "Remove the object and all objects hosted inside.\nRequires updateGeometry().";

    Help["updateGeometry"] = "Update geometry and optionally run the geometry check for conflicts.\n"
                             "To update the view at the geometry window use geowin.redraw()";

    Help["stack"] = "Add empty stack. Objects can be added to the stack by giving the stack name as their container.";
    //                "After the last element is added, call InitializeStack(StackName, Origin) function. "
    //                "It will automatically calculate x,y and z positions of all elements, keeping user-configured xyz position of the Origin element.";
    //Help["initializeStack"] = "Call this function after the last element has been added to the stack."
    //                          "It will automatically calculate x,y and z positions of all elements, keeping user-configured xyz position of the Origin element.";

    Help["setEnabled"] = "Enable or disable the volume with the providfed name, or,\n"
                         "if the name ends with '*', all volumes with the name starting with the provided string.)\n"
                         "Requires updateGeometry() to take effect!";

    Help["getPassedVoulumes"] = "Go through the defined geometry in a straight line from startXYZ in the direction startVxVyVz\n"
                                "and return array of [X Y Z MaterualIndex VolumeName NodeIndex] for all volumes on the way until final exit to the World\n"
                                "the X Y Z are coordinates of the entrance points";
}

AGeo_SI::~AGeo_SI()
{
    clearGeoObjects();
}

bool AGeo_SI::beforeRun()
{
    clearGeoObjects();
    return true;
}

/*
void AGeo_SI::box(QString name, double Lx, double Ly, double Lz, int iMat, QString container,
                  double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoBox(0.5*Lx, 0.5*Ly, 0.5*Lz),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

bool AGeo_SI::checkPosOri(QVariantList position, QVariantList orientation, std::array<double,3> & pos, std::array<double,3> & ori)
{
    if (position.size() != 3)
    {
        abort("position argument should be an array of X, Y and Z");
        return false;
    }
    if (orientation.size() != 3)
    {
        abort("position argument should be an array of phi, theta and psi");
        return false;
    }
    for (size_t i = 0; i < 3; i++)
    {
        pos[i] = position[i].toDouble();
        ori[i] = orientation[i].toDouble();
    }
    return true;
}

void AGeo_SI::box(QString name, QVariantList fullSizes, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori, L;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    if (fullSizes.size() != 3)
    {
        abort("fullSizes should be an array of full sizes in x, y and z directions");
        return;
    }
    for (size_t i = 0; i < 3; i++) L[i] = fullSizes[i].toDouble();

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoBox(0.5 * L[0], 0.5 * L[1], 0.5 * L[2]), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::parallelepiped(QString name, double Lx, double Ly, double Lz, double Alpha, double Theta, double Phi, int iMat, QString container,
                             double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoPara(0.5*Lx, 0.5*Ly, 0.5*Lz, Alpha, Theta, Phi),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::parallelepiped(QString name, QVariantList fullSizes, QVariantList angles, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori, L, A;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    if (fullSizes.size() != 3)
    {
        abort("'fullSizes' should be an array of full sizes in x, y and z directions");
        return;
    }
    if (angles.size() != 3)
    {
        abort("'angles' should be an array of three angles");
        return;
    }
    for (size_t i = 0; i < 3; i++)
    {
        L[i] = fullSizes[i].toDouble();
        A[i] = angles[i].toDouble();
    }

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoPara(0.5*L[0], 0.5*L[1], 0.5*L[2], A[0], A[1], A[2]), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoTrd1(0.5*LXlow, 0.5*LXup, 0.5*Ly, 0.5*Lz),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTrd1(0.5*LXlow, 0.5*LXup, 0.5*Ly, 0.5*Lz), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoTrd2(0.5*LXlow, 0.5*LXup, 0.5*LYlow, 0.5*LYup, 0.5*Lz),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTrd2(0.5*LXlow, 0.5*LXup, 0.5*LYlow, 0.5*LYup, 0.5*Lz), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::cylinder(QString name, double outerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTube(0, 0.5*outerD, 0.5*h), x,y,z, phi,theta,psi);

    GeoObjects.push_back(o);
}
*/

void AGeo_SI::cylinder(QString name, double outerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTube(0, 0.5*outerD, 0.5*h), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoTube(0.5*innerD, 0.5*outerD, 0.5*h),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::tube(QString name, double outerD, double innerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }

    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTube(0.5*innerD, 0.5*outerD, 0.5*h), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoTubeSeg(0.5*innerD, 0.5*outerD, 0.5*h, Phi1, Phi2),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }

    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTubeSeg(0.5*innerD, 0.5*outerD, 0.5*h, Phi1, Phi2), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2,
                      QVariantList Nlow, QVariantList Nhigh,
                      int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }
    if (Nlow.size() != 3 || Nhigh.size() != 3)
    {
        abort("Nlow and Nhigh should be unitary vectors (size=3) of the normals to the cuts");
        return;
    }

    std::array<double, 3> al, ah;
    for (int i = 0; i < 3; i++)
    {
        al[i] = Nlow[i] .toDouble();
        ah[i] = Nhigh[i].toDouble();
    }

    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoCtub(0.5*innerD, 0.5*outerD, 0.5*h, Phi1, Phi2, al[0], al[1], al[2], ah[0], ah[1], ah[2]),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter should be smaller than the outer one");
        return;
    }
    if (Nlow.size() != 3 || Nhigh.size() != 3)
    {
        abort("Nlow and Nhigh should be unitary vectors (size=3) of the normals to the cuts");
        return;
    }

    std::array<double, 3> al, ah;
    for (int i = 0; i < 3; i++)
    {
        al[i] = Nlow[i] .toDouble();
        ah[i] = Nhigh[i].toDouble();
    }

    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoCtub(0.5*innerD, 0.5*outerD, 0.5*h, Phi1, Phi2, al[0], al[1], al[2], ah[0], ah[1], ah[2]), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoEltu(0.5*Dx, 0.5*Dy, 0.5*height),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoEltu(0.5*Dx, 0.5*Dy, 0.5*height), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::polygon(QString name, int edges, double inscribDiameter, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                    new AGeoPolygon(edges, 0.5*h, 0.5*inscribDiameter, 0.5*inscribDiameter),
                                    x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::polygon(QString name, int edges, double inscribDiameter, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoPolygon(edges, 0.5*h, 0.5*inscribDiameter, 0.5*inscribDiameter), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                    new AGeoPolygon(edges, dPhi, 0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut),
                                    x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoPolygon(edges, dPhi, 0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (numEdges < 3)
    {
        abort("Number of edges should be at least 3");
        return;
    }
    AGeoPgon * p = new AGeoPgon();
    p->nedges = numEdges;
    p->phi    = Phi;
    p->dphi   = dPhi;
    p->Sections.clear();

    std::vector<std::array<double,3>> vecSections;
    bool ok = getSectionsPoly(sections, vecSections);
    if (!ok) return;

    for (const auto & s : vecSections)
        p->Sections.push_back( APolyCGsection(s[0], 0.5*s[1], 0.5*s[2]) );   //z rmin rmax

    AGeoObject * o = new AGeoObject(name, container, iMat, p,
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    if (numEdges < 3)
    {
        abort("Number of edges should be at least 3");
        return;
    }

    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoPgon * p = new AGeoPgon();
    p->nedges = numEdges;
    p->phi    = Phi;
    p->dphi   = dPhi;
    p->Sections.clear();

    std::vector<std::array<double,3>> vecSections;
    ok = getSectionsPoly(sections, vecSections);
    if (!ok) return;

    for (const auto & s : vecSections)
        p->Sections.push_back( APolyCGsection(s[0], 0.5*s[1], 0.5*s[2]) );   //z rmin rmax

    AGeoObject * o = new AGeoObject(name, container, iMat, p, pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoCone(0.5*h, 0.5*Dbot, 0.5*Dtop),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoCone(0.5*h, 0.5*Dbot, 0.5*Dtop), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoCone(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::conicalTube(QString name, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoCone(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::coneSegment(QString name, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoConeSeg(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut, phi1, phi2),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::coneSegment(QString name, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoConeSeg(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut, phi1, phi2), pos, ori);

    GeoObjects.push_back(o);
}

bool AGeo_SI::getSectionsPoly(const QVariantList & sections, std::vector<std::array<double,3>> & vecSections)
{
    if (sections.size() < 2)
    {
        abort("There should be at least 2 sections for pCone and pGon");
        return false;
    }

    for (int iVar = 0; iVar < sections.size(); iVar++)
    {
        QVariantList el = sections[iVar].toList();
        if (el.size() != 3)
        {
            abort("Each section of pCone and pGon should have 3 elements: z,innerDiameter and outerDiameter");
            return false;
        }
        bool ok1, ok2, ok3;
        double z    = el[0].toDouble(&ok1);
        double din  = el[1].toDouble(&ok2);
        double dout = el[2].toDouble(&ok3);
        if (!ok1 || !ok2 || !ok3)
        {
            abort("Error in convertion to double for one of the sections");
            return false;
        }
        if (din >= dout)
        {
            abort("Inner diameter should be smaller than outer diamter!");
            return false;
        }
        vecSections.push_back({z, din, dout});
    }

    double lastZ = vecSections[0][0];
    for (size_t i = 1; i < vecSections.size(); i++)
    {
        if (vecSections[i][0] < lastZ)
        {
            abort("Section z coordinates are not in increasing order");
            return false;
        }
        lastZ = vecSections[i][0];
    }
    return true;
}

/*
void AGeo_SI::pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoPcon * p = new AGeoPcon();
    p->phi  = Phi;
    p->dphi = dPhi;
    p->Sections.clear();

    std::vector<std::array<double,3>> vecSections;
    bool ok = getSectionsPoly(sections, vecSections);
    if (!ok) return;

    for (const auto & s : vecSections)
        p->Sections.push_back( APolyCGsection(s[0], 0.5*s[1], 0.5*s[2]) );   //z rmin rmax

    AGeoObject * o = new AGeoObject(name, container, iMat, p,
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoPcon * p = new AGeoPcon();
    p->phi  = Phi;
    p->dphi = dPhi;
    p->Sections.clear();

    std::vector<std::array<double,3>> vecSections;
    ok = getSectionsPoly(sections, vecSections);
    if (!ok) return;

    for (const auto & s : vecSections)
        p->Sections.push_back( APolyCGsection(s[0], 0.5*s[1], 0.5*s[2]) );   //z rmin rmax

    AGeoObject * o = new AGeoObject(name, container, iMat, p, pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::sphere(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoSphere(0.5*Dout, 0.5*Din),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::sphere(QString name, double Dout, double Din, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoSphere(0.5*Dout, 0.5*Din), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::sphereSector(QString name, double Dout, double Din, double Theta1, double Theta2, double Phi1, double Phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoSphere(0.5*Dout, 0.5*Din, Theta1, Theta2, Phi1, Phi2),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::sphereSector(QString name, double Dout, double Din, double theta1, double theta2, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoSphere(0.5*Dout, 0.5*Din, theta1, theta2, phi1, phi2), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoTorus(0.5*D, 0.5*Din, 0.5*Dout, Phi, dPhi),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoTorus(0.5*D, 0.5*Din, 0.5*Dout, Phi, dPhi), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoParaboloid(0.5*Dbot, 0.5*Dup, 0.5*h),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoParaboloid(0.5*Dbot, 0.5*Dup, 0.5*h), pos, ori);

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::composite(QString name, QString compositionString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                    new AGeoBox(),
                                    x,y,z, phi,theta,psi);

    QString s = compositionString.simplified();
    s.remove("(");
    s.remove(")");
    s.remove("+");
    s.remove("*");
    s.remove("-");
    QStringList members = s.split(' ', Qt::SkipEmptyParts);

    //create an empty composite object
    AGeometryHub::getInstance().convertObjToComposite(o);
    o->clearCompositeMembers();

    //attempt to add logicals
    for (int iMem = 0; iMem < members.size(); iMem++)
    {
        int index = -1;
        for (int iObj = 0; iObj < (int)GeoObjects.size(); iObj++)
        {
            if (members[iMem] == GeoObjects[iObj]->Name)
            {
                index = iObj;
                break;
            }
        }
        if (index == -1)
        {
            delete o;
            abort("Error in composite object generation: logical volume "+members[iMem]+" not found!");
            return;
        }
        //found logical, transferring it to logicals container of the compsoite
        o->getContainerWithLogical()->addObjectLast(GeoObjects[index]);
        GeoObjects.erase(GeoObjects.begin() + index);
    }
    o->refreshShapeCompositeMembers();

    bool ok = o->readShapeFromString( QString("TGeoCompositeShape( %0 )").arg(compositionString) );
    if (!ok)
    {
        delete o;
        abort(name + ": failed to create composite shape");
        return;
    }
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::composite(QString name, QString compositionString, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoBox(), pos, ori);

    QString s = compositionString.simplified();
    s.remove("(");
    s.remove(")");
    s.remove("+");
    s.remove("*");
    s.remove("-");
    QStringList members = s.split(' ', Qt::SkipEmptyParts);

    //create an empty composite object
    AGeometryHub::getInstance().convertObjToComposite(o);
    o->clearCompositeMembers();

    //attempt to add logicals
    for (int iMem = 0; iMem < members.size(); iMem++)
    {
        int index = -1;
        for (int iObj = 0; iObj < (int)GeoObjects.size(); iObj++)
        {
            if (members[iMem] == GeoObjects[iObj]->Name)
            {
                index = iObj;
                break;
            }
        }
        if (index == -1)
        {
            delete o;
            abort("Error in composite object generation: logical volume "+members[iMem]+" not found!");
            return;
        }
        //found logical, transferring it to logicals container of the compsoite
        o->getContainerWithLogical()->addObjectLast(GeoObjects[index]);
        GeoObjects.erase(GeoObjects.begin() + index);
    }
    o->refreshShapeCompositeMembers();

    ok = o->readShapeFromString( QString("TGeoCompositeShape( %0 )").arg(compositionString) );
    if (!ok)
    {
        delete o;
        abort(name + ": failed to create composite shape");
        return;
    }

    GeoObjects.push_back(o);
}

/*
void AGeo_SI::arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (NodesXY.size() != 8)
    {
        abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
        return;
    }

    std::array<std::pair<double, double>, 8> nodes;
    bool ok1, ok2;
    for (int i = 0; i < 8; i++)
    {
        QVariantList el = NodesXY[i].toList();
        if (el.size() != 2)
        {
            abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
            return;
        }
        double x = el[0].toDouble(&ok1);
        double y = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
            return;
        }
        nodes[i] = {x,y};
    }

    if (!AGeoArb8::checkPointsForArb8(nodes))
    {
        abort("Arb8 nodes should be defined clockwise for both planes");
        return;
    }

    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoArb8(0.5*h, nodes),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    if (NodesXY.size() != 8)
    {
        abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
        return;
    }

    std::array<std::pair<double, double>, 8> nodes;
    bool ok1, ok2;
    for (int i = 0; i < 8; i++)
    {
        QVariantList el = NodesXY[i].toList();
        if (el.size() != 2)
        {
            abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
            return;
        }
        double x = el[0].toDouble(&ok1);
        double y = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("arb8 NodesXY array: should be an array of 8 elements of [x, y] arrays");
            return;
        }
        nodes[i] = {x,y};
    }

    if (!AGeoArb8::checkPointsForArb8(nodes))
    {
        abort("Arb8 nodes should be defined clockwise for both planes");
        return;
    }

    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, iMat, new AGeoArb8(0.5*h, nodes), pos, ori);

    GeoObjects.push_back(o);
}

void AGeo_SI::toScaled(QString name, double xFactor, double yFactor, double zFactor)
{
    AGeoObject * obj = nullptr;

    for (AGeoObject * GO : GeoObjects)
    {
        if (GO->Name == name)
        {
            obj = GO;
            break;
        }
    }

    if (!obj) //looking through already defined objects in the geometry
        obj = AGeometryHub::getInstance().World->findObjectByName(name);

    if (!obj)
    {
        abort("Cannot find object " + name);
        return;
    }

    // !!!***
    if (obj->Shape->getShapeType() == "TGeoCompositeShape")
    {
        abort("Cannot scale a composite shape!");
        return;
    }

    AGeoScaledShape * scs = dynamic_cast<AGeoScaledShape*>(obj->Shape);
    if (scs)
    {
        scs->scaleX = xFactor;
        scs->scaleY = yFactor;
        scs->scaleZ = zFactor;
    }
    else
    {
        scs = new AGeoScaledShape();
        scs->scaleX = xFactor;
        scs->scaleY = yFactor;
        scs->scaleZ = zFactor;

        scs->BaseShape = obj->Shape;
        obj->Shape = scs;
    }
}

/*
void AGeo_SI::monitor(QString name, int shape, double size1, double size2, QString container, double x, double y, double z, double phi, double theta, double psi, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking)
{
    AGeoObject * o = new AGeoObject(name, container, 0,    // no material -> it will be updated on build
                                    0,                     // no shape yet
                                    x,y,z, phi,theta,psi);

    ATypeMonitorObject* mto = new ATypeMonitorObject();
    delete o->Type; o->Type = mto;

    AMonitorConfig & mc = mto->config;
    mc.shape = shape;
    mc.size1 = 0.5 * size1;
    mc.size2 = 0.5 * size2;
    mc.bUpper = SensitiveTop;
    mc.bLower = SensitiveBottom;
    mc.bStopTracking = StopsTraking;

    //o->updateMonitorShape();
    o->color = 1;

    GeoObjects.push_back(o);
}
*/

void AGeo_SI::monitor(QString name, int shape, double size1, double size2, QString container, QVariantList position, QVariantList orientation, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, 0, nullptr, pos, ori);

    ATypeMonitorObject* mto = new ATypeMonitorObject();
    delete o->Type; o->Type = mto;

    AMonitorConfig & mc = mto->config;
    mc.shape = shape;
    mc.size1 = 0.5 * size1;
    mc.size2 = 0.5 * size2;
    mc.bUpper = SensitiveTop;
    mc.bLower = SensitiveBottom;
    mc.bStopTracking = StopsTraking;

    //o->updateMonitorShape();
    o->color = 1;

    GeoObjects.push_back(o);
}

void AGeo_SI::configurePhotonMonitor(QString monitorName, QVariantList position, QVariantList time, QVariantList angle, QVariantList wave)
{
    AGeoObject * o = nullptr;
    for (AGeoObject * obj : GeoObjects)
    {
        if (obj->Name == monitorName)
        {
            o = obj;
            break;
        }
    }

    if (!o)
    {
        abort("Cannot find monitor \"" + monitorName + "\"");
        return;
    }

    if (!o->Type || !o->Type->isMonitor())
    {
        abort(monitorName + " is not a monitor object!");
        return;
    }

    ATypeMonitorObject * m = static_cast<ATypeMonitorObject*>(o->Type);
    AMonitorConfig & mc = m->config;

    mc.PhotonOrParticle = 0;

    if (!position.isEmpty())
    {
        if (position.size() == 2)
        {
            mc.xbins = position.at(0).toInt();
            mc.ybins = position.at(1).toInt();
        }
        else
        {
            abort("Monitor config: Position argument should be either an empty array for default settings or an array of two integers (binsx and binsy)");
            return;
        }
    }

    if (!time.isEmpty())
    {
        if (time.size() == 3)
        {
            mc.timeBins = time.at(0).toInt();
            mc.timeFrom = time.at(1).toDouble();
            mc.timeTo   = time.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Time argument should be either an empty array for default settings or an array of [bins, from, to]");
            return;
        }
    }

    if (!angle.isEmpty())
    {
        if (angle.size() == 3)
        {
            mc.angleBins = angle.at(0).toInt();
            mc.angleFrom = angle.at(1).toDouble();
            mc.angleTo   = angle.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Angle argument should be either an empty array for default settings or an array of [bins, degreesFrom, degreesTo]");
            return;
        }
    }

    if (!wave.isEmpty())
    {
        if (wave.size() == 3)
        {
            mc.waveBins = wave.at(0).toInt();
            mc.waveFrom = wave.at(1).toDouble();
            mc.waveTo   = wave.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Wave argument should be either an empty array for default settings or an array of [bins, from, to]");
            return;
        }
    }
}

void AGeo_SI::configureParticleMonitor(QString monitorName, QString particle, int both_Primary_Secondary, int both_Direct_Indirect,
                                       QVariantList position, QVariantList time, QVariantList angle, QVariantList energy)
{
    AGeoObject * o = nullptr;
    for (AGeoObject * obj : GeoObjects)
        if (obj->Name == monitorName)
        {
            o = obj;
            break;
        }

    if (!o)
    {
        abort("Cannot find monitor \"" + monitorName + "\"");
        return;
    }

    if (!o->Type || !o->Type->isMonitor())
    {
        abort(monitorName + " is not a monitor object!");
        return;
    }

    ATypeMonitorObject * m = static_cast<ATypeMonitorObject*>(o->Type);
    AMonitorConfig & mc = m->config;

    mc.PhotonOrParticle = 1;
    mc.Particle = particle;

    switch (both_Primary_Secondary)
    {
    case 0: mc.bPrimary = true;  mc.bSecondary = true;  break;
    case 1: mc.bPrimary = true;  mc.bSecondary = false; break;
    case 2: mc.bPrimary = false; mc.bSecondary = true;  break;
    default: abort("Both_Primary_Secondary: 0 - sensitive to both, 1 - sensetive only to primary, 2 - sensitive only to secondary"); return;
    }
    switch (both_Direct_Indirect)
    {
    case 0: mc.bDirect = true;  mc.bIndirect = true;  break;
    case 1: mc.bDirect = true;  mc.bIndirect = false; break;
    case 2: mc.bDirect = false; mc.bIndirect = true;  break;
    default: abort("Both_Direct_Indirect: 0 - sensitive to both, 1 - sensitive only to direct, 2 - sensitive only to indirect"); return;
    }

    if (!position.isEmpty())
    {
        if (position.size() == 2)
        {
            mc.xbins = position.at(0).toInt();
            mc.ybins = position.at(1).toInt();
        }
        else
        {
            abort("Monitor config: Position argument should be either an empty array for default settings or an array of two integers (binsx and binsy)");
            return;
        }
    }

    if (!time.isEmpty())
    {
        if (time.size() == 4)
        {
            mc.timeBins  = time.at(0).toInt();
            mc.timeFrom  = time.at(1).toDouble();
            mc.timeTo    = time.at(2).toDouble();
            mc.timeUnits = time.at(3).toString();

            if (mc.timeUnits != "ns" && mc.timeUnits != "us" && mc.timeUnits != "ms" && mc.timeUnits != "s")
            {
                abort("Monitor config: Valid options for time units are: ns, us, ms, s");
                return;
            }
        }
        else
        {
            abort("Monitor config: Time argument should be either an empty array for default settings or an array of [bins, from, to, units]"
                  "Options for time units: ns, us, ms, s");
            return;
        }
    }

    if (!angle.isEmpty())
    {
        if (angle.size() == 3)
        {
            mc.angleBins = angle.at(0).toInt();
            mc.angleFrom = angle.at(1).toDouble();
            mc.angleTo   = angle.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Angle argument should be either an empty array for default settings or an array of [bins, degreesFrom, degreesTo]");
            return;
        }
    }

    if (!energy.isEmpty())
    {
        if (energy.size() == 4 && energy.at(3).toInt() >= 0 && energy.at(3).toInt() < 4)
        {
            mc.energyBins  = energy.at(0).toInt();
            mc.energyFrom  = energy.at(1).toDouble();
            mc.energyTo    = energy.at(2).toDouble();
            mc.energyUnits = energy.at(3).toString();

            if (mc.energyUnits != "meV" && mc.energyUnits != "eV" && mc.energyUnits != "keV" && mc.energyUnits != "MeV")
            {
                abort("Monitor config: Valid options for energy units are: meV, eV, keV, MeV");
                return;
            }
        }
        else
        {
            abort("Monitor config: Energy argument should be either an empty array for default settings or an array of [bins, from, to, units]\n"
                  "Options for energy units: meV, eV, keV, MeV");
            return;
        }
    }
}

/*
void AGeo_SI::customTGeo(QString name, QString GenerationString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   0,
                                   x,y,z, phi,theta,psi);

    if (GenerationString.simplified().startsWith("TGeoCompositeShape"))
    {
        //qDebug() << "It is a composite!";
        QString s = GenerationString.simplified();
        s.remove("TGeoCompositeShape");
        s.remove("(");
        s.remove(")");
        s.remove("+");
        s.remove("*");
        s.remove("-");
        QStringList members = s.split(" ", Qt::SkipEmptyParts);
        //qDebug() << "Requested logicals:"<<members;

        //create an empty composite object
        AGeometryHub::getInstance().convertObjToComposite(o);
        o->clearCompositeMembers();

        //attempt to add logicals
        for (int iMem=0; iMem<members.size(); iMem++)
        {
            int index = -1;
            for (size_t iObj = 0; iObj < GeoObjects.size(); iObj++)
            {
                if (members[iMem] == GeoObjects[iObj]->Name)
                {
                    index = iObj;
                    break;
                }
            }
            if (index == -1)
            {
                delete o;
                clearGeoObjects();
                abort("Error in composite object generation: logical volume "+members[iMem]+" not found!");
                return;
            }
            //found logical, transferring it to logicals container of the compsoite
            o->getContainerWithLogical()->addObjectLast(GeoObjects[index]);
            GeoObjects.erase(GeoObjects.begin() + index);
        }
        o->refreshShapeCompositeMembers();
    }

    bool ok = o->readShapeFromString(GenerationString);
    if (!ok)
    {
        delete o;
        clearGeoObjects();
        abort(name+": failed to create shape using generation string: "+GenerationString);
        return;
    }
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::customTGeo(QString name, QString generationString, int iMat, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject* o = new AGeoObject(name, container, iMat, nullptr, pos, ori);

    if (generationString.simplified().startsWith("TGeoCompositeShape"))
    {
        //qDebug() << "It is a composite!";
        QString s = generationString.simplified();
        s.remove("TGeoCompositeShape");
        s.remove("(");
        s.remove(")");
        s.remove("+");
        s.remove("*");
        s.remove("-");
        QStringList members = s.split(" ", Qt::SkipEmptyParts);
        //qDebug() << "Requested logicals:"<<members;

        //create an empty composite object
        AGeometryHub::getInstance().convertObjToComposite(o);
        o->clearCompositeMembers();

        //attempt to add logicals
        for (int iMem=0; iMem<members.size(); iMem++)
        {
            int index = -1;
            for (size_t iObj = 0; iObj < GeoObjects.size(); iObj++)
            {
                if (members[iMem] == GeoObjects[iObj]->Name)
                {
                    index = iObj;
                    break;
                }
            }
            if (index == -1)
            {
                delete o;
                clearGeoObjects();
                abort("Error in composite object generation: logical volume "+members[iMem]+" not found!");
                return;
            }
            //found logical, transferring it to logicals container of the compsoite
            o->getContainerWithLogical()->addObjectLast(GeoObjects[index]);
            GeoObjects.erase(GeoObjects.begin() + index);
        }
        o->refreshShapeCompositeMembers();
    }

    ok = o->readShapeFromString(generationString);
    if (!ok)
    {
        delete o;
        clearGeoObjects();
        abort(name+": failed to create shape using generation string: " + generationString);
        return;
    }
    GeoObjects.push_back(o);
}

/*
void AGeo_SI::stack(QString name, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, 0, nullptr, x,y,z, phi,theta,psi);
    delete o->Type;
    o->Type = new ATypeStackContainerObject();
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::stack(QString name, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, 0, new AGeoBox(), pos[0],pos[1],pos[2], ori[0],ori[1],ori[2]);
    delete o->Type; o->Type = new ATypeStackContainerObject();
    GeoObjects.push_back(o);
}

/*
void AGeo_SI::initializeStack(QString StackName, QString MemberName_StackReference)
{
    AGeoObject * StackObj = nullptr;
    for (AGeoObject * obj : GeoObjects)
        if (obj->Name == StackName && obj->Type->isStack())
        {
            StackObj = obj;
            break;
        }

    if (!StackObj)
    {
        abort("Stack with name " + StackName + " not found!");
        return;
    }

    bool bFound = false;
    AGeoObject * origin_obj = nullptr;
    for (size_t io = 0; io < GeoObjects.size(); io++)
    {
        AGeoObject * obj = GeoObjects.at(io);
        if (obj->Name == MemberName_StackReference)
        {
            bFound = true;
            origin_obj = obj;
        }
        if (obj->tmpContName == StackName)
            StackObj->HostedObjects.push_back(obj);
    }

    if (!bFound)
    {
        abort("Stack element with name " + MemberName_StackReference + " not found!");
        return;
    }

    origin_obj->Container = StackObj;
    //static_cast<ATypeStackContainerObject*>(StackObj->Type)->ReferenceVolume = origin_obj->Name;
    origin_obj->Container->updateStack();

    origin_obj->Container = nullptr;
    StackObj->HostedObjects.clear();
}
*/

/*
void AGeo_SI::array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double phi, double theta, double psi, bool centerSymmetric, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeArrayObject * arType = new ATypeArrayObject(numX, numY, numZ, stepX, stepY, stepZ, startIndex);
    arType->bCenterSymmetric = centerSymmetric;
    o->Type = arType;
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::array(QString name, QVariantList numXYZ, QVariantList stepXYZ, QString container, QVariantList position, QVariantList orientation, bool centerSymmetric, int startIndex)
{
    std::array<double,3> pos, ori, num, step;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    if (numXYZ.size() != 3)
    {
        abort("numXYZ should be an array of number of elements along x, y and z directions");
        return;
    }
    if (stepXYZ.size() != 3)
    {
        abort("stepXYZ should be an array of array step along x, y and z directions");
        return;
    }
    for (size_t i = 0; i < 3; i++)
    {
        num[i] = numXYZ[i].toInt();
        step[i] = stepXYZ[i].toDouble();
    }

    AGeoObject * o = new AGeoObject(name, container, 0, 0, pos[0],pos[1],pos[2], ori[0],ori[1],ori[2]);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeArrayObject * arType = new ATypeArrayObject(num[0], num[1], num[2], step[0], step[1], step[2], startIndex);
    arType->bCenterSymmetric = centerSymmetric;
    o->Type = arType;
    GeoObjects.push_back(o);
}

/*
void AGeo_SI::circArray(QString name, int num, double angularStep, double radius, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    o->Type = new ATypeCircularArrayObject(num, angularStep, radius, startIndex);
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::circArray(QString name, int num, double angularStep, double radius, QString container, QVariantList position, QVariantList orientation, int startIndex)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, 0, 0, pos[0],pos[1],pos[2], ori[0],ori[1],ori[2]);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    o->Type = new ATypeCircularArrayObject(num, angularStep, radius, startIndex);
    GeoObjects.push_back(o);
}

/*
void AGeo_SI::hexArray(QString name, int numRings, double pitch, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::Hexagonal, numRings, 1, 1, false, false);
    ar->startIndex = startIndex;
    o->Type = ar;
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::hexArray(QString name, int numRings, double pitch, QString container, QVariantList position, QVariantList orientation, int startIndex)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, 0, 0, pos[0],pos[1],pos[2], ori[0],ori[1],ori[2]);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::Hexagonal, numRings, 1, 1, false, false);
    ar->startIndex = startIndex;
    o->Type = ar;
    GeoObjects.push_back(o);
}

/*
void AGeo_SI::hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipEvenFirst, bool skipOddLast, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::XY, 1, numX, numY, skipEvenFirst, skipOddLast);
    ar->startIndex = startIndex;
    o->Type = ar;
    GeoObjects.push_back(o);
}
*/

void AGeo_SI::hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipEvenFirst, bool skipOddLast, QString container, QVariantList position, QVariantList orientation, int startIndex)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * o = new AGeoObject(name, container, 0, 0, pos[0],pos[1],pos[2], ori[0],ori[1],ori[2]);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::XY, 1, numX, numY, skipEvenFirst, skipOddLast);
    ar->startIndex = startIndex;
    o->Type = ar;
    GeoObjects.push_back(o);
}

void AGeo_SI::prototype(QString name)
{
    AGeoObject * proto = new AGeoObject(name);
    delete proto->Type; proto->Type = new ATypePrototypeObject();
    proto->tmpContName = ProrotypeContainerName;
    GeoObjects.push_back(proto);
}

/*
void AGeo_SI::instance(QString name, QString prototype, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * instance = new AGeoObject(name);
    delete instance->Type; instance->Type = new ATypeInstanceObject(prototype);
    instance->tmpContName = container;
    instance->Position[0] = x;
    instance->Position[1] = y;
    instance->Position[2] = z;
    instance->Orientation[0] = phi;
    instance->Orientation[1] = theta;
    instance->Orientation[2] = psi;
    GeoObjects.push_back(instance);
}
*/

void AGeo_SI::instance(QString name, QString prototype, QString container, QVariantList position, QVariantList orientation)
{
    std::array<double,3> pos, ori;
    bool ok = checkPosOri(position, orientation, pos, ori);
    if (!ok) return;

    AGeoObject * instance = new AGeoObject(name);
    delete instance->Type; instance->Type = new ATypeInstanceObject(prototype);
    instance->tmpContName = container;
    instance->Position[0] = pos[0];
    instance->Position[1] = pos[1];
    instance->Position[2] = pos[2];
    instance->Orientation[0] = ori[0];
    instance->Orientation[1] = ori[1];
    instance->Orientation[2] = ori[2];
    GeoObjects.push_back(instance);
}

void AGeo_SI::setLineProperties(QString name, int color, int width, int style)
{
    AGeoObject * obj = nullptr;

    for (size_t i = 0; i < GeoObjects.size(); i++)
    {
        const QString & GOname = GeoObjects.at(i)->Name;
        if (GOname == name)
        {
            obj = GeoObjects[i];
            break;
        }
    }

    if (!obj)
    {
        //looking through already defined objects in the geometry
        obj = AGeometryHub::getInstance().World->findObjectByName(name);
    }
    if (!obj)
    {
        abort("Cannot find object " + name);
        return;
    }

    //changing line style
    obj->color = color;
    obj->width = width;
    obj->style = style;
}

void AGeo_SI::clearWorld()
{
    clearGeoObjects();

    //AGeometryHub::getInstance().World->recursiveSuicide();  // locked objects are not deleted!
    AGeometryHub::getInstance().clearWorld();

      //Detector->BuildDetector_CallFromScript();
    //AGeometryHub::getInstance().populateGeoManager();
}

void AGeo_SI::clearHosted(QString Object)
{
    AGeoObject* obj = AGeometryHub::getInstance().World->findObjectByName(Object);
    if (!obj)
    {
        abort("Cannot find object "+Object);
        return;
    }
    obj->clearContent();
}

void AGeo_SI::removeWithHosted(QString Object)
{
    AGeoObject* obj = AGeometryHub::getInstance().World->findObjectByName(Object);
    if (!obj)
    {
        abort("Cannot find object "+Object);
        return;
    }
    obj->recursiveSuicide();
}

AGeoObject * AGeo_SI::findObject(const QString & Object)
{
    AGeoObject * obj = nullptr;
    for (AGeoObject * o : GeoObjects)
        if (o->Name == Object)
        {
            obj = o;
            break;
        }

    if (!obj)
    {
        obj = AGeometryHub::getInstance().World->findObjectByName(Object);
        if (!obj)
        {
            abort("Cannot find object " + Object);
            return nullptr;
        }
    }

    return obj;
}

#include "ageospecial.h"
void AGeo_SI::setLightSensor(QString Object, int iModel)
{
    AGeoObject * obj = findObject(Object);
    if (!obj) return;
    delete obj->Role; obj->Role = new AGeoSensor(iModel);
}

void AGeo_SI::setCalorimeter(QString Object, QVariantList bins, QVariantList origin, QVariantList step)
{
    AGeoObject * obj = findObject(Object);
    if (!obj) return;

    if (bins.size() != 3 || origin.size() != 3 || step.size() != 3)
    {
        abort("setCalorimeter parameters should be XYZ arrays (bins, origin and step)");
        return;
    }

    std::array<double, 3> aOrigin, aStep;
    std::array<int, 3>    aBins;
    bool ok1, ok2, ok3;
    for (int i = 0; i < 3; i++)
    {
        aBins[i]   = bins[i]  .toInt(&ok1);
        aOrigin[i] = origin[i].toDouble(&ok2);
        aStep[i]   = step[i]  .toDouble(&ok3);
        if (!ok1 || !ok2 || !ok3)
        {
            abort("Error during convertion to numerics for parameters of the setCalorimeterer method");
            return;
        }
    }

    delete obj->Role; obj->Role = new AGeoCalorimeter(aOrigin, aStep, aBins);
}

void AGeo_SI::setScintillator(QString Object)
{
    AGeoObject * obj = findObject(Object);
    if (!obj) return;
    delete obj->Role; obj->Role = new AGeoScint();
}

void setScintRecursive(AGeoObject * obj, const QString & objectNameStartsWith)
{
    if (obj)
    {
        if (obj->Name.startsWith(objectNameStartsWith, Qt::CaseSensitive))
        {
            delete obj->Role; obj->Role = new AGeoScint();
        }

        for (AGeoObject * hobj : obj->HostedObjects)
            setScintRecursive(hobj, objectNameStartsWith);
    }
}

void AGeo_SI::setScintillatorByName(QString ObjectNameStartsWith)
{
    for (AGeoObject * obj : GeoObjects)
        if (obj->Name.startsWith(ObjectNameStartsWith))
        {
            delete obj->Role; obj->Role = new AGeoScint();
        }

    //std::vector<AGeoObject*> objAr;
    //AGeometryHub::getInstance().World->findObjectsByWildcard(ObjectNameStartsWith, objAr);
    //for (AGeoObject * obj : objAr)
    //{
    //    delete obj->Role; obj->Role = new AGeoScint();
    //}
    setScintRecursive(AGeometryHub::getInstance().World, ObjectNameStartsWith);
}

void AGeo_SI::setSecondaryScintillator(QString Object)
{
    AGeoObject * obj = findObject(Object);
    if (!obj) return;
    delete obj->Role; obj->Role = new AGeoSecScint();
}

#include "asensorhub.h"
int AGeo_SI::countLightSensors()
{
    return ASensorHub::getConstInstance().countSensors();
}

QVariantList AGeo_SI::getLightSensorPositions()
{
    QVariantList vl;
    const ASensorHub & SensHub = ASensorHub::getConstInstance();

    int numSensors = countLightSensors();
    for (int iSens = 0; iSens < numSensors; iSens++)
    {
        QVariantList el;
        AVector3 pos = SensHub.getPositionFast(iSens);
        el << pos[0] << pos[1] << pos[2];
        vl.push_back(el);
    }
    return vl;
}

void AGeo_SI::setPhotonFunctional(QString Object)
{
    AGeoObject * obj = findObject(Object);
    if (!obj) return;
    delete obj->Role; obj->Role = new AGeoPhotonFunctional();
}

#include "aphotonfunctionalhub.h"
#include "aphotonfunctionalmodel.h"
#include <QJsonObject>
QVariantMap AGeo_SI::getDefaultConfigObjectForPhotonFunctionalModel(QString modelName)
{
    APhotonFunctionalModel * model = APhotonFunctionalModel::factory(modelName);
    if ( dynamic_cast<APFM_Dummy*>(model))
    {
        abort("Bad photon functional model type: " + modelName);
        return QVariantMap();
    }

    QJsonObject js;
    model->writeSettingsToJson(js);
    return js.toVariantMap();
}

QVariantMap AGeo_SI::getConfigObjectForPhotonFunctional(int index)
{
    APhotonFunctionalModel * model = APhotonFunctionalHub::getInstance().findModel(index);
    if (!model) return QVariantMap();

    QJsonObject js;
    model->writeSettingsToJson(js);
    QVariantMap res = js.toVariantMap();
    return res;
}

int AGeo_SI::countPhotonFunctionals()
{
    return GeoHub.PhotonFunctionals.size();
}

void AGeo_SI::clearPhotonFunctionalAttribution()
{
    APhotonFunctionalHub::getInstance().clearAllRecords();
}

void AGeo_SI::configurePhotonFunctional(QString modelName, QVariantMap configObject, int index, int linkedIndex)
{
    APhotonFunctionalModel * model = APhotonFunctionalModel::factory(modelName);
    if ( dynamic_cast<APFM_Dummy*>(model))
    {
        abort("Bad photon functional model type: " + modelName);
        return;
    }

    QJsonObject js = QJsonObject::fromVariantMap(configObject);
    qDebug() << configObject << js;
    model->readSettingsFromJson(js);

    QString err = APhotonFunctionalHub::getInstance().modifyOrAddRecord(index, linkedIndex, model);
    if (!err.isEmpty()) abort(err);
}

void AGeo_SI::configurePhotonFunctional(QString modelName, QVariantMap configObject, int index)
{
    configurePhotonFunctional(modelName, configObject, index, index);
}

int AGeo_SI::overrideUnconnectedLinkFunctionals()
{
    return APhotonFunctionalHub::getInstance().overrideUnconnectedLinkFunctionals();
}

void AGeo_SI::setParticleAnalyzer(QString object)
{
    AGeoObject * obj = findObject(object);
    if (!obj) return;
    delete obj->Role; obj->Role = new AGeoParticleAnalyzer();
}

QVariantMap AGeo_SI::getDefaultParticleAnalyzerProperties()
{
    AGeoParticleAnalyzer an;

    QJsonObject js;
    an.Properties.writeToJson(js, false);
    return js.toVariantMap();
}

QVariantMap AGeo_SI::getParticleAnalyzerProperties(QString object)
{
    AGeoObject * paObj = nullptr;

    for (AGeoObject * obj : GeoObjects)
        if (obj->Name == object)
        {
            paObj = obj;
            break;
        }

    if (!paObj)
        paObj = AGeometryHub::getInstance().World->findObjectByName(object);

    if (!paObj)
    {
        abort("Cannot find particle analyzer \"" + object + "\"");
        return QVariantMap();
    }

    if (!paObj->Role)
    {
        abort(object + " is not a particle analyzer!");
        return QVariantMap();
    }
    AGeoParticleAnalyzer * pa = dynamic_cast<AGeoParticleAnalyzer*>(paObj->Role);
    if (!pa)
    {
        abort(object + " is not a particle analyzer!");
        return QVariantMap();
    }

    QJsonObject js;
    pa->Properties.writeToJson(js, false);
    return js.toVariantMap();
}

#include "aerrorhub.h"
void AGeo_SI::configureParticleAnalyzer(QString object, QVariantMap configObject)
{
    AGeoObject * paObj = nullptr;

    for (AGeoObject * obj : GeoObjects)
        if (obj->Name == object)
        {
            paObj = obj;
            break;
        }

    if (!paObj)
        paObj = AGeometryHub::getInstance().World->findObjectByName(object);

    if (!paObj)
    {
        abort("Cannot find particle analyzer \"" + object + "\"");
        return;
    }

    if (!paObj->Role)
    {
        abort(object + " is not a particle analyzer!");
        return;
    }
    AGeoParticleAnalyzer * pa = dynamic_cast<AGeoParticleAnalyzer*>(paObj->Role);
    if (!pa)
    {
        abort(object + " is not a particle analyzer!");
        return;
    }

    QJsonObject js = QJsonObject::fromVariantMap(configObject);
    AErrorHub::clear();
    pa->Properties.readFromJson(js);
    if (AErrorHub::isError())
    {
        abort("Error while attempting to configure particle analyzer " + object + ":\n" + AErrorHub::getQError());
        return;
    }
}

void AGeo_SI::setEnabled(QString ObjectOrWildcard, bool flag)
{
    if (ObjectOrWildcard.endsWith('*'))
    {
        ObjectOrWildcard.chop(1);
        //qDebug() << "Looking for all objects starting with" << ObjectOrWildcard;
        std::vector<AGeoObject*> foundObjs;
        for (AGeoObject * o : GeoObjects)
            if (o->Name.startsWith(ObjectOrWildcard, Qt::CaseSensitive)) foundObjs.push_back(o);
        AGeometryHub::getInstance().World->findObjectsByWildcard(ObjectOrWildcard, foundObjs);

        for (AGeoObject * obj: foundObjs)
            if (!obj->isWorld()) obj->fActive = flag;
    }
    else
    {
        AGeoObject * obj = findObject(ObjectOrWildcard);
        if (!obj) return;
        if (!obj->isWorld()) obj->fActive = flag;
    }
}

#include <QFileInfo>
void AGeo_SI::exportToGDML(QString fileName)
{
    if (QFileInfo(fileName).suffix() != "gdml")
        abort("File suffix should be \"gdml\"");
    else
    {
        //QString err = GeoHub.exportToGDML(FileName);
        QString err = GeoHub.exportGeometry(fileName);
        if (!err.isEmpty()) abort(err);
    }
}

void AGeo_SI::exportToROOT(QString fileName)
{
    if (QFileInfo(fileName).suffix() != "root")
        abort("File suffix should be \"root\"");
    else
    {
        //QString err = GeoHub.exportToROOT(fileName);
        QString err = GeoHub.exportGeometry(fileName);
        if (!err.isEmpty()) abort(err);
    }
}

/*
QString AGeo_SI::getMaterialName(int materialIndex)
{
    QStringList DefMats = Detector->Sandwich->GetMaterials();
    if (materialIndex < 0 || materialIndex >= DefMats.size()) return "Not defined";

    return DefMats[materialIndex];
}

double AGeo_SI::getMaterialDensity(int materialIndex)
{
    const AMaterialParticleCollection & mc = *Detector->Sandwich->MaterialCollection;
    if (materialIndex < 0 || materialIndex >= mc.countMaterials()) return -1.0;

    return mc[materialIndex]->density;
}

QString AGeo_SI::getMaterialComposition(int materialIndex, bool byWeight)
{
    const AMaterialParticleCollection & mc = *Detector->Sandwich->MaterialCollection;
    if (materialIndex < 0 || materialIndex >= mc.countMaterials()) return "Not defined material";

    return ( byWeight ? mc[materialIndex]->ChemicalComposition.getCompositionByWeightString()
                      : mc[materialIndex]->ChemicalComposition.getCompositionString() );
}
*/

void AGeo_SI::updateGeometry(bool CheckOverlaps)
{
    AGeometryHub & GeoHub = AGeometryHub::getInstance();

    //checkup
    for (size_t i = 0; i < GeoObjects.size(); i++)
    {
        const QString & name = GeoObjects[i]->Name;
        if (GeoHub.World->isNameExists(name))
        {
            abort(QString("Name already exists: %1").arg(name));
            clearGeoObjects();
            return;
        }
        for (size_t j = 0; j < GeoObjects.size(); j++)
        {
            if (i == j) continue;
            if (name == GeoObjects[j]->Name)
            {
                abort(QString("At least two objects have the same name: %1").arg(name));
                clearGeoObjects();
                return;
            }
        }

        int imat = GeoObjects[i]->Material;
        if (imat < 0 || imat >= AMaterialHub::getConstInstance().countMaterials())
        {
            abort(QString("Wrong material index for object %1").arg(name));
            clearGeoObjects();
            return;
        }

        const QString & cont = GeoObjects[i]->tmpContName;
        if (cont != ProrotypeContainerName)
        {
            bool fFound = AGeometryHub::getInstance().World->isNameExists(cont);
            if (!fFound)
            {
                //maybe it will be inside one of the GeoObjects defined ABOVE this one?
                for (size_t j = 0; j < i; j++)
                {
                    if (cont == GeoObjects[j]->Name)
                    {
                        fFound = true;
                        break;
                    }
                }
                if (!fFound)
                {
                    abort(QString("Container does not exist: %1").arg(cont));
                    clearGeoObjects();
                    return;
                }
            }
        }
    }

    //adding objects
    for (size_t i = 0; i < GeoObjects.size(); i++)
    {
        AGeoObject * obj = GeoObjects[i];
        const QString & name     = obj->Name;
        const QString & contName = obj->tmpContName;

        if (contName == ProrotypeContainerName)
            //Detector->Sandwich->Prototypes->addObjectLast(obj);
            GeoHub.Prototypes->addObjectLast(obj);
        else
        {
            AGeoObject * contObj = GeoHub.World->findObjectByName(contName);
            if (!contObj)
            {
                abort(QString("Failed to add object %1 to container %2").arg(name).arg(contName));
                clearGeoObjects();
                return;
            }
            contObj->addObjectLast(obj);
        }
        GeoObjects[i] = nullptr;
    }
    clearGeoObjects();

    //Detector->BuildDetector_CallFromScript();
    GeoHub.populateGeoManager(false);

    if (CheckOverlaps)
    {
        //int overlaps = Detector->checkGeoOverlaps();
        int overlaps = GeoHub.checkGeometryForConflicts();
        if (overlaps > 0)
        {
//            emit requestShowCheckUpWindow();
            abort( QString("%0 overlap%1 detected in the geometry!").arg(overlaps).arg(overlaps > 1 ? "s" : ""));
        }
    }
}

void AGeo_SI::clearGeoObjects()
{
    for (AGeoObject * obj : GeoObjects) delete obj;
    GeoObjects.clear();
}

#include "TGeoManager.h"
QVariantList AGeo_SI::trackAndGetPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz)
{
    QVariantList vl;

    if (startXYZ.length() != 3 || startVxVyVz.length() != 3)
    {
        abort("input arguments should be arrays of 3 numbers");
        return vl;
    }

    double r[3], v[3];
    for (int i=0; i<3; i++)
    {
        r[i] = startXYZ[i].toDouble();
        v[i] = startVxVyVz[i].toDouble();
    }

    TGeoNavigator * navigator = AGeometryHub::getInstance().GeoManager->GetCurrentNavigator();
    if (!navigator)
    {
        qDebug() << "Tracking: Current navigator does not exist, creating new";
        navigator = AGeometryHub::getInstance().GeoManager->AddNavigator();
    }

    navigator->SetCurrentPoint(r);
    navigator->SetCurrentDirection(v);
    navigator->FindNode();

    if (navigator->IsOutside())
    {
        abort("The starting point is outside the defined geometry");
        return vl;
    }

    do
    {
        QVariantList el;

        for (int i=0; i<3; i++)
            el << navigator->GetCurrentPoint()[i];
        TGeoNode * node = navigator->GetCurrentNode();
        TGeoVolume * vol = node->GetVolume();
        el << vol->GetMaterial()->GetIndex();
        el << vol->GetName();
        el << node->GetNumber();

        vl.push_back(el);

        navigator->FindNextBoundaryAndStep();
    }
    while (!navigator->IsOutside());

    return vl;
}

QVariantList AGeo_SI::getScintillatorProperties()
{
    std::vector<QString> name;
    GeoHub.getScintillatorVolumeNames(name);
    std::vector<AVector3> pos;
    GeoHub.getScintillatorPositions(pos);
    std::vector<AVector3> ori;
    GeoHub.getScintillatorOrientations(ori);

    QVariantList vl;
    for (int iScint = 0; iScint < (int)pos.size(); iScint++)
    {
        QVariantList rec;

        rec.push_back(iScint);
        rec.push_back(name[iScint]);
        QVariantList vlPos;
            for (size_t i = 0; i < 3; i++) vlPos << pos[iScint][i];
        rec.push_back(vlPos);
        QVariantList vlOri;
            for (size_t i = 0; i < 3; i++) vlOri << ori[iScint][i];
        rec.push_back(vlOri);

        vl.push_back(rec);
    }
    return vl;
}
