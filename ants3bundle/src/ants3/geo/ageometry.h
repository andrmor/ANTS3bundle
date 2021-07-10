#ifndef AGEOMETRY_H
#define AGEOMETRY_H

#include <QStringList>

#include <vector>

class AGeoObject;
class TGeoManager;
class TGeoVolume;
class AMaterialParticleCollection;
class AGridElementRecord;
class TGeoNode;
class TGeoCombiTrans;
class TGeoRotation;
class QJsonObject;

class AGeometry
{
public:
    AGeometry();
    ~AGeometry();

    AGeoObject * World      = nullptr;  // world tree structure, slabs are on the first level!
    AGeoObject * Prototypes = nullptr;  // hosts prototypes; a prototype is an objects (can be a tree of objects) which can be placed multiple times in the geometry
    // Prorotypes object is hosted by World!

    void         clearWorld();
    bool         canBeDeleted(AGeoObject * obj) const;

    //composite
    void         convertObjToComposite(AGeoObject * obj);

    //ptototype/instances
    QString      convertToNewPrototype(std::vector<AGeoObject*> members);
    bool         isValidPrototypeName(const QString & ProtoName) const;

    //grid
/*
    void         convertObjToGrid(AGeoObject * obj);
    void         shapeGrid(AGeoObject * obj, int shape, double p0, double p1, double p2, int wireMat);
    //parallel - 0, pitch, length, wireDiameter
    //mesh - 1, pitchX, pitchY, wireDiameter
    //hexa - 2, outer circle diameter, inner circle diameter, full height
*/

    void         populateGeoManager();

    bool         isMaterialInUse(int imat) const;
    void         DeleteMaterial(int imat);
    bool         isVolumeExistAndActive(const QString & name) const;
    void         changeLineWidthOfVolumes(int delta);

    // JSON
    void         writeToJson(QJsonObject & json) const;
    QString      readFromJson(const QJsonObject & json);  // returns "" if no errors, else error description

    //World size-related
    bool         isWorldSizeFixed() const;
    void         setWorldSizeFixed(bool bFlag);
    double       getWorldSizeXY() const;
    void         setWorldSizeXY(double size);
    double       getWorldSizeZ() const;
    void         setWorldSizeZ(double size);

    std::vector<AGridElementRecord*> GridRecords;

    std::vector<const AGeoObject*> MonitorsRecords;
    std::vector<QString> MonitorIdNames;  //runtime
    std::vector<TGeoNode*> MonitorNodes; //runtime

    QString LastError;

    TGeoManager * GeoManager = nullptr;
    TGeoVolume  * Top        = nullptr; // world in TGeoManager

    //properties used during the call of populateGeoManager()
    const AMaterialParticleCollection * MaterialCollection = nullptr;

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

#endif // AGEOMETRY_H
