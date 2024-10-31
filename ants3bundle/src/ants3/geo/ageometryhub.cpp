#include "ageometryhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
#include "ageoconsts.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ageospecial.h"
#include "ajsontools.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "agridhub.h"
#include "acalorimeterhub.h"
#include "aerrorhub.h"
#include "afiletools.h"

#include <QDebug>
#include <QFileInfo>

#include "TGeoManager.h"
#include "TVector3.h"

AGeometryHub & AGeometryHub::getInstance()
{
    static AGeometryHub instance;
    return instance;
}

const AGeometryHub &AGeometryHub::getConstInstance()
{
    return getInstance();
}

AGeometryHub::AGeometryHub()
{
    World = new AGeoObject("World");
    delete World->Type; World->Type = new ATypeWorldObject();
    World->Material = 0;
    World->Container = nullptr;
    World->color = 1;

    Prototypes = new AGeoObject("_#_PrototypeContainer_#_");
    delete Prototypes->Type; Prototypes->Type = new ATypePrototypeCollectionObject();
    Prototypes->migrateTo(World);
}

AGeometryHub::~AGeometryHub()
{
    //qDebug() << "Dest for A3Geometry";
    clearWorld();
    delete World; World = nullptr;

    delete GeoManager; // should be deleted by aboutToQuit()!
}

void AGeometryHub::clearWorld()
{
    //delete all but World and Prototypes.
    //the Prototypes object should have the same pointer

    World->removeHostedObject(Prototypes);

    for (AGeoObject * obj : World->HostedObjects) obj->clearAll();
    World->HostedObjects.clear();

    World->Material = 0;
    setWorldSizeFixed(false);

    Prototypes->clearContent();
    World->HostedObjects.push_back(Prototypes);

    setWorldSizeFixed(false);
    setWorldSizeXY(500);
    setWorldSizeZ(500);

    clearMonitors();
    ACalorimeterHub::getInstance().clear();
    AGridHub::getInstance().clear();

    Scintillators.clear();
    PhotonFunctionals.clear();
    ParticleAnalyzers.clear();
}

void AGeometryHub::clearMonitors()
{
    AMonitorHub & mh = AMonitorHub::getInstance();
    mh.clear(AMonitorHub::Photon);
    mh.clear(AMonitorHub::Particle);
}

bool AGeometryHub::canBeDeleted(AGeoObject * obj) const
{
    if (obj == World) return false;
    if (obj == Prototypes) return false;

    if (obj->isInUseByComposite()) return false;
    if (obj->Type->isPrototype() && World->isPrototypeInUseRecursive(obj->Name, nullptr)) return false;

    return true;
}

void AGeometryHub::convertObjToComposite(AGeoObject * obj) const
{
    delete obj->Type;
    ATypeCompositeObject* CO = new ATypeCompositeObject();
    obj->Type = CO;

    AGeoObject* logicals = new AGeoObject();
    logicals->Name = "Logicals_"+obj->Name;
    delete logicals->Type;
    logicals->Type = new ATypeCompositeContainerObject();
    obj->addObjectFirst(logicals);

    const QString firstName = generateStandaloneObjectName(obj->Shape);
    AGeoObject * first = new AGeoObject(firstName, obj->Shape);
    first->Material = obj->Material;
    logicals->addObjectLast(first);

    AGeoBox * boxShape = new AGeoBox();
    const QString secondName = generateStandaloneObjectName(boxShape);
    AGeoObject * second = new AGeoObject(secondName, boxShape);
    second->Material = obj->Material;
    second->Position[0] = 15.0;
    second->Position[1] = 15.0;
    logicals->addObjectLast(second);

    QStringList sl;
    sl << first->Name << second->Name;
    const QString str = "TGeoCompositeShape( " + first->Name + " + " + second->Name + " )";
    obj->Shape = new AGeoComposite(sl, str);
}

void AGeometryHub::removePresentInContainers(std::vector<AGeoObject *> & members)
{
    // assume unordered selection
    size_t iCheckingObject = members.size();
    while (iCheckingObject > 0)
    {
        iCheckingObject--;
        AGeoObject * objChecking = members[iCheckingObject];
        bool isInContainer = false;
        for (AGeoObject * obj : members)
        {
            if (obj == objChecking) continue;
            if (obj->isContainsObjectRecursive(objChecking))
            {
                isInContainer = true;
                break;
            }
        }

        if (isInContainer) members.erase(members.begin() + iCheckingObject);
    }
}

QString AGeometryHub::convertToNewPrototype(std::vector<AGeoObject *> & members)
{
    QString errStr;

    for (AGeoObject * obj : members)
    {
        bool ok = obj->isPossiblePrototype(&errStr);
        if (!ok) return errStr;
    }

    const QString name = generateObjectName("Prototype");
    AGeoObject * proto = new AGeoObject(name, nullptr);

    delete proto->Type; proto->Type = new ATypePrototypeObject();
    proto->migrateTo(Prototypes);

    removePresentInContainers(members);
    while (!members.empty())
    {
        AGeoObject * obj = members.front();
        obj->migrateTo(proto, true);
        members.erase(members.begin());
    }

    return "";
}

bool AGeometryHub::isValidPrototypeName(const QString & ProtoName) const
{
    if (!Prototypes) return false;
    for (AGeoObject * proto : Prototypes->HostedObjects)
        if (ProtoName == proto->Name) return true;
    return false;
}

void AGeometryHub::aboutToQuit()
{
    delete GeoManager; GeoManager = nullptr;
}

void rotate(TVector3 & v, double dPhi, double dTheta, double dPsi)
{
    v.RotateZ(TMath::Pi() / 180.0 * dPhi);
    TVector3 X(1.0, 0, 0);
    X.RotateZ(TMath::Pi() / 180.0 * dPhi);
    //v.RotateX( TMath::Pi()/180.0* Theta);
    v.Rotate(TMath::Pi() / 180.0 * dTheta, X);
    TVector3 Z(0, 0, 1.0);
    Z.Rotate(TMath::Pi() / 180.0 * dTheta, X);
    // v.RotateZ( TMath::Pi()/180.0* Psi );
    v.Rotate(TMath::Pi() / 180.0 * dPsi, Z);
}

void AGeometryHub::expandPrototypeInstances()
{
    if (Prototypes->HostedObjects.empty()) return;

    std::vector<AGeoObject*> Instances;
    World->findAllInstancesRecursive(Instances);

    for (AGeoObject * instanceObj : Instances)
    {
        instanceObj->clearContent();

        ATypeInstanceObject * insType = dynamic_cast<ATypeInstanceObject*>(instanceObj->Type);
        if (!insType)
        {
            qWarning() << "Instance" << instanceObj->Name << "has a wrong type!";
            return;
        }

        AGeoObject * prototypeObj = Prototypes->findObjectByName(insType->PrototypeName);
        if (!prototypeObj)
        {
            qWarning() << "Prototype" << insType->PrototypeName << "not found for instance" << instanceObj->Name;
            return;
        }

        for (AGeoObject * obj : prototypeObj->HostedObjects)
        {
            if (obj->isDisabled()) continue;

            AGeoObject * clone = obj->makeCloneForInstance(instanceObj->Name);
            clone->lockRecursively();
            instanceObj->addObjectLast(clone);
            clone->fActive = instanceObj->fActive;
        }
        instanceObj->fExpanded = false;
    }
}

