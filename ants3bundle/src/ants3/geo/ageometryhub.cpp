#include "ageometryhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ageotype.h"
//#include "agridelementrecord.h"
#include "ageoconsts.h"
#include "amaterialhub.h"
#include "ainterfacerulehub.h"
#include "ageospecial.h"
#include "ajsontools.h"
#include "asensorhub.h"
#include "amonitorhub.h"

#include <QDebug>

#include "TGeoManager.h"
#include "TVector3.h"

AGeometryHub & AGeometryHub::getInstance()
{
    static AGeometryHub instance;
    return instance;
}

AGeometryHub::AGeometryHub()
{
    World = new AGeoObject("World");
    delete World->Type; World->Type = new ATypeWorldObject();
    World->Material = 0;
    World->Container = nullptr;

    Prototypes = new AGeoObject("_#_PrototypeContainer_#_");
    delete Prototypes->Type; Prototypes->Type = new ATypePrototypeCollectionObject();
    Prototypes->migrateTo(World);
}

AGeometryHub::~AGeometryHub()
{
    //qDebug() << "Dest for A3Geometry";
    clearWorld(); delete World;

    delete GeoManager; // should be deleted by aboutToQuit()!
}

void AGeometryHub::clearWorld()
{
    //delete all but World and Prototypes.
    //the Prototypes object should have the same pointer

    World->removeHostedObject(Prototypes);

    for (AGeoObject * obj : World->HostedObjects) obj->clearAll();
    World->HostedObjects.clear();

    Prototypes->clearContent();
    World->HostedObjects.push_back(Prototypes);

    clearGridRecords();
    AMonitorHub::getInstance().clear();
}

void AGeometryHub::clearGridRecords()
{
    /*
    for (auto * gr : GridRecords) delete gr;
    GridRecords.clear();
*/
}

bool AGeometryHub::canBeDeleted(AGeoObject * obj) const
{
    if (obj == World) return false;
    if (obj == Prototypes) return false;

    if (obj->isInUseByComposite()) return false;
    if (obj->Type->isPrototype() && World->isPrototypeInUseRecursive(obj->Name, nullptr)) return false;

    return true;
}

void AGeometryHub::convertObjToComposite(AGeoObject *obj)
{
    delete obj->Type;
    ATypeCompositeObject* CO = new ATypeCompositeObject();
    obj->Type = CO;

    AGeoObject* logicals = new AGeoObject();
    logicals->Name = "CompositeSet_"+obj->Name;
    delete logicals->Type;
    logicals->Type = new ATypeCompositeContainerObject();
    obj->addObjectFirst(logicals);

    AGeoObject* first = new AGeoObject();
    while (World->isNameExists(first->Name))
        first->AGeoObject::GenerateRandomObjectName();
    first->Shape = obj->Shape;
    first->Material = obj->Material;
    logicals->addObjectLast(first);

    AGeoObject* second = new AGeoObject();
    while (World->isNameExists(second->Name))
        second->AGeoObject::GenerateRandomObjectName();
    second->Material = obj->Material;
    second->Position[0] = 15;
    second->Position[1] = 15;
    logicals->addObjectLast(second);

    QStringList sl;
    sl << first->Name << second->Name;
    QString str = "TGeoCompositeShape( " + first->Name + " + " + second->Name + " )";
    obj->Shape = new AGeoComposite(sl, str);
}

