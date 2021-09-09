#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

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

        void ReadConfig(const std::string & ConfigFileName, const std::string & WorkingDir, int ID);

        void startSession();
        void terminateSession(const std::string & ReturnMessage); //calls exit()!
        void endSession();

        void runSimulation();

        void onRunFinished();
        bool isGuiMode() const {return bGuiMode;}
        const std::string & getGDML() const {return GDML;}
        const std::string & getPhysicsList() const {return PhysicsList;}
        const std::string & getEventId() const {return EventId;}
        void updateEventId();
        std::vector<ParticleRecord> & getNextEventPrimaries();
        bool isEndOfInputFileReached() const;
        const std::vector<std::string> & getListOfSensitiveVolumes() const {return SensitiveVolumes;}
        std::vector<MonitorSensitiveDetector*> & getMonitors() {return Monitors;}
        const std::map<std::string, double> & getStepLimitMap() const {return StepLimitMap;}
        int findParticle(const std::string & particleName);  // change to pointer search?
        int findMaterial(const std::string & materialName);  // change to pointer search?

        bool activateNeutronThermalScatteringPhysics();
        void updateMaterials(G4VPhysicalVolume * worldPV);

        void writeNewEventMarker();

        void saveDepoRecord(int iPart, int iMat, double edep, double * pos, double time);

        void saveTrackStart(int trackID, int parentTrackID,
                            const G4String & particleName,
                            const G4ThreeVector & pos, double time, double kinE,
                            int iMat, const std::string &volName, int volIndex);
        void saveTrackRecord(const std::string & procName,
                             const G4ThreeVector & pos, double time,
                             double kinE, double depoE,
                             const std::vector<int> *secondaries = nullptr,
                             int iMatTo = -1, const std::string &volNameTo = "", int volIndexTo = -1);

        void resetPredictedTrackID() {NextTrackID = 1;}
        void incrementPredictedTrackID() {NextTrackID++;}
        int  getPredictedTrackID() {return NextTrackID;}

        void findExitVolume();

        void saveParticle(const G4String & particle, double energy, double time, double * PosDir);

public:
        //runtime
        double DepoByRegistered = 0;
        double DepoByNotRegistered = 0;

        enum HistoryMode {NotCollecting, OnlyTracks, FullLog};
        HistoryMode CollectHistory = NotCollecting;
        int TracksToBuild = 0;

        int Precision    = 6;

        bool bMonitorsRequireSteppingAction = false;

        bool bStoppedOnMonitor = false; // bug fix for Geant4? used in (Monitor)SensitiveDetector and SteppingAction

        bool bExitParticles = false;
        G4LogicalVolume * ExitVolume = nullptr;
        bool   bExitTimeWindow = false;
        double ExitTimeFrom = 0;
        double ExitTimeTo = 1.0e6;
        bool   bExitKill = true;

private:
        void prepareParticleCollection();
        void prepareMonitors();
        void prepareInputStream();
        void prepareOutputDepoStream();
        void prepareOutputHistoryStream();
        void prepareOutputExitStream();
        void executeAdditionalCommands();
        void generateReceipt();
        void storeMonitorsData();

        G4ParticleDefinition * findGeant4Particle(const std::string & particleName);
        bool extractIonInfo(const std::string & text, int & Z, int & A, double & E);

    private:
        std::string FileName_Input;
        std::string FileName_Output;
        std::string FileName_Monitors;
        std::string FileName_Receipt;
        std::string FileName_Tracks;
        long Seed = 0;
        std::string EventId; //  "#number"
        std::string NextEventId;
        std::string GDML;
        std::string PhysicsList;
        bool        bUseThermalScatteringNeutronPhysics = false;
        std::vector<json11::Json> ParticleJsonArray;
        std::vector<G4ParticleDefinition*> ParticleCollection; // does not own
        std::map<std::string, int> ParticleMap;
        std::map<std::string, int> MaterialMap;
        std::vector<std::pair<std::string, std::string>> MaterialsToOverrideWithStandard;
        std::vector<std::string> SensitiveVolumes;
        std::vector<std::string> OnStartCommands;
        std::map<std::string, double> StepLimitMap;
        bool bG4antsPrimaries = false;
        bool bBinaryPrimaries = false;
        std::ifstream * inStreamPrimaries   = nullptr;
        std::ofstream * outStreamDeposition = nullptr;
        std::ofstream * outStreamHistory    = nullptr;
        std::ofstream * outStreamExit       = nullptr;
        std::vector<ParticleRecord> GeneratedPrimaries;
        bool bGuiMode = false;

        bool bExitBinary = false;
        bool bBinaryOutput = false;

        std::vector<MonitorSensitiveDetector*> Monitors; //can contain nullptr!

        std::string FileName_Exit;
        std::string ExitVolumeName;

        int EventsDone = 0;
        int NumEventsToDo = 0;
        double ProgressLastReported = 0;
        double ProgressInc = 1.0;

        int NextTrackID = 1;

        std::unordered_set<std::string> SeenNotRegisteredParticles;

        //to report back to ants2
        bool bError;
        std::string ErrorMessage;
        std::vector<std::string> WarningMessages;

        std::map<std::string, int> ElementToZ;
};

#endif // SESSIONMANAGER_H