bool AGeometryHub::processCompositeObject(AGeoObject * obj)
{
    AGeoObject * logicals = obj->getContainerWithLogical();
    if (!logicals)
    {
        QString err = "Composite object: Not found container with logical objects: " + obj->Name;
        AErrorHub::addQError(err);
        qWarning() << err;
        return false;
    }
    AGeoComposite * cs = dynamic_cast<AGeoComposite*>(obj->Shape);
    if (!cs)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(obj->Shape);
        if (!scaled)
        {
            QString err = "Composite: Shape object is not composite nor scaled composite: " + obj->Name;
            AErrorHub::addQError(err);
            qWarning() << err;
            return false;
        }

        // !!!*** scaled processing?
    }

    obj->refreshShapeCompositeMembers();

    for (AGeoObject * lobj : logicals->HostedObjects)
    {
        //registering building blocks
        const QString & name = lobj->Name;
        lobj->Shape->createGeoShape(name);
        const QString RotName = "_r" + name;
        TGeoRotation * lRot = new TGeoRotation(RotName.toLatin1().data(), lobj->Orientation[0], lobj->Orientation[1], lobj->Orientation[2]);
        lRot->RegisterYourself();
        const QString TransName = "_m" + name;
        TGeoCombiTrans * lTrans = new TGeoCombiTrans(TransName.toLatin1().data(), lobj->Position[0], lobj->Position[1], lobj->Position[2], lRot);
        lTrans->RegisterYourself();
    }

    return true;
}

#ifdef USE_ROOT_HTML
#include "aroothttpserver.h"
#endif
void AGeometryHub::populateGeoManager(bool notifyRootServer)
{
    ASensorHub::getInstance().clearSensors();
    clearMonitors();
    ACalorimeterHub::getInstance().clear();
    AGridHub::getInstance().clear();
    Scintillators.clear();
    ParticleAnalyzers.clear();
    PhotonFunctionals.clear();
    World->clearTrueRotationRecursive();

    AGeoConsts::getInstance().updateFromExpressions();
    World->introduceGeoConstValuesRecursive();
    World->updateAllMonitors();  // need to be before updateAllStacks to configure correct shapes!
    World->updateAllStacks();
    expandPrototypeInstances();

    delete GeoManager; GeoManager = new TGeoManager();
    GeoManager->SetVerboseLevel(0);
    GeoManager->SetMaxVisNodes(100000);

    if (DoScaling) World->scaleRecursive(ScalingFactor);

    double WorldSizeXY = getWorldSizeXY();
    double WorldSizeZ  = getWorldSizeZ();
    if (!isWorldSizeFixed())
    {
        WorldSizeXY = 0;
        WorldSizeZ  = 0;
        World->updateWorldSize(WorldSizeXY, WorldSizeZ);
        WorldSizeXY *= 1.05; WorldSizeZ *= 1.05;
        setWorldSizeXY(WorldSizeXY);
        setWorldSizeZ(WorldSizeZ);
    }

    AMaterialHub & MatHub = AMaterialHub::getInstance();
    MatHub.convertPressureToDensity();
    MatHub.generateGeoMedia();

    Top = GeoManager->MakeBox("WorldBox", MatHub[World->Material]->_GeoMed, WorldSizeXY, WorldSizeXY, WorldSizeZ);
    GeoManager->SetTopVolume(Top);
    GeoManager->SetTopVisible(true);

    AInterfaceRuleHub::getInstance().updateVolumesFromTo();
    addTGeoVolumeRecursively(World, Top);

    Top->SetName("World"); // "WorldBox" above is needed - JSROOT uses that name to avoid conflicts
    setVolumeTitle(World, Top);

    GeoManager->CloseGeometry();

    if (notifyRootServer) notifyRootServerGeometryChanged();
}

void AGeometryHub::notifyRootServerGeometryChanged()
{
#ifdef USE_ROOT_HTML
    ARootHttpServer::getInstance().onNewGeoManagerCreated();
#endif
}

#include "amonitor.h"
void AGeometryHub::addMonitorNode(AGeoObject * obj, TGeoVolume * vol, TGeoVolume * parent, TGeoCombiTrans * lTrans)
{
    ATypeMonitorObject * monTobj = static_cast<ATypeMonitorObject*>(obj->Type);

    const AMonitorHub::EType MonType = (monTobj->config.PhotonOrParticle == 0 ? AMonitorHub::Photon : AMonitorHub::Particle);

    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    const int MonitorCounter = MonitorHub.countMonitors(MonType);

    monTobj->index = MonitorCounter; // !!!*** need?
    parent->AddNode(vol, MonitorCounter, lTrans);

    TString fixedName = vol->GetName();
    fixedName += IndexSeparator;
    fixedName += MonitorCounter;
    vol->SetName(fixedName);

    AMonitorData md;
    md.Name     = QString(fixedName);
    md.GeoObj   = obj;
    md.Monitor  = new AMonitor(obj);

    TObjArray * nList = parent->GetNodes();
    const int numNodes = nList->GetEntries();
    const TGeoNode * node = (TGeoNode*)nList->At(numNodes - 1);
    getGlobalPosition(node, md.Position);

    if (MonType == AMonitorHub::Photon) MonitorHub.PhotonMonitors.push_back(md);
    else                                MonitorHub.ParticleMonitors.push_back(md);
}

#include "acalorimeter.h"
void AGeometryHub::addCalorimeterNode(AGeoObject * obj, TGeoVolume * vol, TGeoVolume * parent, TGeoCombiTrans * lTrans)
{
    ACalorimeterHub & CalorimeterHub = ACalorimeterHub::getInstance();
    const int CalorimeterCounter = CalorimeterHub.countCalorimeters();

    parent->AddNode(vol, CalorimeterCounter, lTrans);

    TString fixedName = vol->GetName();
    fixedName += IndexSeparator;
    fixedName += CalorimeterCounter;
    vol->SetName(fixedName);

    ACalorimeterData calData;
    calData.Name     = QString(fixedName);
    calData.GeoObj   = obj;
    calData.Calorimeter  = new ACalorimeter(obj);

    TObjArray * nList = parent->GetNodes();
    const int numNodes = nList->GetEntries();
    const TGeoNode * node = (TGeoNode*)nList->At(numNodes - 1);
    getGlobalPosition(node, calData.Position);
    getGlobalUnitVectors(node, calData.UnitXMaster, calData.UnitYMaster, calData.UnitZMaster);

    CalorimeterHub.Calorimeters.push_back(calData);
}

void AGeometryHub::addSensorNode(AGeoObject * obj, TGeoVolume * vol, TGeoVolume * parent, TGeoCombiTrans * lTrans)
{
    ASensorHub & SensorHub = ASensorHub::getInstance();
    const int SensorCounter = SensorHub.countSensors();

    parent->AddNode(vol, SensorCounter, lTrans);

    ASensorData sr;
    sr.GeoObj = obj;
    sr.ModelIndex = static_cast<AGeoSensor*>(obj->Role)->SensorModel;

    TObjArray * nList = parent->GetNodes();
    const int numNodes = nList->GetEntries();
    const TGeoNode * node = (TGeoNode*)nList->At(numNodes - 1);
    getGlobalPosition(node, sr.Position);

    SensorHub.registerNextSensor(sr);
}

bool AGeometryHub::findMotherNodeFor(const TGeoNode * node, const TGeoNode * startNode, const TGeoNode* & foundNode) const
{
    TGeoVolume * startVol = startNode->GetVolume();
    //qDebug() << "    Starting from"<<startVol->GetName();
    TObjArray * nList = startVol->GetNodes();
    if (!nList) return false;
    int numNodes = nList->GetEntries();
    //qDebug() << "    Num nodes:"<< numNodes;
    for (int inod=0; inod<numNodes; inod++)
    {
        TGeoNode * n = (TGeoNode*)nList->At(inod);
        //qDebug() << "    Checking "<< n->GetName();
        if (n == node)
        {
            //qDebug() << "    Match!";
            foundNode = startNode;
            return true;
        }
        //qDebug() << "    Sending down the line";
        bool bFound = findMotherNodeFor(node, n, foundNode);
        //qDebug() << "    Found?"<<bFound;
        if (bFound) return true;
    }
    return false;
}

void AGeometryHub::findMotherNode(const TGeoNode * node, const TGeoNode* & motherNode) const
{
    //qDebug() << "--- search for " << node->GetName();
    TObjArray* allNodes = GeoManager->GetListOfNodes();
    //qDebug() << allNodes->GetEntries();
    if (allNodes->GetEntries() != 1) return; // should be only World
    TGeoNode * worldNode = (TGeoNode*)allNodes->At(0);
    //qDebug() << worldNode->GetName();

    motherNode = worldNode;
    if (node == worldNode) return; //already there

    findMotherNodeFor(node, worldNode, motherNode); //bool OK = ...
    //if (bOK) qDebug() << "--- found mother node:"<<motherNode->GetName();
    //else qDebug() << "--- search failed!";
}