QString AGeometryHub::convertToNewPrototype(std::vector<AGeoObject *> members)
{
    QString errStr;

    for (AGeoObject * obj : members)
    {
        bool ok = obj->isPossiblePrototype(&errStr);
        if (!ok) return errStr;
    }

    int index = 0;
    QString name;
    do name = QString("Prototype_%1").arg(index++);
    while (World->isNameExists(name));

    AGeoObject * proto = new AGeoObject(name);
    delete proto->Type; proto->Type = new ATypePrototypeObject();
    proto->migrateTo(Prototypes);

    for (AGeoObject * obj : members)
        obj->migrateTo(proto);

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

/*
void AGeometry::convertObjToGrid(AGeoObject *obj)
{
    delete obj->Type;
    obj->Type = new ATypeGridObject();
    QString name = obj->Name;

    //grid element inside
    AGeoObject* elObj = new AGeoObject();
    delete elObj->Type;
    ATypeGridElementObject* GE = new ATypeGridElementObject();
    elObj->Type = GE;
    GE->dz = obj->Shape->getHeight();
    if (GE->dz == 0) GE->dz = 1.001;
    elObj->Name = "GridElement_" + name;
    elObj->color = 1;
    obj->addObjectFirst(elObj);
    elObj->updateGridElementShape();
}

void AGeometry::shapeGrid(AGeoObject *obj, int shape, double p0, double p1, double p2, int wireMat)
{
    //qDebug() << "Grid shape request:"<<shape<<p0<<p1<<p2;
    if (!obj) return;

    if (!obj->Type->isGrid()) return;
    AGeoObject* GEobj = obj->getGridElement();
    if (!GEobj) return;
    ATypeGridElementObject* GE = static_cast<ATypeGridElementObject*>(GEobj->Type);

    //clear anything which is inside grid element
    for (int i=GEobj->HostedObjects.size()-1; i>-1; i--)
    {
        //qDebug() << "...."<< GEobj->HostedObjects[i]->Name;
        GEobj->HostedObjects[i]->clearAll();
    }
    GEobj->HostedObjects.clear();

    obj->Shape->  setHeight(0.5*p2 + 0.001);
    GE->shape = shape;
    GE->dz = 0.5*p2 + 0.001;
    GE->size1 = 0.5*p0;
    GE->size2 = 0.5*p1;

    switch (shape)
    {
    case 0: // parallel wires
    {
        AGeoObject* w = new AGeoObject(new ATypeSingleObject(), new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, 90, 270));
        w->Position[0] = 0.5*p0;
        w->Orientation[1] = 90;
        w->Material = wireMat;
        do w->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w->Name));
        GEobj->addObjectFirst(w);
        w = new AGeoObject(new ATypeSingleObject(), new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, -90, 90));
        w->Position[0] = -0.5*p0;
        w->Orientation[1] = 90;
        w->Material = wireMat;
        do w->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w->Name));
        GEobj->addObjectFirst(w);
        break;
    }
    case 1: // mesh
    {
        AGeoObject* com = new AGeoObject();
        do com->Name = AGeoObject::GenerateRandomCompositeName();
        while (World->isNameExists(com->Name));
        convertObjToComposite(com);
        com->Material = wireMat;
        AGeoObject* logicals = com->getContainerWithLogical();
        for (int i=0; i<logicals->HostedObjects.size(); i++)
            delete logicals->HostedObjects[i];
        logicals->HostedObjects.clear();

        AGeoObject* w1 = new AGeoObject();
        do w1->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w1->Name));
        delete w1->Shape;
        w1->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, -90, 90);
        w1->Position[0] = -0.5*p0;
        w1->Orientation[1] = 90;
        w1->Material = wireMat;
        logicals->addObjectFirst(w1);

        AGeoObject* w2 = new AGeoObject();
        do w2->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w2->Name));
        delete w2->Shape;
        w2->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p1, 90, 270);
        w2->Position[0] = 0.5*p0;
        w2->Orientation[1] = 90;
        w2->Material = wireMat;
        logicals->addObjectLast(w2);

        AGeoObject* w3 = new AGeoObject();
        do w3->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w3->Name));
        delete w3->Shape;
        w3->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p0, 90, 270);
        w3->Position[1] = 0.5*p1;
        w3->Orientation[0] = 90;
        w3->Orientation[1] = 90;
        w3->Material = wireMat;
        logicals->addObjectLast(w3);

        AGeoObject* w4 = new AGeoObject();
        do w4->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w4->Name));
        delete w4->Shape;
        w4->Shape = new AGeoTubeSeg(0, 0.5*p2, 0.5*p0, -90, 90);
        w4->Position[1] = -0.5*p1;
        w4->Orientation[0] = 90;
        w4->Orientation[1] = 90;
        w4->Material = wireMat;
        logicals->addObjectLast(w4);

        AGeoComposite* comSh = static_cast<AGeoComposite*>(com->Shape);
        comSh->GenerationString = "TGeoCompositeShape( " +
                w1->Name + " + " +
                w2->Name + " + " +
                w3->Name + " + " +
                w4->Name + " )";

        GEobj->addObjectFirst(com);
        break;
    }
    case 2: //hexagon
    {
        AGeoObject* com = new AGeoObject();
        do com->Name = AGeoObject::GenerateRandomCompositeName();
        while (World->isNameExists(com->Name));
        convertObjToComposite(com);
        com->Material = wireMat;
        AGeoObject* logicals = com->getContainerWithLogical();
        for (int i=0; i<logicals->HostedObjects.size(); i++)
            delete logicals->HostedObjects[i];
        logicals->HostedObjects.clear();

        //qDebug() << "p0, p1, p2"<<p0<<p1<<p2;
        double d = 0.5*p0/0.86603; //radius of circumscribed circle
        double delta = 0.5*(p0-p1); // wall thickness
        //qDebug() << "d, delta:"<<d << delta;
        double dd = d + 2.0*delta*0.57735;
        double x = 0.5*(p0-delta)*0.866025;

        AGeoObject* w1 = new AGeoObject();
        do w1->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w1->Name));
        delete w1->Shape;
        w1->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w1->Position[1] = 0.5*p0 - 0.5*delta;
        w1->Orientation[1] = 90;
        w1->Material = wireMat;
        logicals->addObjectFirst(w1);

        AGeoObject* w2 = new AGeoObject();
        do w2->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w2->Name));
        delete w2->Shape;
        w2->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w2->Position[1] = -0.5*p0 + 0.5*delta;
        w2->Orientation[0] = 180;
        w2->Orientation[1] = 90;
        w2->Material = wireMat;
        logicals->addObjectFirst(w2);

        AGeoObject* w3 = new AGeoObject();
        do w3->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w3->Name));
        delete w3->Shape;
        w3->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w3->Position[0] = -x;
        w3->Position[1] = 0.5*(0.5*p0 - 0.5*delta);
        w3->Orientation[0] = 60;
        w3->Orientation[1] = 90;
        w3->Material = wireMat;
        logicals->addObjectFirst(w3);

        AGeoObject* w4 = new AGeoObject();
        do w4->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w4->Name));
        delete w4->Shape;
        w4->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w4->Position[0] = x;
        w4->Position[1] = 0.5*(0.5*p0 - 0.5*delta);
        w4->Orientation[0] = -60;
        w4->Orientation[1] = 90;
        w4->Material = wireMat;
        logicals->addObjectFirst(w4);

        AGeoObject* w5 = new AGeoObject();
        do w5->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w5->Name));
        delete w5->Shape;
        w5->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w5->Position[0] = -x;
        w5->Position[1] = -0.5*(0.5*p0 - 0.5*delta);
        w5->Orientation[0] = 120;
        w5->Orientation[1] = 90;
        w5->Material = wireMat;
        logicals->addObjectFirst(w5);

        AGeoObject* w6 = new AGeoObject();
        do w6->Name = AGeoObject::GenerateRandomObjectName();
        while (World->isNameExists(w6->Name));
        delete w6->Shape;
        w6->Shape = new AGeoTrd1( 0.5*d, 0.5*dd, 0.5*p2, 0.5*delta );
        w6->Position[0] = x;
        w6->Position[1] = -0.5*(0.5*p0 - 0.5*delta);
        w6->Orientation[0] = -120;
        w6->Orientation[1] = 90;
        w6->Material = wireMat;
        logicals->addObjectFirst(w6);

        AGeoComposite* comSh = static_cast<AGeoComposite*>(com->Shape);
        comSh->GenerationString = "TGeoCompositeShape( " +
                w1->Name + " + " +
                w2->Name + " + " +
                w3->Name + " + " +
                w4->Name + " + " +
                w5->Name + " + " +
                w6->Name + " )";

        GEobj->addObjectFirst(com);
        break;
    }

    default:
        qWarning() << "Unknown grid element shape!";
        return;
    }
    GEobj->updateGridElementShape();
}
*/

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
        qWarning()<< "Composite object: Not found container with logical objects!";
        return false;
    }
    AGeoComposite * cs = dynamic_cast<AGeoComposite*>(obj->Shape);
    if (!cs)
    {
        AGeoScaledShape * scaled = dynamic_cast<AGeoScaledShape*>(obj->Shape);
        if (!scaled)
        {
            qWarning()<< "Composite: Shape object is not composite nor scaled composite!!";
            return false;
        }
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

void AGeometryHub::populateGeoManager()
{
    ASensorHub::getInstance().SensorData.clear();
    AMonitorHub::getInstance().clear();
    clearGridRecords();

    World->introduceGeoConstValuesRecursive();
    World->updateAllStacks();
    expandPrototypeInstances();

    delete GeoManager; GeoManager = new TGeoManager();
    GeoManager->SetVerboseLevel(0);

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
    MatHub.generateGeoMedia();

    Top = GeoManager->MakeBox("WorldBox", MatHub[World->Material]->GeoMed, WorldSizeXY, WorldSizeXY, WorldSizeZ);
    GeoManager->SetTopVolume(Top);
    GeoManager->SetTopVisible(true);

    AInterfaceRuleHub::getInstance().updateVolumesFromTo();
    addTGeoVolumeRecursively(World, Top);

    Top->SetName("World"); // "WorldBox" above is needed - JSROOT uses that name to avoid conflicts

    GeoManager->CloseGeometry();
}

#include "amonitor.h"
void AGeometryHub::addMonitorNode(AGeoObject * obj, TGeoVolume * vol, TGeoVolume * parent, TGeoCombiTrans * lTrans)
{
    AMonitorHub & MonitorHub = AMonitorHub::getInstance();
    const int MonitorCounter = MonitorHub.countMonitors();

    (static_cast<ATypeMonitorObject*>(obj->Type))->index = MonitorCounter;
    parent->AddNode(vol, MonitorCounter, lTrans);

    TString fixedName = vol->GetName();
    fixedName += "_-_";
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

    MonitorHub.Monitors.push_back(md);
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

    SensorHub.SensorData.push_back(sr);
}

bool AGeometryHub::findMotherNodeFor(const TGeoNode * node, const TGeoNode * startNode, const TGeoNode* & foundNode)
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

void AGeometryHub::findMotherNode(const TGeoNode * node, const TGeoNode* & motherNode)
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

void AGeometryHub::getGlobalPosition(const TGeoNode * node, AVector3 & position)
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

void AGeometryHub::addTGeoVolumeRecursively(AGeoObject * obj, TGeoVolume * parent, int forcedNodeNumber)
{
    if (!obj->fActive) return;

    TGeoVolume     * vol    = nullptr;
    TGeoCombiTrans * lTrans = nullptr;

    if      (obj->Type->isWorld())
        vol = parent; // resuse the cycle by HostedVolumes below
    else if (obj->Type->isPrototypes() || obj->isCompositeMemeber() || obj->Type->isCompositeContainer())
        return;       // logicals do not host anything to be added to the geometry
    else if (obj->Type->isHandlingSet() || obj->Type->isHandlingArray() || obj->Type->isInstance())
        vol = parent; // group objects are pure virtual, pass the volume of the parent
    else
    {
        if (obj->Type->isMonitor())
        {
            if (obj->Container) obj->Material = obj->Container->getMaterial();
            else
            {
                qWarning() << "Error: Monitor without container" << obj->Name;
                return;
            }
        }

        if (obj->Type->isComposite())
        {
            bool ok = processCompositeObject(obj);
            if (!ok) return;
        }

        AMaterialHub & MatHub = AMaterialHub::getInstance();
        vol = new TGeoVolume(obj->Name.toLocal8Bit().data(), obj->Shape->createGeoShape(), MatHub[obj->Material]->GeoMed);

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
            //            GridRecords.append(obj->createGridRecord());
            //            parent->AddNode(vol, GridRecords.size() - 1, lTrans);
        }
        else if (obj->Type->isMonitor())
            addMonitorNode(obj, vol, parent, lTrans);
        else if (obj->Role && obj->Role->getType() == "Sensor")
            addSensorNode(obj, vol, parent, lTrans);
        else
            parent->AddNode(vol, forcedNodeNumber, lTrans);
    }

    // Position hosted objects
    if      (obj->Type->isHandlingArray())
        positionArray(obj, vol);
    else if (obj->Type->isStack())
        positionStack(obj, vol, forcedNodeNumber);
    else if (obj->Type->isInstance())
        positionInstance(obj, vol, forcedNodeNumber);
    else
        for (AGeoObject * el : obj->HostedObjects)
            addTGeoVolumeRecursively(el, vol, forcedNodeNumber);

    setVolumeTitle(obj, vol);
}

