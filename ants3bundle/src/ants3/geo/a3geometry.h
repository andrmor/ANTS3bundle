#ifndef A3GEOMETRY_H
#define A3GEOMETRY_H

#include <QStringList>

#include <vector>

class AGeoObject;
class TGeoManager;
class TGeoVolume;
class A3MatHub;
class AGridElementRecord;
class TGeoNode;
class TGeoCombiTrans;
class TGeoRotation;
class QJsonObject;

class A3Geometry
{
public:
    static A3Geometry & getInstance();

private:
    A3Geometry();
    ~A3Geometry();

    A3Geometry(const A3Geometry&)            = delete;
    A3Geometry(A3Geometry&&)                 = delete;
    A3Geometry& operator=(const A3Geometry&) = delete;
    A3Geometry& operator=(A3Geometry&&)      = delete;

public:
    AGeoObject  * World      = nullptr;  // world tree structure
    AGeoObject  * Prototypes = nullptr;  // container with prorotypes. It is hosted by the World

    TGeoManager * GeoManager = nullptr;
    TGeoVolume  * Top        = nullptr;  // world in TGeoManager

    QString ErrorString;

    std::vector<const AGeoObject*> MonitorsRecords;
    std::vector<QString> MonitorIdNames;  //runtime
    std::vector<TGeoNode*> MonitorNodes; //runtime

    std::vector<AGridElementRecord*> GridRecords;

    //properties used during the call of populateGeoManager()
    const A3MatHub * MaterialCollection = nullptr; // TODO: will be a singleton!

    void         populateGeoManager();

    void         writeToJson(QJsonObject & json) const;
    bool         readFromJson(const QJsonObject & json);  // returns "" if no errors, else error description

    void         clearWorld();
    bool         canBeDeleted(AGeoObject * obj) const;

    void         convertObjToComposite(AGeoObject * obj);

    QString      convertToNewPrototype(std::vector<AGeoObject*> members);
    bool         isValidPrototypeName(const QString & ProtoName) const;

    void         onStart();
    void         aboutToQuit();

    //grids
/*
    void         convertObjToGrid(AGeoObject * obj);
    void         shapeGrid(AGeoObject * obj, int shape, double p0, double p1, double p2, int wireMat);
    //parallel - 0, pitch, length, wireDiameter
    //mesh - 1, pitchX, pitchY, wireDiameter
    //hexa - 2, outer circle diameter, inner circle diameter, full height
*/

    bool         isMaterialInUse(int imat) const;
    void         deleteMaterial(int imat);

    bool         isVolumeExistAndActive(const QString & name) const;

    void         colorVolumes(int scheme, int id = 0);
    void         changeLineWidthOfVolumes(int delta);

    //World size-related
    bool         isWorldSizeFixed() const;
    void         setWorldSizeFixed(bool bFlag);
    double       getWorldSizeXY() const;
    void         setWorldSizeXY(double size);
    double       getWorldSizeZ() const;
    void         setWorldSizeZ(double size);

private:
    void addTGeoVolumeRecursively(AGeoObject * obj, TGeoVolume * parent, int forcedNodeNumber = 0);

    void positionArray(AGeoObject * obj, TGeoVolume * vol);
    void positionStack(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber);
    void positionInstance(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber);

    void positionArrayElement(int ix, int iy, int iz, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex);
    void positionCircularArrayElement(int ia, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex);
    void positionStackElement(AGeoObject * el, const AGeoObject * RefObj, TGeoVolume * parent, int forcedNodeNumber);

    void expandPrototypeInstances();
    bool processCompositeObject(AGeoObject *obj);
    void addMonitorNode(AGeoObject *obj, TGeoVolume *vol, TGeoVolume *parent, TGeoCombiTrans *lTrans);
    TGeoRotation * createCombinedRotation(TGeoRotation * firstRot, TGeoRotation * secondRot, TGeoRotation * thirdRot = nullptr);

    void clearGridRecords();
    void clearMonitorRecords();
};

#endif // A3GEOMETRY_H