void AGeometryHub::getGlobalPosition(const TGeoNode * node, AVector3 & position) const
{
    double master[3];
    TGeoVolume * motherVol = node->GetMotherVolume();
    while (motherVol)
    {
        node->LocalToMaster(position.data(), master);
        position[0] = master[0];
        position[1] = master[1];
        position[2] = master[2];

        const TGeoNode * motherNode = nullptr;
        findMotherNode(node, motherNode);
        if (!motherNode)
        {
            //qDebug() << "  Mother node not found!";
            break;
        }
        if (motherNode == node)
        {
            //qDebug() << "  strange - world passed";
            break;
        }

        node = motherNode;

        motherVol = node->GetMotherVolume();
        //qDebug() << "  Continue search: current node:"<<n->GetName();
    }
}

void AGeometryHub::getGlobalUnitVectors(const TGeoNode * node, double * uvX, double * uvY, double * uvZ) const
{
    uvX[0] = 1.0; uvX[1] = 0.0; uvX[2] = 0.0;
    uvY[0] = 0.0; uvY[1] = 1.0; uvY[2] = 0.0;
    uvZ[0] = 0.0; uvZ[1] = 0.0; uvZ[2] = 1.0;

    double master[3];

    TGeoVolume * motherVol = node->GetMotherVolume();
    while (motherVol)
    {
        node->LocalToMasterVect(uvX, master); for (int i=0; i<3; i++) uvX[i] = master[i];
        node->LocalToMasterVect(uvY, master); for (int i=0; i<3; i++) uvY[i] = master[i];
        node->LocalToMasterVect(uvZ, master); for (int i=0; i<3; i++) uvZ[i] = master[i];

        const TGeoNode * motherNode = nullptr;
        findMotherNode(node, motherNode);
        if (!motherNode)
        {
            //qDebug() << "  Mother node not found!";
            break;
        }
        if (motherNode == node)
        {
            //qDebug() << "  strange - world passed";
            break;
        }

        node = motherNode;

        motherVol = node->GetMotherVolume();
        //qDebug() << "  Continue search: current node:"<<n->GetName();
    }
}

void AGeometryHub::addTGeoVolumeRecursively(AGeoObject * obj, TGeoVolume * parent, int forcedNodeNumber)
{
    if (!obj->fActive) return;

    TGeoVolume     * vol    = nullptr;
    TGeoCombiTrans * lTrans = nullptr;

    if      (obj->Type->isWorld())
        vol = parent; // resuse the cycle by HostedVolumes below
    else if (obj->Type->isPrototypeCollection() || obj->isCompositeMemeber() || obj->Type->isCompositeContainer())
        return;       // logicals do not host anything to be added to the geometry
    else if (obj->Type->isHandlingSet() || obj->Type->isHandlingArray() || obj->Type->isInstance())
        vol = parent; // group objects are pure virtual, pass the volume of the parent
    else
    {
        if (obj->Type->isComposite())
        {
            bool ok = processCompositeObject(obj);
            if (!ok) return;
        }

        AMaterialHub & MatHub = AMaterialHub::getInstance();
        vol = new TGeoVolume(obj->Name.toLocal8Bit().data(), obj->Shape->createGeoShape(), MatHub[obj->Material]->_GeoMed);

        TGeoRotation * lRot = nullptr;
        if (obj->TrueRot)
        {
            //parent is a virtual object
            lRot = obj->TrueRot;
            lTrans = new TGeoCombiTrans("lTrans", obj->TruePos[0], obj->TruePos[1], obj->TruePos[2], lRot);
        }
        else
        {
            //parent is a physical object
            lRot = new TGeoRotation("lRot", obj->Orientation[0], obj->Orientation[1], obj->Orientation[2]);
            lRot->RegisterYourself();
            lTrans = new TGeoCombiTrans("lTrans", obj->Position[0], obj->Position[1], obj->Position[2], lRot);
        }

        // Positioning this object if it is physical
        if (obj->Type->isGrid())
        {
            AGridHub & GridHub = AGridHub::getInstance();
            GridHub.GridRecords.push_back(GridHub.createGridRecord(obj));
            parent->AddNode(vol, GridHub.GridRecords.size() - 1, lTrans);
        }
        else if (obj->Type->isMonitor())
            addMonitorNode(obj, vol, parent, lTrans);
        else if (obj->isCalorimeter())
            addCalorimeterNode(obj, vol, parent, lTrans);
        else if (obj->isSensor())
            addSensorNode(obj, vol, parent, lTrans);
        else if (obj->isScintillator())
        {
            parent->AddNode(vol, Scintillators.size(), lTrans);
            TGeoNode * node = parent->GetNode(parent->GetNdaughters()-1);
            Scintillators.push_back({obj, node});
        }
        else if (obj->isParticleAnalyzer())
        {
            parent->AddNode(vol, ParticleAnalyzers.size(), lTrans);
            TGeoNode * node = parent->GetNode(parent->GetNdaughters()-1);
            AVector3 globalPosition;
            getGlobalPosition(node, globalPosition);
            ParticleAnalyzers.push_back({obj, node, globalPosition});
        }
        else
            parent->AddNode(vol, forcedNodeNumber, lTrans);
    }

    // Position hosted objects
    if      (obj->Type->isHandlingArray())
        positionArray(obj, vol, forcedNodeNumber);
    else if (obj->Type->isStack())
        positionStack(obj, vol, forcedNodeNumber);
    else if (obj->Type->isInstance())
        positionInstance(obj, vol, forcedNodeNumber);
    else
        for (AGeoObject * el : obj->HostedObjects)
            addTGeoVolumeRecursively(el, vol, forcedNodeNumber);

    if (obj->Role && obj->Role->getType() == "PhotonFunctional") registerPhotonFunctional(obj, parent);

    setVolumeTitle(obj, vol);
}

void AGeometryHub::setVolumeTitle(AGeoObject * obj, TGeoVolume * vol)
{
    //  Photon tracer uses volume title for identification of special volumes
    //  First character ([0]) can be 'M' for monitor, 'S' for light sensor, '2' for secondary scintillator, 'G' for optical grid
    //  * in the second (or third) places indicate that this volume has a defined optical interface rule from (or to)
    // the forth position ([3]) is currently not used
    // starting from index 4

    TString title = "----";

    if      (obj->Role)
    {
        if      (obj->Role->getType() == "Sensor")           title[0] = 'S';
        else if (obj->Role->getType() == "SecScint")         title[0] = '2';
        else if (obj->Role->getType() == "Calorimeter")      title[0] = 'C'; // not used with photons, but needed to update name for interface rules
        else if (obj->Role->getType() == "PhotonFunctional") title[0] = 'F';
    }
    else if (obj->Type->isMonitor())
    {
        ATypeMonitorObject * monTobj = static_cast<ATypeMonitorObject*>(obj->Type);
        if (monTobj->config.PhotonOrParticle == 0)           title[0] = 'M';
    }
    else if (obj->Type->isGrid())                            title[0] = 'G';

    const bool bInstance = (!obj->NameWithoutSuffix.isEmpty());
    TString BaseName;
    if (bInstance) BaseName = TString(obj->NameWithoutSuffix.toLatin1().data());
    else
    {
        BaseName = vol->GetName();
        if (title[0] != '-') AGeometryHub::getConstInstance().removeNameDecorators(BaseName);
    }

    const AInterfaceRuleHub & IRH = AInterfaceRuleHub::getConstInstance();
    if (IRH.isFromVolume(BaseName)) title[1] = '*';
    if (IRH.isToVolume(BaseName))   title[2] = '*';

    if (bInstance) title += BaseName;
    // !!!*** consider setting titles only for volumes with named overrides from/to

    vol->SetTitle(title);
}

