#ifndef AGEOMETRYHUB_H
#define AGEOMETRYHUB_H

#include "avector.h"

#include <QString>

#include <vector>
#include <tuple>
#include <string>

class AGeoObject;
class TGeoManager;
class TGeoVolume;
class TGeoNode;
class TGeoCombiTrans;
class TGeoRotation;
class QJsonObject;
class AVector3;
class QStringLists;
class AGeoShape;
class AParticleAnalyzerSettings;

#include "TString.h"

class AGeometryHub
{
public:
    static AGeometryHub & getInstance();
    static const AGeometryHub & getConstInstance();

private:
    AGeometryHub();
    ~AGeometryHub();

    AGeometryHub(const AGeometryHub&)            = delete;
    AGeometryHub(AGeometryHub&&)                 = delete;
    AGeometryHub& operator=(const AGeometryHub&) = delete;
    AGeometryHub& operator=(AGeometryHub&&)      = delete;

public:
    AGeoObject  * World      = nullptr;  // world tree structure
    AGeoObject  * Prototypes = nullptr;  // container with prorotypes. It is hosted by the World

    TGeoManager * GeoManager = nullptr;
    TGeoVolume  * Top        = nullptr;  // world in TGeoManager

    const TString IndexSeparator = "_-_";

    void         populateGeoManager(bool notifyRootServer = true);
    void         notifyRootServerGeometryChanged();

    void         writeToJson(QJsonObject & json) const;
    QString      readFromJson(const QJsonObject & json);

    void         clearWorld();
    bool         canBeDeleted(AGeoObject * obj) const;

    void         convertObjToComposite(AGeoObject * obj) const;

    QString      convertToNewPrototype(std::vector<AGeoObject*> members);
    bool         isValidPrototypeName(const QString & ProtoName) const;

    void         aboutToQuit();

    bool         isMaterialInUse(int imat, QString & volName) const;
    void         onMaterialRemoved(int imat);  // !!!*** add signal

    bool         isVolumeExistAndActive(const QString & name) const;

    void         colorVolumes(int scheme, int id = 0);  // !!!*** to geometry window? consider jsroot  !!!*** redo, can be very slow
    void         changeLineWidthOfVolumes(int delta);

    bool         isWorldSizeFixed() const;
    void         setWorldSizeFixed(bool bFlag);
    double       getWorldSizeXY() const;
    void         setWorldSizeXY(double size);
    double       getWorldSizeZ() const;
    void         setWorldSizeZ(double size);

    int          checkGeometryForConflicts();
    QString      checkVolumesExist(const std::vector<std::string> & VolumesAndWildcards) const;

    //QString      exportToGDML(const QString & fileName) const;  // old system based on mm->cm replacing in the GDML file
    //QString      exportToROOT(const QString & fileName) const;  // old system
    QString      exportGeometry(const QString & fileName);      // new system based on geometry scaling

    QString      importGDML(const QString & fileName);          // old system based on mm->cm replacing in the GDML file
    QString      importGeometry(const QString & fileName);      // new system based on geometry scaling

    QString      generateStandaloneObjectName(const AGeoShape * shape) const;
    QString      generateObjectName(const QString & prefix) const;

    void         removeNameDecorators(TString & name) const;

    size_t       countScintillators() const;
    void         getScintillatorPositions(std::vector<AVector3> & positions) const;
    void         getScintillatorOrientations(std::vector<AVector3> & orientations) const;
    void         getScintillatorVolumeNames(std::vector<QString> & vol) const;
    void         getScintillatorVolumeUniqueNames(std::vector<QString> & vol) const;

    void         checkGeometryCompatibleWithGeant4() const;

    size_t       countParticleAnalyzers() const;
    void         fillParticleAnalyzerRecords(AParticleAnalyzerSettings * settings) const;

private:
    void addTGeoVolumeRecursively(AGeoObject * obj, TGeoVolume * parent, int forcedNodeNumber = 0);

    void positionArray(AGeoObject * obj, TGeoVolume * vol, int parentNodeIndex);  // !!!*** split to array types!
    void positionStack(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber);
    void positionInstance(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber);

    void positionArrayElement(int ix, int iy, int iz, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex);
    void positionCircularArrayElement(int ia, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex);
    void positionHexArrayRing(int iR, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int & arrayIndex);
    void positionHexArrayElement(double localX, double localY, AGeoObject *el, AGeoObject *arrayObj, TGeoVolume *parent, int arrayIndex);
    void positionStackElement(AGeoObject * el, const AGeoObject * RefObj, TGeoVolume * parent, int forcedNodeNumber);

    void expandPrototypeInstances();
    bool processCompositeObject(AGeoObject *obj);
    void addMonitorNode(AGeoObject *obj, TGeoVolume *vol, TGeoVolume *parent, TGeoCombiTrans *lTrans);
    void addCalorimeterNode(AGeoObject *obj, TGeoVolume *vol, TGeoVolume *parent, TGeoCombiTrans *lTrans);
    void addSensorNode(AGeoObject *obj, TGeoVolume *vol, TGeoVolume *parent, TGeoCombiTrans *lTrans);
    TGeoRotation * createCombinedRotation(TGeoRotation * firstRot, TGeoRotation * secondRot, TGeoRotation * thirdRot = nullptr);

    void clearMonitors();
    void getGlobalPosition(const TGeoNode * node, AVector3 & position) const;
    void getGlobalUnitVectors(const TGeoNode * node, double * uvX, double * uvY, double * uvZ) const;
    void findMotherNode(const TGeoNode * node, const TGeoNode* & motherNode) const;
    bool findMotherNodeFor(const TGeoNode * node, const TGeoNode * startNode, const TGeoNode* & foundNode) const;
    void setVolumeTitle(AGeoObject * obj, TGeoVolume * vol);
    QString readGDMLtoTGeo(const QString & fileName);

    void registerPhotonFunctional(AGeoObject * obj, TGeoVolume * parentVol);

private:
    bool   DoScaling = false;
    double ScalingFactor = 1.0;

    std::vector<std::pair<AGeoObject*,TGeoNode*>> Scintillators;
public:
    std::vector<std::tuple<AGeoObject*,TGeoNode*,AVector3>> ParticleAnalyzers;  // last is global position
    std::vector<std::tuple<AGeoObject*,TGeoNode*,AVector3>> PhotonFunctionals;  // last is global position
};

#endif // AGEOMETRYHUB_H
