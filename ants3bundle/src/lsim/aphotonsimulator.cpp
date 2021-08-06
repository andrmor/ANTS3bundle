#include "aphotonsimulator.h"
#include "alogger.h"
#include "ajsontools.h"
#include "amaterialhub.h"
#include "ageometryhub.h"
#include "asensorhub.h"
#include "aphotonsimhub.h"
#include "anoderecord.h"
#include "aoneevent.h"

#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

#include <iostream>

APhotonSimulator::APhotonSimulator(const QString & fileName, const QString & dir, int id) :
    ConfigFN(fileName), WorkingDir(dir), ID(id),
    SimSet(APhotonSimHub::getConstInstance().Settings) {}

void APhotonSimulator::start()
{
    ALogger::getInstance().open(QString("PhotonSimLog-%0.log").arg(ID));
    LOG << "Working Dir: " << WorkingDir << "\n";
    LOG << "Config file: " << ConfigFN   << "\n";

    QJsonObject json;
    jstools::loadJsonFromFile(json, ConfigFN);

    QString Error = AMaterialHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded materials: " << AMaterialHub::getInstance().countMaterials() << '\n';
    LOG.flush();
    Error         = AGeometryHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Geometry loaded\n";
    LOG << "World: " << AGeometryHub::getInstance().World << "\n";
    LOG << "GeoManager: " << AGeometryHub::getInstance().GeoManager << "\n";
    LOG.flush();
    Error         = ASensorHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sensor models: " << ASensorHub::getInstance().countSensorModels();
    LOG.flush();
    Error = APhotonSimHub::getInstance().readFromJson(json);
    if (!Error.isEmpty()) terminate(Error);
    LOG << "Loaded sim  settings. Simulation type: " << (int)SimSet.SimType << "\n";
    LOG.flush();

    Event = new AOneEvent();


//    QFile fileIn (FileDir + '/' + inFN);  fileIn .open(QIODevice::ReadOnly);
//    QFile fileOut(FileDir + '/' + outFN); fileOut.open(QIODevice::WriteOnly);
//    QTextStream in(&fileIn);
//    QTextStream out(&fileOut);

//    QTimer Timer;
//    QObject::connect(&Timer, &QTimer::timeout, this, &A3PSim::onProgressTimer);
//    Timer.start(300);

//    EventsProcessed = 0;
//    while (!in.atEnd())
//    {
//          QString line = in.readLine();
//          line.replace(from, to);
//          out << line << '\n';
//          EventsProcessed++;
//          QThread::msleep(1000); //just to simulate long execution!
//          qApp->processEvents();
//    }
//    fileIn.close();
//    fileOut.close();

    QCoreApplication::exit();
}

/*
void APhotonSimulator::simulateOneNode(const ANodeRecord & node)
{
    const ANodeRecord * thisNode = &node;
    std::unique_ptr<ANodeRecord> outNode(ANodeRecord::createS(1e10, 1e10, 1e10)); // if outside will use this instead of thisNode

    const int numPoints = 1 + thisNode->getNumberOfLinkedNodes();
    Event->clearHits();
    AScanRecord * sr = new AScanRecord();
    sr->Points.Reinitialize(numPoints);

    for (int iPoint = 0; iPoint < numPoints; iPoint++)
    {
        const bool bInside = !(bLimitToVolume && !isInsideLimitingObject(thisNode->R));
        if (bInside)
        {
            for (int i=0; i<3; i++) sr->Points[iPoint].r[i] = thisNode->R[i];
            sr->Points[iPoint].energy = (thisNode->NumPhot == -1 ? getNumPhotToRun() : thisNode->NumPhot);
        }
        else
        {
            for (int i=0; i<3; i++) sr->Points[iPoint].r[i] =  outNode->R[i];
            sr->Points[iPoint].energy = 0;
        }

        generateAndTracePhotons(sr, thisNode->Time, iPoint);

        //if exists, continue to work with the linked node(s)
        thisNode = thisNode->getLinkedNode();
        if (!thisNode) break; //paranoic
    }

    Event->HitsToSignal();

//    saveEvent();
//    saveTrue();
}

#include "TGeoNavigator.h"
#include "TGeoManager.h"
#include "TVector3.h"
void APhotonSimulator::generateAndTracePhotons(AScanRecord *scs, double time0, int iPoint)
{
    TGeoNavigator * navigator = AGeometryHub::getInstance().GeoManager->GetCurrentNavigator();

    Photon.r[0] = scs->Points[iPoint].r[0];
    Photon.r[1] = scs->Points[iPoint].r[1];
    Photon.r[2] = scs->Points[iPoint].r[2];
    Photon.scint_type = 0;
    Photon.time = time0;

    for (int i=0; i<scs->Points[iPoint].energy; i++)
    {
        //photon direction
        if (bIsotropic) photonGenerator->GenerateDirection(&Photon);
        else if (bCone)
        {
            double z = CosConeAngle + RandGen->Rndm() * (1.0 - CosConeAngle);
            double tmp = sqrt(1.0 - z*z);
            double phi = RandGen->Rndm()*3.1415926535*2.0;
            TVector3 K1(tmp*cos(phi), tmp*sin(phi), z);
            TVector3 Coll(ConeDir);
            K1.RotateUz(Coll);
            Photon.v[0] = K1[0];
            Photon.v[1] = K1[1];
            Photon.v[2] = K1[2];
        }
        //else it is already set

        int thisMatIndex;
        TGeoNode* node = navigator->FindNode(Photon.r[0], Photon.r[1], Photon.r[2]);
        if (node) thisMatIndex = node->GetVolume()->GetMaterial()->GetIndex();
        else
        {
            thisMatIndex = detector.top->GetMaterial()->GetIndex(); //get material of the world
            qWarning() << "Node not found when generating photons, using material of the world";
        }

        if (!PhotSimSettings.FixedPhotSettings.bFixWave)
            photonGenerator->GenerateWave(&Photon, thisMatIndex);//if directly given wavelength -> waveindex is already set in PhotonOnStart

        photonGenerator->GenerateTime(&Photon, thisMatIndex);

        if (PhotSimSettings.SpatialDistSettings.bEnabled)
            InNodeDistributor.apply(Photon, scs->Points[iPoint].r, RandGen->Rndm());

        Photon.SimStat = Event->SimStat;

        photonTracker->TracePhoton(&Photon);
    }
}
*/

void APhotonSimulator::onProgressTimer()
{
    std::cout << "$$>" << EventsProcessed << "<$$\n";
    std::cout.flush();
    LOG << EventsProcessed << '\n';
}

void APhotonSimulator::terminate(const QString & reason)
{
    LOG << reason;
    exit(1);
}