void AGeometryHub::registerPhotonFunctional(AGeoObject * obj, TGeoVolume * parentVol)
{
    TGeoNode * node = parentVol->GetNode(parentVol->GetNdaughters()-1);
    node->SetNumber(PhotonFunctionals.size());
    AVector3 globalPosition;
    getGlobalPosition(node, globalPosition);
    PhotonFunctionals.push_back({obj, node, globalPosition});
}

void AGeometryHub::fillParticleAnalyzerRecords(AParticleAnalyzerSettings * settings) const
{
    settings->AnalyzerTypes.clear();
    settings->UniqueToTypeLUT.clear();
    settings->GlobalToUniqueLUT.clear();

    int typeIndex = 0;
    int uniqueIndex = 0;
    for (const auto & tup : ParticleAnalyzers)
    {
        const AGeoSpecial * role = std::get<0>(tup)->Role;
        const AGeoParticleAnalyzer * pa = static_cast<const AGeoParticleAnalyzer*>(role);
        const AParticleAnalyzerRecord & rec = pa->Properties;

        const std::string volumeName = std::get<0>(tup)->Name.toLatin1().data();

        std::string baseName = std::get<0>(tup)->NameWithoutSuffix.toLatin1().data();
        // all AGeoObjects not belonging to an instance have baseName empty
        if (baseName.empty()) baseName = volumeName;

        bool found = false;
        for (AParticleAnalyzerRecord & filledRecord : settings->AnalyzerTypes) // !!!*** consider map
        {
            if (baseName == filledRecord.VolumeBaseName)  // record for this base type already exists
            {
                if (rec.SingleInstanceForAllCopies)
                {
                    settings->GlobalToUniqueLUT.push_back(filledRecord.UniqueIndex); // reuse unique
                    // UniqueToTypeLUT already filled
                }
                else
                {
                    settings->GlobalToUniqueLUT.push_back(uniqueIndex);
                    settings->UniqueToTypeLUT.push_back(filledRecord.TypeIndex);
                    uniqueIndex++;
                }

                filledRecord.addVolumeNameIfNew(volumeName);

                found = true;
                break;
            }
        }

        if (!found)
        {
            AParticleAnalyzerRecord recordCopy = rec;

                recordCopy.TypeIndex      = typeIndex;   // just tmp
                recordCopy.UniqueIndex    = uniqueIndex; // just tmp

                recordCopy.VolumeBaseName = baseName;
                recordCopy.addVolumeNameIfNew(volumeName);

            settings->AnalyzerTypes.push_back(recordCopy);

            settings->UniqueToTypeLUT.push_back(typeIndex);
            settings->GlobalToUniqueLUT.push_back(uniqueIndex);

            typeIndex++;
            uniqueIndex++;
        }
    }

    settings->NumberOfUniqueAnalyzers = uniqueIndex;
}

void AGeometryHub::positionArray(AGeoObject * obj, TGeoVolume * vol, int parentNodeIndex)
{
    ATypeArrayObject * array = static_cast<ATypeArrayObject*>(obj->Type);

    ATypeCircularArrayObject  * circArray = dynamic_cast<ATypeCircularArrayObject*>(obj->Type);
    ATypeHexagonalArrayObject * hexArray  = dynamic_cast<ATypeHexagonalArrayObject*>(obj->Type);

    int iCounterStart = array->startIndex;
    if (array->strStartIndex.contains("ParentIndex"))
    {
        QString tmpStr = array->strStartIndex;
        tmpStr.replace("ParentIndex", QString::number(parentNodeIndex));
        const AGeoConsts & GC = AGeoConsts::getConstInstance();
        QString errorStr;
        bool ok = GC.updateIntParameter(errorStr, tmpStr, iCounterStart, false, true);
        if (!ok) qWarning() << "Error in start index using ParentIndex";
    }

    for (AGeoObject * el : obj->HostedObjects)
    {
        //int iCounter = array->startIndex;
        int iCounter = iCounterStart;
        if (iCounter < 0) iCounter = 0;

        if (circArray)
        {
            for (int ia = 0; ia < circArray->num; ia++)
                positionCircularArrayElement(ia, el, obj, vol, iCounter++);
        }
        else if (hexArray)
        {
            if (hexArray->Shape == ATypeHexagonalArrayObject::Hexagonal)
            {
                for (int iR = 0; iR < hexArray->Rings; iR++)
                    positionHexArrayRing(iR, el, obj, vol, iCounter);
                iCounter++;
            }
            else
            {
                double delta = 0;
                if (!hexArray->SkipEvenFirst &&  hexArray->SkipOddLast) delta = 0;
                if (!hexArray->SkipEvenFirst && !hexArray->SkipOddLast) delta = -0.25 * hexArray->Step;
                if ( hexArray->SkipEvenFirst && !hexArray->SkipOddLast) delta = -0.5  * hexArray->Step;
                if ( hexArray->SkipEvenFirst &&  hexArray->SkipOddLast) delta = -0.25  * hexArray->Step;

                for (int iy = 0; iy < hexArray->NumY; iy++)
                {
                    bool bOdd = ( (iy+1) % 2 == 0);
                    for (int ix = 0; ix < hexArray->NumX; ix++)
                    {
                        if (hexArray->SkipEvenFirst && !bOdd && ix == 0) continue;
                        if (hexArray->SkipOddLast   &&  bOdd && ix == hexArray->NumX-1) continue;

                        double x = el->Position[0] + (ix - 0.5*(hexArray->NumX - 1)) * hexArray->Step + delta;
                        if (bOdd) x +=  0.5 * hexArray->Step;
                        double y = el->Position[1] + (iy - 0.5*(hexArray->NumY - 1)) * hexArray->Step * cos(30.0*3.1415926535/180.0);

                        positionHexArrayElement(x, y, el, obj, vol, iCounter++);
                    }
                }
            }
        }
        else
        {
            for (int iz = 0; iz < array->numZ; iz++)
                for (int iy = 0; iy < array->numY; iy++)
                    for (int ix = 0; ix < array->numX; ix++)
                        positionArrayElement(ix, iy, iz, el, obj, vol, iCounter++);
        }
    }
}

void AGeometryHub::positionStack(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber)
{
    const ATypeStackContainerObject * stack = static_cast<const ATypeStackContainerObject*>(obj->Type);
    const QString & RefObjName = stack->ReferenceVolume;
    const AGeoObject * RefObj = nullptr;
    for (const AGeoObject * el : obj->HostedObjects)
        if (el->Name == RefObjName)
        {
            RefObj = el;
            break;
        }

    if (RefObj)
    {
        for (AGeoObject * el : obj->HostedObjects)
            positionStackElement(el, RefObj, vol, forcedNodeNumber);
    }
    else qWarning() << "Error: Reference object not found for stack" << obj->Name;
}

void AGeometryHub::positionInstance(AGeoObject * obj, TGeoVolume * vol, int forcedNodeNumber)
{
    for (AGeoObject * el : obj->HostedObjects)
    {
        //Position
        double local[3], master[3];
        local[0] = el->Position[0];
        local[1] = el->Position[1];
        local[2] = el->Position[2];
        TGeoRotation ArRot("0", obj->Orientation[0] , obj->Orientation[1], obj->Orientation[2]);
        if (obj->TrueRot)
        {
            obj->TrueRot->LocalToMaster(local, master);
            for (int i = 0; i < 3; i++)
                el->TruePos[i] = master[i] + obj->TruePos[i];
        }
        else
        {
            ArRot.LocalToMaster(local, master);
            for (int i = 0; i < 3; i++)
                el->TruePos[i] = master[i] + obj->Position[i];
        }

        //Orientation
        TGeoRotation elRot("1", el->Orientation[0], el->Orientation[1], el->Orientation[2]);
        if (obj->TrueRot)
            el->TrueRot = createCombinedRotation(&elRot, obj->TrueRot);
        else
            el->TrueRot = createCombinedRotation(&elRot, &ArRot);
        el->TrueRot->RegisterYourself();

        addTGeoVolumeRecursively(el, vol, forcedNodeNumber);
    }
}

