#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "aparticlesimsettings.h"

#include "json11.hh" //https://github.com/dropbox/json11

#include <string>
#include <vector>
#include <unordered_set>
#include <map>

#include "G4ThreeVector.hh"

class G4ParticleDefinition;
class G4StepPoint;
class MonitorSensitiveDetector;
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

        void ReadConfig(const std::string & workingDir, const std::string & ConfigFileName, int ID);

        void startSession();
        void terminateSession(const std::string & ReturnMessage); //calls exit()!
        void endSession();

        void runSimulation();

        void onEventFinished();
        const std::string & getEventId() const {return EventId;}
        std::vector<MonitorSensitiveDetector*> & getMonitors() {return Monitors;}
        const std::map<std::string, double> & getStepLimitMap() const {return StepLimitMap;}
        int findMaterial(const std::string & materialName);  // change to pointer search?

        bool activateNeutronThermalScatteringPhysics();
        void updateMaterials(G4VPhysicalVolume * worldPV);

        void writeNewEventMarker();

        void saveDepoRecord(const std::string & pName, int iMat, double edep, double * pos, double time);

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

        void saveParticle(const G4String & particle, double energy, double time, double * PosDir);

public:
        std::string          WorkingDir;
        AParticleSimSettings Settings;

        bool CollectHistory = false;

        int Precision    = 6;

        bool bMonitorsRequireSteppingAction = false;

        bool bStoppedOnMonitor = false; // bug fix for Geant4? used in (Monitor)SensitiveDetector and SteppingAction

        bool bExitParticles = false;
        G4LogicalVolume * ExitVolume = nullptr;
        bool   bExitTimeWindow = false;
        double ExitTimeFrom = 0;
        double ExitTimeTo = 1.0e6;
        bool   bExitKill = true;

        PrimaryGeneratorAction * PrimGenAction = nullptr;

        int CurrentEvent = 0;
        AParticleGun * ParticleGenerator = nullptr;

        std::vector<std::string> SensitiveVolumes;  // !!!*** not needed alias

        G4ParticleDefinition * findGeant4Particle(const std::string & particleName);  // !!!*** to separate class
private:
        void prepareParticleGun();
        void prepareMonitors();
        void prepareOutputDepoStream();
        void prepareOutputHistoryStream();
        void prepareOutputExitStream();
        void executeAdditionalCommands();
        void generateReceipt();
        void storeMonitorsData();

        bool extractIonInfo(const std::string & text, int & Z, int & A, double & E);

    private:
        std::string FileName_Input;
        std::string FileName_Output; // !!!*** rename
        std::string FileName_Monitors;
        std::string FileName_Tracks;  // !!!*** remove, it is Settings.RunSet.FileNameTrackingHistory
        std::string EventId; //  "#number"
        std::map<std::string, int> MaterialMap;
        std::map<std::string, double> StepLimitMap;
        bool bG4antsPrimaries = false;
        bool bBinaryPrimaries = false;
        std::ofstream * outStreamDeposition = nullptr;
        std::ofstream * outStreamHistory    = nullptr;
        std::ofstream * outStreamExit       = nullptr;

        bool bExitBinary = false;
        bool bBinaryOutput = false;

        std::vector<MonitorSensitiveDetector*> Monitors; //can contain nullptr!

        std::string FileName_Exit;
        std::string ExitVolumeName;

        //to report back to ants2
        bool bError;
        std::string ErrorMessage;
        std::vector<std::string> WarningMessages;

        std::map<std::string, int> ElementToZ;
};

#endif // SESSIONMANAGER_H
