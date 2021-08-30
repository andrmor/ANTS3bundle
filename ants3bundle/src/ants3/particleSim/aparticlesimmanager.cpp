#include "aparticlesimmanager.h"
#include "aparticlesimhub.h"
#include "aparticlesimsettings.h"
#include "asourceparticlegenerator.h"
#include "ageometryhub.h"

#include <QDebug>

AParticleSimManager & AParticleSimManager::getInstance()
{
    static AParticleSimManager instance;
    return instance;
}

AParticleSimManager::AParticleSimManager() :
    SimSet(AParticleSimHub::getInstance().Settings),
    Geometry(AGeometryHub::getConstInstance())
{
    Generator_Sources = new ASourceParticleGenerator();
}

AParticleSimManager::~AParticleSimManager()
{
    delete Generator_Sources;
}

#include "a3farmnoderecord.h"
#include "a3workdistrconfig.h"
#include "adispatcherinterface.h"
#include "a3global.h"
bool AParticleSimManager::simulate(int numLocalProc)
{
    qDebug() << "Particle sim triggered";
    ErrorString.clear();

    checkG4Settings();
    checkDirectories();
    addErrorLine(Geometry.checkVolumesExist(SimSet.G4Set.SensitiveVolumes));

    if (!ErrorString.isEmpty()) return false;



//    removeOutputFiles();  // note that output files in exchange dir will be deleted in adispatcherinterface

    int numEvents = 0;
    /*
    switch (SimSet.SimType)
    {
    case EPhotSimType::PhotonBombs :
        switch (SimSet.BombSet.GenerationMode)
        {
        case EBombGen::Single :
            numEvents = 1;
            break;
        case EBombGen::Flood :
            numEvents = SimSet.BombSet.FloodSettings.Number;
            break;
        default:
            ErrorString = "This bomb generation mode is not implemented yet!";
            return false;
        }
        break;
    case EPhotSimType::FromLRFs :
        {
            // TODO direct calculation here! !!!***
            ErrorString = "This simulation mode is not implemented yet!";
            return false;
        }
    default:
        ErrorString = "This simulation mode is not implemented yet!";
        return false;
    }
    */

    if (numEvents == 0)
    {
        ErrorString = "Nothing to simulate!";
        return false;
    }

    // configure number of local/remote processes to run
    std::vector<A3FarmNodeRecord> RunPlan;
    ADispatcherInterface & Dispatcher = ADispatcherInterface::getInstance();
    QString err = Dispatcher.fillRunPlan(RunPlan, numEvents, numLocalProc);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }

    A3WorkDistrConfig Request;
    Request.NumEvents = numEvents;

    bool ok = configureSimulation(RunPlan, Request);
    if (!ok) return false;

    qDebug() << "Running simulation...";
    QJsonObject Reply = Dispatcher.performTask(Request);

//    processReply(Reply);

//    if (ErrorString.isEmpty()) mergeOutput();

    qDebug() << "Particle simulation finished";
    return ErrorString.isEmpty();
}

#include "amaterialhub.h"
void AParticleSimManager::checkG4Settings()
{
    // TODO: no sensitive volumes but photon sim scheduled

    // TODO: optical grids will not be expanded

    AMaterialHub::getInstance().checkReadyForGeant4Sim(ErrorString);
}

bool AParticleSimManager::configureSimulation(const std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request)
{
    Request.Command = "lsim";

    const QString ExchangeDir = A3Global::getConstInstance().ExchangeDir;

    const QString LocalGdmlName = ExchangeDir + '/' +SimSet.RunSet.getGdmlFileName();
    QString err = Geometry.exportToGDML(LocalGdmlName);
    if (!err.isEmpty())
    {
        ErrorString = err;
        return false;
    }
    Request.CommonFiles.push_back(LocalGdmlName);

    // ...

    return true;
}

// ---

void AParticleSimManager::addErrorLine(const QString &error)
{
    if (error.isEmpty()) return;

    if (ErrorString.isEmpty()) ErrorString = error;
    else                       ErrorString += QString("\n%0").arg(error);
}

#include "a3global.h"
#include <QDir>
void AParticleSimManager::checkDirectories()
{
    if (SimSet.RunSet.OutputDirectory.isEmpty())       addErrorLine("Output directory is not set!");
    if (!QDir(SimSet.RunSet.OutputDirectory).exists()) addErrorLine("Output directory does not exist!");

    addErrorLine(A3Global::getInstance().checkExchangeDir());
}