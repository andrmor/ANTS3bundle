#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "aparticlesimsettings.h"
#include "aanalyzeruniqueinstance.h"

#include "json11.hh" //https://github.com/dropbox/json11

#include <string>
#include <vector>
#include <unordered_set>
#include <map>

#include "G4ThreeVector.hh"

class G4ParticleDefinition;
class G4StepPoint;
class MonitorSensitiveDetectorWrapper;
class CalorimeterSensitiveDetectorWrapper;
class AnalyzerSensitiveDetector;
class G4LogicalVolume;
class G4VPhysicalVolume;
class AParticleGun;
class PrimaryGeneratorAction;

struct ParticleRecord
{
    G4ParticleDefinition * Particle = 0;
    G4double Energy = 0;
    G4ThreeVector Position  = {0, 0, 0};
    G4ThreeVector Direction = {0, 0, 0};
    G4double Time = 0;
};

struct TmpDepositionBuffer
{
    std::string Name;
    int    iMat;
    double EDep;
    double Pos[3];
    double Time;
    int    iCopyNumber;
};

class SessionManager
{
    public:
        static SessionManager& getInstance();

    private:
        SessionManager();
        ~SessionManager();

    public:
        SessionManager(SessionManager const&) = delete;
        void operator=(SessionManager const&) = delete;

        void readConfig(const std::string & workingDir, const std::string & ConfigFileName, int ID);

        void startSession();
        void terminateSession(const std::string & ReturnMessage); //calls exit()!
        void endSession();

        void runSimulation();

        void onEventFinished();
        int  findMaterial(const std::string & materialName);  // change to pointer search?

        bool activateNeutronThermalScatteringPhysics();
        void updateMaterials();

        void writeNewEventMarker();

        void saveDepoRecord(const std::string & pName, int iMat, double edep, double * pos, double time, int copyNumber);

        void saveTrackStart(int trackID, int parentTrackID,
                            const G4String & particleName,
                            const G4ThreeVector & pos, double time, double kinE,
                            int iMat, const std::string &volName, int volIndex);
        void saveTrackRecord(const std::string & procName,
                             const G4ThreeVector & pos, double time,
                             double kinE, double depoE,
                             const std::vector<int> *secondaries = nullptr,
                             int iMatTo = -1, const std::string &volNameTo = "", int volIndexTo = -1);

        void resetPredictedTrackID();
        void incrementPredictedTrackID();
        int  getPredictedTrackID() const;

        void findExitVolume();
        G4ParticleDefinition * findGeant4Particle(const std::string & particleName);

        void saveParticle(const G4String & particle, double energy, double time, double * PosDir);

        bool isEnergyDepoLogger(G4LogicalVolume * vol);

public:
        std::string          WorkingDir;
        AParticleSimSettings Settings;

        bool   bMonitorsRequireSteppingAction = false;
        bool   bStoppedOnMonitor = false; // bug fix for Geant4? used in (Monitor)SensitiveDetector and SteppingAction

        PrimaryGeneratorAction * PrimGenAction     = nullptr;
        G4LogicalVolume        * ExitVolume        = nullptr;
        AParticleGun           * ParticleGenerator = nullptr;

        G4VPhysicalVolume * WorldPV = nullptr;

        int CurrentEvent = 0;

        std::vector<MonitorSensitiveDetectorWrapper*>     Monitors;      // can contain nullptr!
        std::vector<CalorimeterSensitiveDetectorWrapper*> Calorimeters;  // can contain nullptr!
        std::vector<AAnalyzerUniqueInstance>              Analyzers;

        const G4String DepoLoggerSDName = "SD_Depo";

        std::vector<TmpDepositionBuffer> DirectDepositionBuffer;

private:
        void prepareParticleGun();
        void prepareMonitors();
        void prepareAnalyzers();
        void prepareOutputDepoStream();
        void prepareOutputHistoryStream();
        void prepareOutputExitStream();
        void executeAdditionalCommands();
        void generateReceipt();
        void storeMonitorsData();
        void storeCalorimeterData();
        void storeAnalyzerData();
        bool extractIonInfo(const std::string & text, int & Z, int & A, double & E);
        void replaceMatNameInMatLimitedSources(const G4String & name, const G4String & G4Name);

    private:
        std::ofstream * outStreamDeposition = nullptr;
        std::ofstream * outStreamHistory    = nullptr;
        std::ofstream * outStreamExit       = nullptr;

        std::map<std::string, int> ElementToZ;
        std::map<std::string, int> MaterialMap;

        bool bExitBinary = false;
        bool bBinaryOutput = false;

        //to report back to ants2
        bool bError;
        std::string ErrorMessage;
};

#endif // SESSIONMANAGER_H