void AGeometryHub::setVolumeTitle(AGeoObject * obj, TGeoVolume * vol)
{
    //  Photon tracer uses volume title for identification of special volumes
    //  First character can be 'M' for monitor, 'S' for light sensor, 'G' for optical grid
    TString title = "----";
    if      (obj->Role)
    {
         if (obj->Role->getType() == "Sensor") title[0] = 'S';
    }
    else if (obj->Type->isMonitor())           title[0] = 'M';
    else if (obj->Type->isGrid())              title[0] = 'G';

    const AInterfaceRuleHub & IRH = AInterfaceRuleHub::getConstInstance();
    if (IRH.isFromVolume(vol->GetName())) title[1] = '*';
    if (IRH.isToVolume(vol->GetName()))   title[2] = '*';

    vol->SetTitle(title);
}

void AGeometryHub::positionArray(AGeoObject * obj, TGeoVolume * vol)
{
    ATypeArrayObject * array = static_cast<ATypeArrayObject*>(obj->Type);

    ATypeCircularArrayObject  * circArray = dynamic_cast<ATypeCircularArrayObject*>(obj->Type);
    ATypeHexagonalArrayObject * hexArray  = dynamic_cast<ATypeHexagonalArrayObject*>(obj->Type);

    for (AGeoObject * el : obj->HostedObjects)
    {
        int iCounter = array->startIndex;
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
                    positionHexArrayRing(iR, el, obj, vol, iCounter++);
            }
            else
            {
                for (int iy = 0; iy < hexArray->NumY; iy++)
                {
                    bool bOdd = ( (iy+1) % 2 == 0);
                    for (int ix = 0; ix < hexArray->NumX; ix++)
                    {
                        if (hexArray->SkipOddLast && bOdd && ix == hexArray->NumX-1) continue;

                        double x = el->Position[0] + (ix - 0.5*(hexArray->NumX - 1)) * hexArray->Step;
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
    for (const AGeoObject * el : qAsConst(obj->HostedObjects))
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

    //Position
    double local[3], master[3];
    local[0] = el->Position[0] + (ix - 0.5*(array->numX - 1)) * array->stepX;
    local[1] = el->Position[1] + (iy - 0.5*(array->numY - 1)) * array->stepY;
    local[2] = el->Position[2] + (iz - 0.5*(array->numZ - 1)) * array->stepZ;
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

void AGeometryHub::positionHexArrayRing(int iR, AGeoObject *el, AGeoObject *arrayObj, TGeoVolume *parent, int arrayIndex)
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
        if (!obj->Type->isPrototypes()) continue;
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
            if      (name.startsWith("PM"))  vol->SetLineColor(kGreen);
            else
            {
                const AGeoObject * obj = World->findObjectByName(name); // !*! can be very slow for large detectors!
                if (!obj && !name.isEmpty())
                {
                    //special for monitors
                    QString mName = name.split("_-_").at(0);
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