void AGeometryHub::positionArrayElement(int ix, int iy, int iz, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex)
{
    ATypeArrayObject* array = static_cast<ATypeArrayObject*>(arrayObj->Type);

    double local[3], master[3];
    if (array->bCenterSymmetric)
    {
        local[0] = el->Position[0] + (ix - 0.5*(array->numX - 1)) * array->stepX;
        local[1] = el->Position[1] + (iy - 0.5*(array->numY - 1)) * array->stepY;
        local[2] = el->Position[2] + (iz - 0.5*(array->numZ - 1)) * array->stepZ;
    }
    else
    {
        local[0] = el->Position[0] + ix * array->stepX;
        local[1] = el->Position[1] + iy * array->stepY;
        local[2] = el->Position[2] + iz * array->stepZ;
    }

    TGeoRotation ArRot("0", arrayObj->Orientation[0] , arrayObj->Orientation[1], arrayObj->Orientation[2]);
    if (arrayObj->TrueRot)
    {
        arrayObj->TrueRot->LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->TruePos[i];
    }
    else
    {
        ArRot.LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->Position[i];
    }

    //Orientation
    TGeoRotation elRot("1", el->Orientation[0], el->Orientation[1], el->Orientation[2]);
    if (arrayObj->TrueRot)
        el->TrueRot = createCombinedRotation(&elRot, arrayObj->TrueRot);
    else
        el->TrueRot = createCombinedRotation(&elRot, &ArRot);
    el->TrueRot->RegisterYourself();

    addTGeoVolumeRecursively(el, parent, arrayIndex);
}

void AGeometryHub::positionCircularArrayElement(int ia, AGeoObject * el, AGeoObject * arrayObj, TGeoVolume * parent, int arrayIndex)
{
    ATypeCircularArrayObject * array = static_cast<ATypeCircularArrayObject*>(arrayObj->Type);

    double angle = array->angularStep * ia;

    //Position
    double local[3], master[3];
    local[0] = el->Position[0] + array->radius;
    local[1] = el->Position[1];
    local[2] = el->Position[2];
    TGeoRotation ArRot = ( arrayObj->TrueRot ? TGeoRotation("0", 0, 0, angle)
                                             : TGeoRotation("0", arrayObj->Orientation[0], arrayObj->Orientation[1], arrayObj->Orientation[2] + angle) );
    if (arrayObj->TrueRot)
    {
        TGeoRotation * Rot = createCombinedRotation(&ArRot, arrayObj->TrueRot);
        Rot->LocalToMaster(local, master);
        delete Rot;
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->TruePos[i];
    }
    else
    {
        ArRot.LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->Position[i];
    }

    //Orientation
    TGeoRotation elRot("1", el->Orientation[0], el->Orientation[1], el->Orientation[2]);
    el->TrueRot = createCombinedRotation(&elRot, &ArRot, arrayObj->TrueRot);
    el->TrueRot->RegisterYourself();

    addTGeoVolumeRecursively(el, parent, arrayIndex);
}

void AGeometryHub::positionHexArrayElement(double localX, double localY, AGeoObject *el, AGeoObject *arrayObj, TGeoVolume *parent, int arrayIndex)
{
    //Position
    double local[3], master[3];
    local[0] = el->Position[0] + localX;
    local[1] = el->Position[1] + localY;
    local[2] = el->Position[2];
    TGeoRotation ArRot("0", arrayObj->Orientation[0], arrayObj->Orientation[1], arrayObj->Orientation[2]);
    if (arrayObj->TrueRot)
    {
        arrayObj->TrueRot->LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->TruePos[i];
    }
    else
    {
        ArRot.LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + arrayObj->Position[i];
    }

    //Orientation
    TGeoRotation elRot("1", el->Orientation[0], el->Orientation[1], el->Orientation[2]);
    if (arrayObj->TrueRot)
        el->TrueRot = createCombinedRotation(&elRot, arrayObj->TrueRot);
    else
        el->TrueRot = createCombinedRotation(&elRot, &ArRot);
    el->TrueRot->RegisterYourself();

    addTGeoVolumeRecursively(el, parent, arrayIndex);
}

