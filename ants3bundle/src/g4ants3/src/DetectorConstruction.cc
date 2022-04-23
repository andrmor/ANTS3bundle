#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"
#include "SessionManager.hh"

#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserLimits.hh"

DetectorConstruction::DetectorConstruction(G4VPhysicalVolume *setWorld)
    : G4VUserDetectorConstruction(), fWorld(setWorld) {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    setStepLimiters();
    return fWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    // ---- Energy depositions in sensitive volumes -----
    SessionManager & SM = SessionManager::getInstance();

    SensitiveDetector* pSD = new SensitiveDetector(SM.DepoLoggerSDName);
    G4SDManager::GetSDMpointer()->AddNewDetector(pSD);

    G4LogicalVolumeStore* store = G4LogicalVolumeStore::GetInstance();

    const std::vector<std::string> & SVlist = SM.Settings.G4Set.SensitiveVolumes;
    for (auto & sv : SVlist)
    {
        if (sv.size() == 0) continue;

        if (sv[sv.length()-1] == '*')
        {
            //Wildcard!
            std::string wildcard = sv.substr(0, sv.size()-1);
            //std::cout << "--> Wildcard found in sensitive volume names: " << wildcard << std::endl;

            for (G4LogicalVolumeStore::iterator pos=store->begin(); pos!=store->end(); pos++)
            {
                const std::string & volName = (*pos)->GetName();
                //std::cout << "   analysing vol:" << volName << std::endl;
                if (isAccordingTo(volName, wildcard))
                {
                    //std::cout << "   match!" << std::endl;
                    SetSensitiveDetector( *pos, pSD );
                }
            }
        }
        else
            SetSensitiveDetector(sv, pSD, true);
    }

    // ---- Monitors ----
    for (MonitorSensitiveDetector * mon : SM.Monitors)
        SetSensitiveDetector(mon->Name, mon);

    // ---- Calorimeters ----
    for (CalorimeterSensitiveDetector * cal : SM.Calorimeters)
        SetSensitiveDetector(cal->Name, cal);
}

bool DetectorConstruction::isAccordingTo(const std::string &name, const std::string & wildcard) const
{
    const size_t size = wildcard.size();
    if (name.size() < size) return false;

    return ( wildcard == name.substr(0, size) );
}

void DetectorConstruction::setStepLimiters()
{
    SessionManager & SM = SessionManager::getInstance();
    const std::map<std::string, double> & StepLimitMap = SM.Settings.G4Set.StepLimits;

    if (StepLimitMap.empty()) return;

    G4LogicalVolumeStore* store = G4LogicalVolumeStore::GetInstance();
    for (auto const & it : StepLimitMap)
    {
        std::string VolName = it.first;
        double step = it.second * mm;
        if (step == 0)
        {
            SM.terminateSession("Found zero step limit for volume " + VolName);
            return;
        }

        if (VolName[VolName.length()-1] == '*')
        {
            //Wildcard!
            std::string wildcard = VolName.substr(0, VolName.size()-1);
            //std::cout << "--> Wildcard found in volume names for step limiter: " << wildcard << std::endl;

            for (G4LogicalVolumeStore::iterator pos=store->begin(); pos!=store->end(); pos++)
            {
                const std::string & volName = (*pos)->GetName();
                //std::cout << "   analysing vol:" << volName << std::endl;
                if (isAccordingTo(volName, wildcard))
                {
                    //std::cout << "   match!" << std::endl;
                    G4UserLimits * stepLimit = new G4UserLimits(step);
                    (*pos)->SetUserLimits(stepLimit);
                }
            }
        }
        else
        {
            for (G4LogicalVolumeStore::iterator pos=store->begin(); pos!=store->end(); pos++)
            {
                if ( VolName == (*pos)->GetName())
                {
                    //std::cout << "   found!" << std::endl;
                    G4UserLimits * stepLimit = new G4UserLimits(step);
                    (*pos)->SetUserLimits(stepLimit);
                }
            }
        }
    }
}
