#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"
#include "SessionManager.hh"

#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserLimits.hh"

#include "G4Material.hh"

#include <algorithm>

DetectorConstruction::DetectorConstruction(G4VPhysicalVolume *setWorld)
    : G4VUserDetectorConstruction(), fWorld(setWorld) {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    setStepLimiters();
    return fWorld;
}

#include "aparticleanalyzersettings.h"
void DetectorConstruction::ConstructSDandField()
{
    // ---- Energy depositions in sensitive volumes -----
    SessionManager & SM = SessionManager::getInstance();

    DepositionSensitiveDetector* pSD = new DepositionSensitiveDetector(SM.DepoLoggerSDName);
    G4SDManager::GetSDMpointer()->AddNewDetector(pSD);

    G4LogicalVolumeStore* store = G4LogicalVolumeStore::GetInstance();

    std::vector<std::string> SVlist;
    SVlist.reserve(SM.Settings.G4Set.SensitiveVolumes.size() + SM.Settings.G4Set.ScintSensitiveVolumes.size());
    std::copy(SM.Settings.G4Set.SensitiveVolumes.begin(), SM.Settings.G4Set.SensitiveVolumes.end(), std::back_inserter(SVlist));
    if (SM.Settings.G4Set.AddScintillatorsToSensitiveVolumes)
        std::copy(SM.Settings.G4Set.ScintSensitiveVolumes.begin(), SM.Settings.G4Set.ScintSensitiveVolumes.end(), std::back_inserter(SVlist));
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
    if (!SM.Monitors.empty())
    {
        MonitorSensitiveDetector * msens = new MonitorSensitiveDetector("MonSensDet");

        std::vector<std::string> alreadyAddedNames;
        for (MonitorSensitiveDetectorWrapper * mon : SM.Monitors)
        {
            if (!mon) continue;
            if (std::find(alreadyAddedNames.begin(), alreadyAddedNames.end(), mon->Name) == alreadyAddedNames.end())
            {
                SetSensitiveDetector(mon->Name, msens, true);
                alreadyAddedNames.push_back(mon->Name);
            }
        }
    }

    // ---- Calorimeters ----
    if (!SM.Calorimeters.empty())
    {
        CalorimeterSensitiveDetector * csens = new CalorimeterSensitiveDetector("CaloSensDet");

        std::vector<std::string> alreadyAddedNames;
        for (CalorimeterSensitiveDetectorWrapper * cal : SM.Calorimeters)
        {
            if (!cal) continue;
            if (std::find(alreadyAddedNames.begin(), alreadyAddedNames.end(), cal->Name) == alreadyAddedNames.end())
            {
                SetSensitiveDetector(cal->Name, csens, true);
                alreadyAddedNames.push_back(cal->Name);
            }
        }
    }
    if (!SM.Settings.RunSet.CalorimeterSettings.DelegatingCalorimeters.empty())
    {
        DelegatingCalorimeterSensitiveDetector * delCal = new DelegatingCalorimeterSensitiveDetector("DelCal");

        std::vector<std::string> alreadyAddedNames;
        for (const std::string & name : SM.Settings.RunSet.CalorimeterSettings.DelegatingCalorimeters)
        {
            if (std::find(alreadyAddedNames.begin(), alreadyAddedNames.end(), name) == alreadyAddedNames.end())
            {
                SetSensitiveDetector(name, delCal, true);
                alreadyAddedNames.push_back(name);
            }
        }
    }

    // ---- Analyzers ----
    if (SM.Settings.RunSet.AnalyzerSettings.Enabled)
    {
        AnalyzerSensitiveDetector * asd_entrance = new AnalyzerSensitiveDetector("AnSensDet_Ent"); asd_entrance->AnalyzeCreated = false;
        AnalyzerSensitiveDetector * asd_creation = new AnalyzerSensitiveDetector("AnSensDet_Cre"); asd_creation->AnalyzeCreated = true;

        for (const AParticleAnalyzerRecord & rec : SM.Settings.RunSet.AnalyzerSettings.AnalyzerTypes)
        {
            AnalyzerSensitiveDetector * asd = (rec.AnalyzeCreated ? asd_creation : asd_entrance);
            for (const std::string & name : rec.VolumeNames)
                SetSensitiveDetector(name, asd, true);
        }
    }
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
        if (step <= 0)
        {
            SM.terminateSession("Found zero or negative step limit for volume " + VolName);
            return;
        }

        if (VolName[VolName.length()-1] == '*')
        {
            //Wildcard!
            std::string wildcard = VolName.substr(0, VolName.size()-1);
            //std::cout << "--> Wildcard found in volume names for step limiter: " << wildcard << std::endl;

            for (G4LogicalVolumeStore::iterator pos=store->begin(); pos!=store->end(); pos++)
            {
                std::string volName = (*pos)->GetName();
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
                std::string name = (*pos)->GetName();
                if (VolName == name)
                {
                    //std::cout << "   found!" << std::endl;
                    G4UserLimits * stepLimit = new G4UserLimits(step);
                    (*pos)->SetUserLimits(stepLimit);
                }
            }
        }
    }
}
