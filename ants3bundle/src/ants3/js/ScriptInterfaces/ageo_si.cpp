#include "ageo_si.h"
#include "ageometryhub.h"
#include "amaterialhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "afiletools.h"

#include <QDebug>

AGeo_SI::AGeo_SI()
{
    Description = "Allows to configure detector geometry. Based on CERN ROOT TGeoManager";

    QString s = "It is filled with the given material (material index iMat)\n"
                "and positioned inside 'container' object\n"
                "at coordinates (x,y,z) and orientation angles (phi,thetha,psi)\n"
                "in the local frame of the container;\n"
                "Requires updateGeometry() to take effect!";

    Help["box"] = "Adds to geometry a box 'name' with the sizes (sizeX,sizeY,sizeZ)\n." + s;
    Help["cylinder"] = "Adds to geometry a cylinder 'name' with the given diameter and height\n" + s;
    Help["tube"] = "Adds to geometry a tube 'name' with the given outer and inner diameters and height\n" + s;
    Help["cone"] =  "Adds to geometry a cone 'name' with the given top and bottom diameters and height\n" +s;
    Help["polygone"] =  "Adds to geometry a polygon 'name' with the given number of edges, top and bottom diameters of the inscribed "
                        "circles and height.\n" +s;
    Help["dphere"] = "Adds to geometry a sphere 'name' with the given diameter.\n" + s;
    Help["srb8"] = "Adds to geometry a TGeoArb8 object with name 'name' and the given height and two arrays,\n"
                   "containing X and Y coordinates of the nodes.\n"+ s;
    Help["customTGeo"] = "Adds to geometry an object with name 'name' and the shape generated using the CERN ROOT geometry system.\n" + s +
                   "\nFor example, to generate a box of (10,10,10) half sizes use the generation string \"TGeoBBox(10, 10, 10)\".\n"
                   "To generate a composite object, first create logical volumes (using TGeo command or \"Box\" etc), and then "
                   "create the composite using, e.g., the generation string \"TGeoCompositeShape( name1 + name2 )\". Note that the logical volume is removed "
                   "from the generation list after it was used by composite object generator!";

    Help["setLineProperties"] = "Sets color, width and style of the line for visualisation of the object \"name\".";
    Help["clearWorld"] = "Removes all objects and prototypes leaving only World";

    Help["clearHosted"] = "Removes all objects hosted inside the given Object.\nRequires updateGeometry().";
    Help["removeWithHosted"] = "Removes the Object and all objects hosted inside.\nRequires updateGeometry().";

    Help["updateGeometry"] = "Updates geometry and optionally check it for errors.\n";

    Help["stack"] = "Adds empty stack object. Volumes can be added normally to this object, stating its name as the container.\n"
                    "After the last element is added, call InitializeStack(StackName, Origin) function. "
                    "It will automatically calculate x,y and z positions of all elements, keeping user-configured xyz position of the Origin element.";
    Help["initializeStack"] = "Call this function after the last element has been added to the stack."
                              "It will automatically calculate x,y and z positions of all elements, keeping user-configured xyz position of the Origin element.";

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

void AGeo_SI::box(QString name, double Lx, double Ly, double Lz, int iMat, QString container,
                  double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoBox(0.5*Lx, 0.5*Ly, 0.5*Lz),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::cylinder(QString name, double D, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoTube(0.5*D, 0.5*h),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    if (innerD >= outerD)
    {
        abort("Inner diameter of a Tube should be smaller than the outer one");
        return;
    }
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                   new AGeoTube(0.5*innerD, 0.5*outerD, 0.5*h),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::polygone(QString name, int edges, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, iMat,
                                    new AGeoPolygon(edges, 0.5*h, 0.5*Dbot, 0.5*Dtop),
                                    x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoCone(0.5*h, 0.5*Dbot, 0.5*Dtop),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoCone(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::coneSegment(QString name, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoConeSeg(0.5*h, 0.5*DbotIn, 0.5*DbotOut, 0.5*DtopIn, 0.5*DtopOut, phi1, phi2),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::sphere(QString name, double D, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoSphere(0.5*D),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::sphereLayer(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoSphere(0.5*Dout, 0.5*Din),
                                   x,y,z, phi,theta,psi);
    GeoObjects.push_back(o);
}

void AGeo_SI::arb8(QString name, QVariant NodesX, QVariant NodesY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    //qDebug() << NodesX << NodesY;
    QStringList lx = NodesX.toStringList();
    QStringList ly = NodesY.toStringList();
    //qDebug() << lx;
    //qDebug() << ly;
    if (lx.size()!=8 || ly.size()!=8)
    {
        clearGeoObjects();
        abort("Node arrays should contain 8 points each");
        return;
    }

    QList<QPair<double, double> > V;
    bool ok1, ok2;
    for (int i=0; i<8; i++)
    {
        double x = lx[i].toDouble(&ok1);
        double y = ly[i].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            clearGeoObjects();
            abort("Arb8 node array - conversion to double error");
            return;
        }
        V.push_back( QPair<double,double>(x,y) );
    }

    if (!AGeoArb8::checkPointsForArb8(V))
    {
        clearGeoObjects();
        abort("Arb8 nodes should be define clockwise for both planes");
        return;
    }

    AGeoObject* o = new AGeoObject(name, container, iMat,
                                   new AGeoArb8(0.5*h, V),
                                   x,y,z, phi,theta,psi);
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

void AGeo_SI::monitor(QString name, int shape, double size1, double size2, QString container, double x, double y, double z, double phi, double theta, double psi, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking)
{
    AGeoObject * o = new AGeoObject(name, container, 0,    // no material -> it will be updated on build
                                    0,                     // no shape yet
                                    x,y,z, phi,theta,psi);

    ATypeMonitorObject* mto = new ATypeMonitorObject();
    delete o->Type; o->Type = mto;

    AMonitorConfig& mc = mto->config;
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

void AGeo_SI::configurePhotonMonitor(QString MonitorName, QVariant Position, QVariant Time, QVariant Angle, QVariant Wave)
{
    AGeoObject* o = nullptr;
    for (AGeoObject* obj : GeoObjects)
    {
        if (obj->Name == MonitorName)
        {
            o = obj;
            break;
        }
    }

    if (!o)
    {
        abort("Cannot find monitor \"" + MonitorName + "\"");
        return;
    }

    if (!o->Type || !o->Type->isMonitor())
    {
        abort(MonitorName + " is not a monitor object!");
        return;
    }

    ATypeMonitorObject* m = static_cast<ATypeMonitorObject*>(o->Type);
    AMonitorConfig& mc = m->config;

    mc.PhotonOrParticle = 0;

    QVariantList pos = Position.toList();
    if (!pos.isEmpty())
    {
        if (pos.size() == 2)
        {
            mc.xbins = pos.at(0).toInt();
            mc.ybins = pos.at(1).toInt();
        }
        else
        {
            abort("Monitor config: Position argument should be either an empty array for default settings or an array of two integers (binsx and binsy)");
            return;
        }
    }

    QVariantList time = Time.toList();
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

    QVariantList a = Angle.toList();
    if (!a.isEmpty())
    {
        if (a.size() == 3)
        {
            mc.angleBins = a.at(0).toInt();
            mc.angleFrom = a.at(1).toDouble();
            mc.angleTo   = a.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Angle argument should be either an empty array for default settings or an array of [bins, degreesFrom, degreesTo]");
            return;
        }
    }

    QVariantList w = Wave.toList();
    if (!w.isEmpty())
    {
        if (w.size() == 3)
        {
            mc.waveBins = w.at(0).toInt();
            mc.waveFrom = w.at(1).toDouble();
            mc.waveTo   = w.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Wave argument should be either an empty array for default settings or an array of [bins, from, to]");
            return;
        }
    }
}

void AGeo_SI::configureParticleMonitor(QString MonitorName, QString Particle, int Both_Primary_Secondary, int Both_Direct_Indirect,
                                            QVariant Position, QVariant Time, QVariant Angle, QVariant Energy)
{
    AGeoObject* o = 0;
    for (AGeoObject* obj : GeoObjects)
        if (obj->Name == MonitorName)
        {
            o = obj;
            break;
        }

    if (!o)
    {
        abort("Cannot find monitor \"" + MonitorName + "\"");
        return;
    }

    if (!o->Type || !o->Type->isMonitor())
    {
        abort(MonitorName + " is not a monitor object!");
        return;
    }

    ATypeMonitorObject* m = static_cast<ATypeMonitorObject*>(o->Type);
    AMonitorConfig& mc = m->config;

    mc.PhotonOrParticle = 1;
    mc.Particle = Particle;

    switch (Both_Primary_Secondary)
    {
    case 0: mc.bPrimary = true;  mc.bSecondary = true;  break;
    case 1: mc.bPrimary = true;  mc.bSecondary = false; break;
    case 2: mc.bPrimary = false; mc.bSecondary = true;  break;
    default: abort("Both_Primary_Secondary: 0 - sensitive to both, 1 - sensetive only to primary, 2 - sensitive only to secondary"); return;
    }
    switch (Both_Direct_Indirect)
    {
    case 0: mc.bDirect = true;  mc.bIndirect = true;  break;
    case 1: mc.bDirect = true;  mc.bIndirect = false; break;
    case 2: mc.bDirect = false; mc.bIndirect = true;  break;
    default: abort("Both_Direct_Indirect: 0 - sensitive to both, 1 - sensitive only to direct, 2 - sensitive only to indirect"); return;
    }

    QVariantList pos = Position.toList();
    if (!pos.isEmpty())
    {
        if (pos.size() == 2)
        {
            mc.xbins = pos.at(0).toInt();
            mc.ybins = pos.at(1).toInt();
        }
        else
        {
            abort("Monitor config: Position argument should be either an empty array for default settings or an array of two integers (binsx and binsy)");
            return;
        }
    }

    QVariantList time = Time.toList();
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

    QVariantList a = Angle.toList();
    if (!a.isEmpty())
    {
        if (a.size() == 3)
        {
            mc.angleBins = a.at(0).toInt();
            mc.angleFrom = a.at(1).toDouble();
            mc.angleTo   = a.at(2).toDouble();
        }
        else
        {
            abort("Monitor config: Angle argument should be either an empty array for default settings or an array of [bins, degreesFrom, degreesTo]");
            return;
        }
    }

    QVariantList e = Energy.toList();
    if (!e.isEmpty())
    {
        if (e.size() == 4 && e.at(3).toInt() >= 0 && e.at(3).toInt() < 4)
        {
            mc.energyBins = e.at(0).toInt();
            mc.energyFrom = e.at(1).toDouble();
            mc.energyTo   = e.at(2).toDouble();
            mc.energyUnitsInHist = e.at(3).toInt();
        }
        else
        {
            abort("Monitor config: Energy argument should be either an empty array for default settings or an array of [bins, from, to, units]\n"
                  "Energy units: 0,1,2,3 -> meV, eV, keV, MeV;");
            return;
        }
    }
}

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
            for (int iObj=0; iObj<GeoObjects.size(); iObj++)
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

void AGeo_SI::stack(QString name, QString container, double x, double y, double z, double phi, double theta, double psi)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Type;
    o->Type = new ATypeStackContainerObject();
    GeoObjects.push_back(o);
}

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
    AGeoObject* origin_obj = nullptr;
    for (int io=0; io<GeoObjects.size(); io++)
    {
        AGeoObject* obj = GeoObjects.at(io);
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
    static_cast<ATypeStackContainerObject*>(StackObj->Type)->ReferenceVolume = origin_obj->Name;
    origin_obj->updateStack();

    origin_obj->Container = nullptr;
    StackObj->HostedObjects.clear();
}

void AGeo_SI::array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    o->Type = new ATypeArrayObject(numX, numY, numZ, stepX, stepY, stepZ, startIndex);
    GeoObjects.push_back(o);
}

void AGeo_SI::circArray(QString name, int num, double angularStep, double radius, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    o->Type = new ATypeCircularArrayObject(num, angularStep, radius, startIndex);
    GeoObjects.push_back(o);
}

void AGeo_SI::hexArray(QString name, int numRings, double pitch, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::Hexagonal, numRings, 1, 1, false);
    o->Type = ar;
    GeoObjects.push_back(o);
}

void AGeo_SI::hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipLast, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex)
{
    AGeoObject * o = new AGeoObject(name, container, 0, 0, x,y,z, phi,theta,psi);
    delete o->Shape; o->Shape = new AGeoBox;
    delete o->Type;
    ATypeHexagonalArrayObject * ar = new ATypeHexagonalArrayObject();
    ar->reconfigure(pitch, ATypeHexagonalArrayObject::XY, 1, numX, numY, skipLast);
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

void AGeo_SI::setLineProperties(QString name, int color, int width, int style)
{
    AGeoObject * obj = nullptr;

    for (int i = 0; i < GeoObjects.size(); i++)
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

    AGeometryHub::getInstance().World->recursiveSuicide();  // locked objects are not deleted!

    //Detector->BuildDetector_CallFromScript();
    AGeometryHub::getInstance().populateGeoManager();
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

#include "ageospecial.h"
void AGeo_SI::setLightSensor(QString Object)
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
            return;
        }
    }

    delete obj->Role; obj->Role = new AGeoSensor(0);
}

void AGeo_SI::setEnabled(QString ObjectOrWildcard, bool flag)
{
    if (ObjectOrWildcard.endsWith('*'))
    {
        ObjectOrWildcard.chop(1);
        //qDebug() << "Looking for all objects starting with" << ObjectOrWildcard;
        QVector<AGeoObject*> foundObjs;
        AGeometryHub::getInstance().World->findObjectsByWildcard(ObjectOrWildcard, foundObjs);

        for (AGeoObject * obj: foundObjs)
            if (!obj->isWorld())
                obj->fActive = flag;
    }
    else
    {
        AGeoObject* obj = AGeometryHub::getInstance().World->findObjectByName(ObjectOrWildcard);
        if (!obj)
            abort("Cannot find object " + ObjectOrWildcard);
        else
        {
            if (!obj->isWorld())
                obj->fActive = flag;
        }
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
    for (int i = 0; i < GeoObjects.size(); i++)
    {
        const QString & name = GeoObjects.at(i)->Name;
        if (GeoHub.World->isNameExists(name))
        {
            abort(QString("Name already exists: %1").arg(name));
            clearGeoObjects();
            return;
        }
        for (int j = 0; j < GeoObjects.size(); j++)
        {
            if (i == j) continue;
            if (name == GeoObjects.at(j)->Name)
            {
                abort(QString("At least two objects have the same name: %1").arg(name));
                clearGeoObjects();
                return;
            }
        }

        int imat = GeoObjects.at(i)->Material;
        if (imat < 0 || imat >= AMaterialHub::getConstInstance().countMaterials())
        {
            abort(QString("Wrong material index for object %1").arg(name));
            clearGeoObjects();
            return;
        }

        const QString & cont = GeoObjects.at(i)->tmpContName;
        if (cont != ProrotypeContainerName)
        {
            bool fFound = AGeometryHub::getInstance().World->isNameExists(cont);
            if (!fFound)
            {
                //maybe it will be inside one of the GeoObjects defined ABOVE this one?
                for (int j = 0; j < i; j++)
                {
                    if (cont == GeoObjects.at(j)->Name)
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
    for (int i = 0; i < GeoObjects.size(); i++)
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
    GeoHub.populateGeoManager();

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

/*
#include "aopticaloverride.h"
QString AGeo_SI::printOverrides()
{
    QString s("");
    int nmat = Detector->MpCollection->countMaterials();
    s += QString("%1\n").arg(nmat);

    for (int i=0; i<nmat; i++) {
        QVector<AOpticalOverride*> VecOvr = (*(Detector->MpCollection))[i]->OpticalOverrides;
        if (VecOvr.size() > 0)
            for (int j=0; j<VecOvr.size(); j++) {
                if (VecOvr[j]) {
                    s = s + QString("%1 ").arg(i) + QString("%1 ").arg(j);
                    //                  s = s + VecOvr[j]->getType() + " "
                    s = s + VecOvr[j]->getReportLine() + QString("\n");
                }
            }
    }
    return s;
}
*/

#include "TGeoManager.h"
QVariantList AGeo_SI::getPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz)
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