void AGeometryHub::positionHexArrayRing(int iR, AGeoObject *el, AGeoObject *arrayObj, TGeoVolume *parent, int & arrayIndex)
{
    ATypeHexagonalArrayObject * array = static_cast<ATypeHexagonalArrayObject*>(arrayObj->Type);

    double CtC = array->Step;
    double CtCbis = CtC * cos(30.0*3.1415926535/180.0);

    double x, y;
    if (iR == 0)
        positionHexArrayElement(0,0, el, arrayObj, parent, arrayIndex++);
    else
    {
        x = iR * CtC;
        y = 0;
        positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        for (int j=1; j<iR+1; j++)
        {  //   //
            x -= 0.5*CtC;
            y -= CtCbis;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
        for (int j=1; j<iR+1; j++)
        {   // --
            x -= CtC;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
        for (int j=1; j<iR+1; j++)
        {  // \\ //
            x -= 0.5*CtC;
            y += CtCbis;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
        for (int j=1; j<iR+1; j++)
        {  // //
            x += 0.5*CtC;
            y += CtCbis;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
        for (int j=1; j<iR+1; j++)
        {   // --
            x += CtC;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
        for (int j=1; j<iR; j++)
        {  // \\       //dont do the last step - already positioned PM
            x += 0.5*CtC;
            y -= CtCbis;
            positionHexArrayElement(x,y, el, arrayObj, parent, arrayIndex++);
        }
    }
}

void AGeometryHub::positionStackElement(AGeoObject * el, const AGeoObject * RefObj, TGeoVolume *parent, int forcedNodeNumber)
{
    AGeoObject * Stack = el->Container;

    //Position
    double local[3], master[3];
    local[0] = el->Position[0] - RefObj->Position[0];
    local[1] = el->Position[1] - RefObj->Position[1];
    local[2] = el->Position[2] - RefObj->Position[2];
    TGeoRotation StackRot("0", Stack->Orientation[0], Stack->Orientation[1], Stack->Orientation[2]);
    if (Stack->TrueRot)
    {
        Stack->TrueRot->LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + RefObj->Position[i] + Stack->TruePos[i];
    }
    else
    {
        StackRot.LocalToMaster(local, master);
        for (int i = 0; i < 3; i++)
            el->TruePos[i] = master[i] + RefObj->Position[i] + Stack->Position[i];
    }

    //Orientation
    TGeoRotation elRot("1", el->Orientation[0], el->Orientation[1], el->Orientation[2]);
    if (Stack->TrueRot)
        el->TrueRot = createCombinedRotation(&elRot, Stack->TrueRot);
    else
        el->TrueRot = createCombinedRotation(&elRot, &StackRot);
    el->TrueRot->RegisterYourself();

    addTGeoVolumeRecursively(el, parent, forcedNodeNumber);
}

TGeoRotation * AGeometryHub::createCombinedRotation(TGeoRotation * firstRot, TGeoRotation * secondRot, TGeoRotation * thirdRot)
{
    double local[3], master[3];

    local[0] = 1.0; local[1] = 0; local[2] = 0;
    firstRot->LocalToMaster(local, master);
    local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
    secondRot->LocalToMaster(local, master);
    if (thirdRot)
    {
        local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
        thirdRot->LocalToMaster(local, master);
    }
    double X3 = master[2];
    double X2 = master[1];
    double X1 = master[0];

    local[0] = 0; local[1] = 1.0; local[2] = 0;
    firstRot->LocalToMaster(local, master);
    local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
    secondRot->LocalToMaster(local, master);
    if (thirdRot)
    {
        local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
        thirdRot->LocalToMaster(local, master);
    }
    double Y3 = master[2];
    double Y2 = master[1];
    double Y1 = master[0];

    local[0] = 0; local[1] = 0; local[2] = 1.0;
    firstRot->LocalToMaster(local, master);
    local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
    secondRot->LocalToMaster(local, master);
    if (thirdRot)
    {
        local[0] = master[0]; local[1] = master[1]; local[2] = master[2];
        thirdRot->LocalToMaster(local, master);
    }
    double Z3 = master[2];
    double Z2 = master[1];
    double Z1 = master[0];

    TGeoRotation * Rot = new TGeoRotation();
    double rotMat[9] = {X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3};
    Rot->SetMatrix(rotMat);

    return Rot;
}

bool AGeometryHub::isMaterialInUse(int imat, QString & volName) const
{
    return World->isMaterialInUse(imat, volName);
}

void AGeometryHub::onMaterialRemoved(int imat)
{
    World->onMaterialRemoved(imat);

    populateGeoManager();
}

bool AGeometryHub::isVolumeExistAndActive(const QString & name) const
{
    AGeoObject * obj = World->findObjectByName(name);
    if (!obj) return false;

    return obj->fActive;
}

void AGeometryHub::changeLineWidthOfVolumes(int delta)
{
    World->changeLineWidthRecursive(delta);
}

void AGeometryHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;
    {
        QJsonArray arrTree;
        World->writeAllToJarr(arrTree);
        js["WorldTree"] = arrTree;

        QJsonArray arrGC;
        AGeoConsts::getConstInstance().writeToJsonArr(arrGC);
        js["GeoConsts"] = arrGC;
    }
    json["Geometry"] = js;
}

QString AGeometryHub::readFromJson(const QJsonObject & json)
{
    QJsonObject js;
    bool ok = jstools::parseJson(json, "Geometry", js);
    if (!ok) return "Json does not contain geometry settings!";

    clearWorld();

    QJsonArray arrGC;
    jstools::parseJson(js, "GeoConsts", arrGC);
    AGeoConsts::getInstance().readFromJsonArr(arrGC);

    QJsonArray arrTree;
    jstools::parseJson(js, "WorldTree", arrTree);
    QString Error = World->readAllFromJarr(World, arrTree);
    if (!Error.isEmpty()) return Error;

    //if config contained Prototypes, there are two protoypes objects in the geometry now!
    for (AGeoObject * obj : World->HostedObjects)
    {
        if (!obj->Type->isPrototypeCollection()) continue;
        if (obj == Prototypes) continue;
        //found another Prototypes object  - it was loaded from json
        for (AGeoObject * proto : obj->HostedObjects)
            Prototypes->addObjectLast(proto);
        obj->HostedObjects.clear();
        World->removeHostedObject(obj);
        delete obj;
        break;
    }

    populateGeoManager();
    return "";
}

bool AGeometryHub::isWorldSizeFixed() const
{
    ATypeWorldObject * wt = static_cast<ATypeWorldObject*>(World->Type);
    return wt->bFixedSize;
}

void AGeometryHub::setWorldSizeFixed(bool bFlag)
{
    ATypeWorldObject * wt = static_cast<ATypeWorldObject*>(World->Type);
    wt->bFixedSize = bFlag;
}

double AGeometryHub::getWorldSizeXY() const
{
    AGeoBox * box = dynamic_cast<AGeoBox*>(World->Shape);
    return ( box ? box->dx : 0 );
}

void AGeometryHub::setWorldSizeXY(double size)
{
    AGeoBox * box = dynamic_cast<AGeoBox*>(World->Shape);
    if (box)
    {
        box->dx = size;
        box->dy = size;
        box->str2dx.clear();
        box->str2dy.clear();
    }
}

double AGeometryHub::getWorldSizeZ() const
{
    AGeoBox * box = dynamic_cast<AGeoBox*>(World->Shape);
    return ( box ? box->dz : 0 );
}

void AGeometryHub::setWorldSizeZ(double size)
{
    AGeoBox * box = dynamic_cast<AGeoBox*>(World->Shape);
    if (box)
    {
        box->dz = size;
        box->str2dz.clear();
    }
}

void AGeometryHub::colorVolumes(int scheme, int id)
{
    //scheme = 0 - default
    //scheme = 1 - by material
    //scheme = 2 - volumes made of material index=id will be red, the rest - black

    TObjArray * list = GeoManager->GetListOfVolumes();
    int size = list->GetEntries();
    for (int iVol = 0; iVol < size; iVol++)
    {
        TGeoVolume* vol = (TGeoVolume*)list->At(iVol);
        if (!vol) break;

        QString name = vol->GetName();
        switch (scheme)
        {
        case 0:  //default color volumes for PMs and dPMs otherwise color from AGeoObject
            if      (name.startsWith("PM"))  vol->SetLineColor(kGreen);  // !!!*** obsolete?
            else
            {
                const AGeoObject * obj = World->findObjectByName(name); // !!!*** can be very slow for large detectors!
                if (!obj && !name.isEmpty())
                {
                    //special for monitors
                    QString mName = name.split(IndexSeparator.Data()).at(0);
                    obj = World->findObjectByName(mName);
                }
                if (obj)
                {
                    vol->SetLineColor(obj->color);
                    vol->SetLineWidth(obj->width);
                    vol->SetLineStyle(obj->style);
                }
                else vol->SetLineColor(kGray);
                //qDebug() << name << obj << vol->GetTitle();
            }
            break;
        case 1:  //color by material
            vol->SetLineColor(vol->GetMaterial()->GetIndex() + 1);
            break;
        case 2:  //highlight a given material
            vol->SetLineColor( vol->GetMaterial()->GetIndex() == id ? kRed : kBlack );
            break;
        }
    }
}

int AGeometryHub::checkGeometryForConflicts()
{
    if (!GeoManager) return 0;

    const double Precision = 0.01; //overlap search precision - in cm

    GeoManager->ClearOverlaps();
    int segments = GeoManager->GetNsegments();

    GeoManager->CheckOverlaps(Precision);
    TObjArray * overlaps = GeoManager->GetListOfOverlaps();
    int overlapCount = overlaps->GetEntries();
    if (overlapCount == 0)
    {
        // Repeating the search with sampling
        //qDebug() << "No overlaps found, checking using sampling method..";
        GeoManager->CheckOverlaps(Precision, "s"); // could be "sd", but the result is the same
        overlaps = GeoManager->GetListOfOverlaps();
        overlapCount = overlaps->GetEntries();
    }

    GeoManager->SetNsegments(segments);  //restore back, get auto reset during the check to some bad default value
    return overlapCount;
}

QString AGeometryHub::exportGeometry(const QString & fileName)
{
    QFileInfo fi(fileName);
    const QString suffix = fi.suffix();
    if (!suffix.compare("gdml", Qt::CaseInsensitive))
        if (!suffix.compare("root", Qt::CaseInsensitive))
            return "File name should have .gdml or .root suffix";

    QJsonObject json;
    writeToJson(json);
    DoScaling = true;
    ScalingFactor = 0.1; // 1[mm] becomes 0.1[cm]
    populateGeoManager();

    GeoManager->SetName("geometry");
    int res = GeoManager->Export(fileName.toLocal8Bit().data());

    DoScaling = false;
    readFromJson(json);
    populateGeoManager();

    return (res == 0 ? "Failed to export to file "+fileName : "");
}

/*
QString AGeometryHub::exportToGDML(const QString & fileName) const
{
    QFileInfo fi(fileName);
    if (fi.suffix().compare("gdml", Qt::CaseInsensitive))
        return "File name should have .gdml extension";

    QByteArray ba = fileName.toLocal8Bit();
    const char *c_str = ba.data();
    GeoManager->SetName("geometry");
    GeoManager->Export(c_str);

    QFile f(fileName);
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        QString txt = in.readAll();
        f.close();

        if (f.remove())
        {
            txt.replace("unit=\"cm\"", "unit=\"mm\"");
            bool bOK = ftools::saveTextToFile(txt, fileName);
            if (bOK) return "";
        }
    }
    return "Error during cm->mm conversion stage";
}

QString AGeometryHub::exportToROOT(const QString & fileName) const
{
    QFileInfo fi(fileName);
    if (fi.suffix().compare("root", Qt::CaseInsensitive))
        return "File name should have .root extension";

    QByteArray ba = fileName.toLocal8Bit();
    const char *c_str = ba.data();
    GeoManager->SetName("geometry");
    GeoManager->Export(c_str);

    return "";
}
*/

//#include "a3global.h"
QString AGeometryHub::readGDMLtoTGeo(const QString & fileName)
{
    QString txt;
    bool bOK = ftools::loadTextFromFile(txt, fileName);
    if (!bOK) return "Cannot open file " + fileName;

    if (txt.contains("unit=\"cm\"") || txt.contains("unit=\"m\""))
        return "Cannot load GDML files with length units other than \"mm\"";

    txt.replace("unit=\"mm\"", "unit=\"cm\"");
    //const QString tmpFileName = A3Global::getConstInstance().ExecutableDir + "/gdmlTMP.gdml";
    const QString tmpFileName = "gdmlTMP.gdml";
    bOK = ftools::saveTextToFile(txt, tmpFileName);
    if (!bOK) return "Conversion failed - tmp file cannot be allocated";

    delete GeoManager; GeoManager = nullptr;
    GeoManager = TGeoManager::Import(tmpFileName.toLatin1());
    QFile(tmpFileName).remove();

    if (!GeoManager || !GeoManager->IsClosed())
    {
        GeoManager = new TGeoManager();
        return "Load GDML failed!"; // needs rebuild!
    }
    qDebug() << "--> tmp GeoManager loaded from GDML file";

    return "";
}

#include <TVector3.h>
#include <TMatrixD.h>
#include <TMath.h>

// input: R - rotation matrix
// return: vector of Euler angles in X-convention (Z0, X, Z1)
// based on the pseudocode by David Eberly from
// https://www.geometrictools.com/Documentation/EulerAngles.pdf
TVector3 euler(TMatrixD R)  // !!!*** need?
{
    double tol = 1e-6; // tolerance to detect special cases
    double Pi = TMath::Pi();
    double thetaZ0, thetaX, thetaZ1; // Euler angles

    if (R(2,2) < 1.-tol) {
        if (R(2,2) > -1.+tol) {
            thetaX = acos(R(2,2));
            thetaZ0 = atan2(R(0,2), -R(1,2));
            thetaZ1 = atan2(R(2,0), R(2,1));
        } else { // r22 == -1.
            thetaX = Pi;
            thetaZ0 = -atan2(-R(0,1), R(0,0));
            thetaZ1 = 0.;
        }
    } else { // r22 == +1.
        thetaX = 0.;
        thetaZ0 = atan2(-R(0,1), R(0,0));
        thetaZ1 = 0.;
    }
    return TVector3(thetaZ0, thetaX, thetaZ1);
}

void processNonComposite(QString Name, TGeoShape* Tshape, const TGeoMatrix* Matrix, QVector<AGeoObject*>& LogicalObjects)
{
    //qDebug() << Name;
    TGeoTranslation trans(*Matrix);
    //qDebug() << "Translation:"<<trans.GetTranslation()[0]<<trans.GetTranslation()[1]<<trans.GetTranslation()[2];
    TGeoRotation rot(*Matrix);
    double phi, theta, psi;
    rot.GetAngles(phi, theta, psi);
    //qDebug() << "Rotation:"<<phi<<theta<<psi;

    AGeoObject* GeoObj = new AGeoObject(Name);
    for (int i=0; i<3; i++) GeoObj->Position[i] = trans.GetTranslation()[i];
    GeoObj->Orientation[0] = phi; GeoObj->Orientation[1] = theta; GeoObj->Orientation[2] = psi;
    delete GeoObj->Shape;
    GeoObj->Shape = AGeoShape::GeoShapeFactory(Tshape->ClassName());
    if (!GeoObj->Shape)
    {
        qWarning() << "Unknown TGeoShape:"<<Tshape->ClassName();
        GeoObj->Shape = new AGeoBox();
    }
    bool fOK = GeoObj->Shape->readFromTShape(Tshape);
    if (!fOK)
    {
        qWarning() << "Not implemented: import data from"<<Tshape->ClassName()<< "to ANTS2 object";
        GeoObj->Shape = new AGeoBox();
    }
    LogicalObjects << GeoObj;
}

bool isLogicalObjectsHaveName(const QVector<AGeoObject*>& LogicalObjects, const QString name)
{
    for (AGeoObject* obj : LogicalObjects)
        if (obj->Name == name) return true;
    return false;
}

#include "TGeoBoolNode.h"
#include "TGeoCompositeShape.h"
void processTCompositeShape(TGeoCompositeShape* Tshape, QVector<AGeoObject*>& LogicalObjects, QString& GenerationString )
{
    TGeoBoolNode* n = Tshape->GetBoolNode();
    if (!n)
    {
        qWarning() << "Failed to read BoolNode in TCompositeShape!";
    }

    TGeoBoolNode::EGeoBoolType operation = n->GetBooleanOperator(); // kGeoUnion, kGeoIntersection, kGeoSubtraction
    QString operationStr;
    switch (operation)
    {
    default:
        qCritical() << "Unknown EGeoBoolType, assuming it is kGeoUnion";
    case TGeoBoolNode::kGeoUnion:
        operationStr = " + "; break;
    case TGeoBoolNode::kGeoIntersection:
        operationStr = " * "; break;
    case TGeoBoolNode::kGeoSubtraction:
        operationStr = " - "; break;
    }
    //qDebug() << "UnionIntersectSubstr:"<<operationStr;

    TGeoShape* left = n->GetLeftShape();
    QString leftName;
    TGeoCompositeShape* CompositeShape = dynamic_cast<TGeoCompositeShape*>(left);
    if (CompositeShape)
    {
        QString tmp;
        processTCompositeShape(CompositeShape, LogicalObjects, tmp);
        if (tmp.isEmpty())
            qWarning() << "Error processing TGeoComposite: no generation string obtained";
        leftName = " (" + tmp + ") ";
    }
    else
    {
        QString leftNameBase = leftName = left->GetName();
        while (isLogicalObjectsHaveName(LogicalObjects, leftName))
            leftName = leftNameBase + "_" + AGeoObject::GenerateRandomName();
        processNonComposite(leftName, left, n->GetLeftMatrix(), LogicalObjects);
    }

    TGeoShape* right = n->GetRightShape();
    QString rightName;
    CompositeShape = dynamic_cast<TGeoCompositeShape*>(right);
    if (CompositeShape)
    {
        QString tmp;
        processTCompositeShape(CompositeShape, LogicalObjects, tmp);
        if (tmp.isEmpty())
            qWarning() << "Error processing TGeoComposite: no generation string obtained";
        rightName = " (" + tmp + ") ";
    }
    else
    {
        QString rightNameBase = rightName = right->GetName();
        while (isLogicalObjectsHaveName(LogicalObjects, rightName))
            rightName = rightNameBase + "_" + AGeoObject::GenerateRandomName();
        processNonComposite(rightName, right, n->GetRightMatrix(), LogicalObjects);
    }

    GenerationString = " " + leftName + operationStr + rightName + " ";
    //qDebug() << leftName << operationStr << rightName;
}

void readGeoObjectTree(AGeoObject * obj, const TGeoNode * node)
{
    obj->Name = node->GetName();

    // Material
    //const QString mat = node->GetVolume()->GetMaterial()->GetName();
    //qDebug() << mat << node->GetVolume()->GetMaterial()->GetIndex();
    obj->Material = node->GetVolume()->GetMaterial()->GetIndex();

    obj->color = obj->Material+1;
    obj->fExpanded = true;

    // Shape
    TGeoShape * Tshape = node->GetVolume()->GetShape();
    const QString Sshape = Tshape->ClassName();
    //qDebug() << "TGeoShape:"<<Sshape;
    AGeoShape * Ashape = AGeoShape::GeoShapeFactory(Sshape);
    bool fOK = false;
    if (!Ashape) qWarning() << "TGeoShape was not recognized - using box";
    else
    {
        delete obj->Shape; //delete default one
        obj->Shape = Ashape;

        fOK = Ashape->readFromTShape(Tshape);  //composite has special procedure!
        if (Ashape->getShapeType() == "TGeoCompositeShape")
        {
            //TGeoShape -> TGeoCompositeShape
            TGeoCompositeShape* tshape = static_cast<TGeoCompositeShape*>(Tshape);

            //AGeoObj converted to composite type:
            delete obj->Type; obj->Type = new ATypeCompositeObject();
            //creating container for logical objects:
            AGeoObject* logicals = new AGeoObject();
            logicals->Name = "CompositeSet_"+obj->Name;
            delete logicals->Type; logicals->Type = new ATypeCompositeContainerObject();
            obj->addObjectFirst(logicals);

            QVector<AGeoObject*> AllLogicalObjects;
            QString GenerationString;
            processTCompositeShape(tshape, AllLogicalObjects, GenerationString);
            AGeoComposite* cshape = static_cast<AGeoComposite*>(Ashape);
            cshape->GenerationString = "TGeoCompositeShape(" + GenerationString + ")";
            for (AGeoObject* ob : AllLogicalObjects)
            {
                ob->Material = obj->Material;
                logicals->addObjectLast(ob);
                cshape->members << ob->Name;
            }
            //qDebug() << cshape->GenerationString;// << cshape->members;

            fOK = true;
        }
    }
    //qDebug() << "Shape:"<<Sshape<<"Read success:"<<fOK;
    if (!fOK) qDebug() << "Failed to read shape for:"<<obj->Name;

    //position + angles
    const TGeoMatrix* matrix = node->GetMatrix();
    TGeoTranslation trans(*matrix);
    for (int i=0; i<3; i++) obj->Position[i] = trans.GetTranslation()[i];
    TGeoRotation mrot(*matrix);
    mrot.GetAngles(obj->Orientation[0], obj->Orientation[1], obj->Orientation[2]);
    //qDebug() << "xyz:"<<obj->Position[0]<< obj->Position[1]<< obj->Position[2]<< "phi,theta,psi:"<<obj->Orientation[0]<<obj->Orientation[1]<<obj->Orientation[2];

    //hosted nodes
    int totNodes = node->GetNdaughters();
    //qDebug() << "Number of hosted nodes:"<<totNodes;
    for (int i=0; i<totNodes; i++)
    {
        TGeoNode* daugtherNode = node->GetDaughter(i);
        QString name = daugtherNode->GetName();
        //qDebug() << i<< name;

        //not a PM
        AGeoObject* inObj = new AGeoObject(name);
        obj->addObjectLast(inObj);
        readGeoObjectTree(inObj, daugtherNode);
    }
}

QString AGeometryHub::importGDML(const QString & fileName)
{
    QString err = readGDMLtoTGeo(fileName.toLatin1());
    if (!err.isEmpty()) return err;

    const TGeoNode * top = GeoManager->GetTopNode();
    //ShowNodes(top, 0);

    AMaterialHub::getInstance().importMaterials(GeoManager->GetListOfMaterials());

    clearWorld();
    readGeoObjectTree(World, top);
    World->makeItWorld();
    AGeoBox * wb = dynamic_cast<AGeoBox*>(World->Shape);
    if (wb)
    {
        setWorldSizeXY( std::max(wb->dx, wb->dy) );
        setWorldSizeZ(wb->dz);
    }
    setWorldSizeFixed(wb);

    //GeoManager->FindNode(0,0,0); // need?

    return "";
}

QString AGeometryHub::importGeometry(const QString &fileName)
{
    delete GeoManager; GeoManager = nullptr;
    GeoManager = TGeoManager::Import(fileName.toLocal8Bit());

    if (!GeoManager || !GeoManager->IsClosed())
    {
        GeoManager = new TGeoManager();
        return "Load GDML failed!"; // needs rebuild!
    }
    qDebug() << "--> tmp GeoManager loaded from GDML file";

    const TGeoNode * top = GeoManager->GetTopNode();
    //ShowNodes(top, 0);

    AMaterialHub::getInstance().importMaterials(GeoManager->GetListOfMaterials());

    clearWorld();
    readGeoObjectTree(World, top);
    World->makeItWorld();
    AGeoBox * wb = dynamic_cast<AGeoBox*>(World->Shape);
    if (wb)
    {
        setWorldSizeXY( std::max(wb->dx, wb->dy) );
        setWorldSizeZ(wb->dz);
    }
    setWorldSizeFixed(wb);

    World->scaleRecursive(10.0);  // 1[cm] becomes 10[mm]

    return "";
}

QString AGeometryHub::generateStandaloneObjectName(const AGeoShape * shape) const
{
    assert(shape);

    int iCounter = 1;
    QString name;
    do
    {
        name = shape->getShortName() + QString::number(iCounter);
        ++iCounter;
    }
    while (World->isNameExists(name));

    return name;
}

QString AGeometryHub::generateObjectName(const QString & prefix) const
{
    int iCounter = 1;
    QString name;
    do
    {
        name = prefix + QString::number(iCounter);
        ++iCounter;
    }
    while (World->isNameExists(name));

    return name;
}

void AGeometryHub::removeNameDecorators(TString & name) const
{
    const int ind = name.Index(IndexSeparator, IndexSeparator.Length(), 0, TString::kExact);
    if (ind != TString::kNPOS) name.Resize(ind);
}

size_t AGeometryHub::countScintillators() const
{
    return Scintillators.size();
}

void AGeometryHub::getScintillatorPositions(std::vector<AVector3> & positions) const
{
    positions.resize(Scintillators.size());
    for (size_t i = 0; i < Scintillators.size(); i++)
    {
        positions[i] = {0,0,0};
        const TGeoNode * node = Scintillators[i].second;
        getGlobalPosition(node, positions[i]);
    }
}

AVector3 AGeometryHub::getScintillatorPosition(size_t index) const
{
    if (index >= Scintillators.size()) return {0,0,0};

    AVector3 position;
    const TGeoNode * node = Scintillators[index].second;
    getGlobalPosition(node, position);
    return position;
}

void AGeometryHub::getScintillatorOrientations(std::vector<AVector3> & orientations) const
{
    orientations.resize(Scintillators.size());
    for (size_t i = 0; i < Scintillators.size(); i++)
    {
        const TGeoNode * node = Scintillators[i].second;
        double uvX[3], uvY[3], uvZ[3];
        getGlobalUnitVectors(node, uvX, uvY, uvZ);
        orientations[i] = {uvX[0], uvX[1], uvX[2]};
    }
}

void AGeometryHub::getScintillatorVolumeNames(std::vector<QString> & vol) const
{
    vol.resize(Scintillators.size());
    for (size_t i = 0; i < Scintillators.size(); i++)
        vol[i] = Scintillators[i].first->Name;
}

void AGeometryHub::getScintillatorVolumeUniqueNames(std::vector<QString> & vol) const
{
    for (size_t i = 0; i < Scintillators.size(); i++)
    {
        const QString & name = Scintillators[i].first->Name;
        if (std::find(vol.begin(), vol.end(), name) == vol.end())
            vol.push_back(name);
    }
}

void AGeometryHub::checkGeometryCompatibleWithGeant4() const
{
    World->checkCompatibleWithGeant4();
}

size_t AGeometryHub::countParticleAnalyzers() const
{
    return ParticleAnalyzers.size();
}

QString AGeometryHub::checkVolumesExist(const std::vector<std::string> & VolumesAndWildcards) const
{
    if (VolumesAndWildcards.empty()) return ""; //can be empty

    QStringList NotFoundVolumes;
    TObjArray * va = GeoManager->GetListOfVolumes();
    const int numVol = va->GetEntries();

    for (const std::string & vw : VolumesAndWildcards)
    {
        QString s(vw.data());

        bool bWild = false;
        if (s.endsWith('*'))
        {
            s.chop(1);
            bWild = true;
        }

        bool bFound = false;
        for (int iV=0; iV<numVol; iV++)
        {
            const TString tname = ((TGeoVolume*)va->At(iV))->GetName();
            const QString sname(tname.Data());

            if (bWild)
            {
                if (sname.startsWith(s))
                {
                    bFound = true;
                    break;
                }
            }
            else
            {
                if (sname == s)
                {
                    bFound = true;
                    break;
                }
            }
        }
        if (!bFound) NotFoundVolumes << s;
    }

    if (NotFoundVolumes.isEmpty()) return "";
    else return QString("The following volumes/widlcards do not identify any volume in the geometry:\n%1").arg(NotFoundVolumes.join(", "));
}
