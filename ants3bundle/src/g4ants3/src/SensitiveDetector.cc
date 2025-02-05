#include "SensitiveDetector.hh"
#include "SessionManager.hh"
#include "ahistogram.h"
#include "arandomg4hub.h"

#include <sstream>
#include <iomanip>

#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

void SensitiveDetectorTools::stopAndKill(G4Step * step)
{
    step->GetTrack()->SetTrackStatus(fStopAndKill);

    // Found cases when secondaries were generated inside the analyzer -> error in processing on ants3 side
    // !!! cannot use fKillTrackAndSecondaries --> there could be secondaries generated before entrance to the analyzer
    //  Beacuse of the bug in Geant4, the approach based on step->GetSecondaryInCurrentStep() does not work:
    //auto secondaries = step->GetSecondaryInCurrentStep();
    //for (auto sec : *secondaries){
    //    std::cout << "!!!" << sec->GetTrackID() << G4endl;
    //}  // it returns nullptrs -> track ids are not yet assigned

    // Using time info to kill the secondaries, created later than time of entrance
    G4TrackVector * vec = step->GetfSecondary();
    if (vec->empty()) return;

    size_t iTr = vec->size();
    do
    {
        iTr--;
        G4Track * tr = vec->at(iTr);
        //tr->SetTrackStatus(fStopAndKill); // ignored if set on a track in fSecondary container :(
        //std::cout << "->" << tr->GetGlobalTime() << G4endl;
        if (tr->GetGlobalTime() > step->GetPreStepPoint()->GetGlobalTime())
        {
            //std::cout << "->kill secondary created after particle tracking was aborted" << G4endl;
            vec->erase(vec->begin() + iTr);
            delete tr;
        }
        else break; // tracks created in the last step are at the end
    }
    while (iTr != 0);
}

DepositionSensitiveDetector::DepositionSensitiveDetector(const G4String& name)
    : G4VSensitiveDetector(name) {}

DepositionSensitiveDetector::~DepositionSensitiveDetector() {}

G4bool DepositionSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{  
    G4double edep = aStep->GetTotalEnergyDeposit()/keV;
    if (edep == 0.0) return false;

    SessionManager & SM = SessionManager::getInstance();

    const std::string&   pName = aStep->GetTrack()->GetParticleDefinition()->GetParticleName();
    const int&           iMat = SM.findMaterial( aStep->GetPreStepPoint()->GetMaterial()->GetName() ); //will terminate session if not found!
    const G4ThreeVector& G4pos = aStep->GetPostStepPoint()->GetPosition();
    const double&        time = aStep->GetPostStepPoint()->GetGlobalTime()/ns;
    const int &          copyNumber = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();

    double pos[3];
    pos[0] = G4pos.x();
    pos[1] = G4pos.y();
    pos[2] = G4pos.z();

    SM.saveDepoRecord(pName, iMat, edep, pos, time, copyNumber);

    return true;
}

// ---------------------------------------------------------------------------

#include "G4VProcess.hh"

MonitorSensitiveDetector::MonitorSensitiveDetector(const std::string & name) : G4VSensitiveDetector(name) {}

G4bool MonitorSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory * history)
{
    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    const G4VProcess * proc = preStepPoint->GetProcessDefinedStep();
    if (proc && proc->GetProcessType() == fTransportation)
        if (preStepPoint->GetStepStatus() == fGeomBoundary)
        {
            int index = preStepPoint->GetPhysicalVolume()->GetCopyNo();
            SessionManager & SM = SessionManager::getInstance();
            if (index < SM.Monitors.size())
            {
                if (!SM.Monitors[index]) SM.terminateSession("Nullptr monitor in sensitive detector hit");
                return SM.Monitors[index]->ProcessHits(step, history);
            }
            else SM.terminateSession("Invalid monitor index in sensitive detector hit");
        }

    return true;
}

MonitorSensitiveDetectorWrapper::MonitorSensitiveDetectorWrapper(const std::string & name, const std::string & particle, int index) :
    Name(name), ParticleName(particle), MonitorIndex(index) {}

G4bool MonitorSensitiveDetectorWrapper::ProcessHits(G4Step *step, G4TouchableHistory *)
{
    if ( pParticleDefinition && (step->GetTrack()->GetParticleDefinition() != pParticleDefinition) ) // for "all particles" pParticleDefinition == nullptr
        return true;

    const bool bIsPrimary = (step->GetTrack()->GetParentID() == 0);
    if (  bIsPrimary && !bAcceptPrimary   ) return true;
    if ( !bIsPrimary && !bAcceptSecondary ) return true;

    const bool bIsDirect = !step->GetTrack()->GetUserInformation();
    if (  bIsDirect && !bAcceptDirect)   return true;
    if ( !bIsDirect && !bAcceptIndirect) return true;

    //position info
    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    const G4ThreeVector & coord1 = preStepPoint->GetPosition();
    const G4AffineTransform & transformation = preStepPoint->GetTouchable()->GetHistory()->GetTopTransform();
    const G4ThreeVector localPosition = transformation.TransformPoint(coord1);
    //std::cout << "Local position: " << localPosition[0] << " " << localPosition[1] << " " << localPosition[2] << " " << std::endl;
    if ( localPosition[2] > 0  && !bAcceptUpper ) return true;
    if ( localPosition[2] < 0  && !bAcceptLower ) return true;
    const double x = localPosition[0] / mm;
    const double y = localPosition[1] / mm;
    hPosition->fill(x, y);

    // time info
    const double time = preStepPoint->GetGlobalTime() / TimeFactor;
    hTime->fill(time);

    // angle info
    G4ThreeVector vec = step->GetTrack()->GetMomentumDirection();
    //std::cout << "Global dir: "<< vec[0] << ' ' << vec[1] << ' '<< vec[2] << std::endl;
    transformation.ApplyAxisTransform(vec);
    double angle = 180.0/3.14159265358979323846*acos(vec[2]);
    if (angle > 90.0) angle = 180.0 - angle;
    //std::cout << "Local vector: " << vec[0] << " " << vec[1] << " " << vec[2] << " "<< angle << std::endl;
    hAngle->fill(angle);

    //energy
    const double energy = preStepPoint->GetKineticEnergy() / EnergyFactor;
    hEnergy->fill(energy);

    //stop tracking?
    if (bStopTracking)
    {
        //step->GetTrack()->SetTrackStatus(fStopAndKill);
        SensitiveDetectorTools::stopAndKill(step);

        SessionManager & SM = SessionManager::getInstance();
        if (SM.Settings.RunSet.SaveTrackingHistory)
        {
            const G4ThreeVector & pos = preStepPoint->GetPosition();
            const double kinE = preStepPoint->GetKineticEnergy()/keV;
            const double depoE = step->GetTotalEnergyDeposit()/keV;
            SM.saveTrackRecord("MonitorStop",
                               pos, time,
                               kinE, depoE);
        }
        // bug in Geant4.10.5.1? Tracking reports one more step - transportation from the monitor to the next volume
        //the next is the fix:
        SM.bStoppedOnMonitor = true;
        return true;
    }

    return true;
}

bool MonitorSensitiveDetectorWrapper::readFromJson(const json11::Json & json)
{
    //std::cout << "Monitor created for volume " << Name << " and particle " << ParticleName << std::endl;

    bAcceptLower =      json["bLower"].bool_value();
    bAcceptUpper =      json["bUpper"].bool_value();
    bStopTracking =     json["bStopTracking"].bool_value();
    bAcceptDirect =     json["bDirect"].bool_value();
    bAcceptIndirect =   json["bIndirect"].bool_value();
    bAcceptPrimary =    json["bPrimary"].bool_value();
    bAcceptSecondary =  json["bSecondary"].bool_value();

    angleBins =         json["angleBins"].int_value();
    angleFrom =         json["angleFrom"].number_value();
    angleTo =           json["angleTo"].number_value();

    energyBins =        json["energyBins"].int_value();
    energyFrom =        json["energyFrom"].number_value();
    energyTo =          json["energyTo"].number_value();
//    energyUnits =       json["energyUnitsInHist"].int_value(); // 0,1,2,3 -> meV, eV, keV, MeV;
//    double multipler = 1.0;
//    switch (energyUnits)
//    {
//    case 0: multipler *= 1e-6; break;
//    case 1: multipler *= 1e-3; break;
//    case 3: multipler *= 1e3;  break;
//    default:;
//    }
//    energyFrom *= multipler;
//    energyTo   *= multipler;
    EnergyUnits       = json["energyUnits"].string_value();
    if      (EnergyUnits == "meV") EnergyFactor = 0.001*eV;
    else if (EnergyUnits == "eV")  EnergyFactor = eV;
    else if (EnergyUnits == "keV") EnergyFactor = keV;
    else if (EnergyUnits == "MeV") EnergyFactor = MeV;
    else return false; // !!!*** error reporting system

    timeBins =          json["timeBins"].int_value();
    timeFrom =          json["timeFrom"].number_value();
    timeTo =            json["timeTo"].number_value();
    TimeUnits         = json["timeUnits"].string_value();
    if      (TimeUnits == "ns") TimeFactor = ns;
    else if (TimeUnits == "us") TimeFactor = us;
    else if (TimeUnits == "ms") TimeFactor = ms;
    else if (TimeUnits == "s")  TimeFactor = s;
    else return false; // !!!*** error reporting system

    xbins =             json["xbins"].int_value();
    ybins =             json["ybins"].int_value();
    size1 =             json["size1"].number_value();
    int shape =         json["shape"].number_value();
    if (shape == 0)         //rectangular
        size2 =         json["size2"].number_value();
    else                    //round
        size2 = size1;

    // creating histograms to store statistics
    hTime     = new AHistogram1D(timeBins,   timeFrom,   timeTo);
    hAngle    = new AHistogram1D(angleBins,  angleFrom,  angleTo);
    hEnergy   = new AHistogram1D(energyBins, energyFrom, energyTo);

    hPosition = new AHistogram2D(xbins, -size1, size1, ybins, -size2, size2);

    return true;
}

void MonitorSensitiveDetectorWrapper::writeToJson(json11::Json::object &json)
{
    json["MonitorIndex"] = MonitorIndex;

    json11::Json::object jsTime;
        writeHist1D(hTime, jsTime);
        jsTime["Units"] = TimeUnits;
    json["Time"] = jsTime;

    json11::Json::object jsAngle;
    writeHist1D(hAngle, jsAngle);
    json["Angle"] = jsAngle;

    json11::Json::object jsEnergy;
        writeHist1D(hEnergy, jsEnergy);
        jsEnergy["Units"] = EnergyUnits;
    json["Energy"] = jsEnergy;

    json11::Json::object jsSpatial;
    {
        std::vector<std::vector<double>> vSpatial = hPosition->getContent(); //[y][x]
        json11::Json::array ar;
        for (auto & row : vSpatial)
        {
            json11::Json::array el;
            for (const double & d : row)
                el.push_back(d);
            ar.push_back(el);
        }
        jsSpatial["data"] = ar;

        //getContent can change from/to!
        double xfrom, xto, yfrom, yto;
        hPosition->getLimits(xfrom, xto, yfrom, yto);
        jsSpatial["xfrom"] = xfrom;
        jsSpatial["xto"]   = xto;
        jsSpatial["yfrom"] = yfrom;
        jsSpatial["yto"]   = yto;

        const std::vector<double> vec = hPosition->getStat(); // [0] - sumVals, [1] - sumVals2, [2] - sumValX, [3] - sumValX2, [4] - sumValY, [5] - sumValY2, [6] - # entries
        json11::Json::array sjs;
        for (const double & d : vec)
            sjs.push_back(d);
        jsSpatial["stat"] = sjs;
    }
    json["Spatial"] = jsSpatial;
}

void MonitorSensitiveDetectorWrapper::writeHist1D(AHistogram1D *hist, json11::Json::object &json) const
{
    json11::Json::array ar;
    for (const double & d : hist->getContent())
        ar.push_back(d);
    json["data"] = ar;

    //getContent can change from/to!
    double from, to;
    hist->getLimits(from, to);
    json["from"] = from;
    json["to"] =   to;

    const std::vector<double> vec = hist->getStat(); //[0] - sumVals, [1] - sumVals2, [2] - sumValX, [3] - sumValX2, [4] - # entries
    json11::Json::array sjs;
    for (const double & d : vec)
        sjs.push_back(d);
    json["stat"] = sjs;
}

// ---

CalorimeterSensitiveDetector::CalorimeterSensitiveDetector(const std::string & name) :
    G4VSensitiveDetector(name) {}

G4bool CalorimeterSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory * history)
{
    const G4double depo = step->GetTotalEnergyDeposit();
    if (depo == 0.0) return true;

    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    int index = preStepPoint->GetPhysicalVolume()->GetCopyNo();
    SessionManager & SM = SessionManager::getInstance();
    if (index < SM.Calorimeters.size())
    {
        if (!SM.Calorimeters[index]) SM.terminateSession("Nullptr calorimeter in sensitive detector hit");
        return SM.Calorimeters[index]->ProcessHits(step, history);
    }
    else SM.terminateSession("Invalid calorimeter index in sensitive detector hit");

    return true;
}

CalorimeterSensitiveDetectorWrapper::CalorimeterSensitiveDetectorWrapper(const std::string & name, ACalorimeterProperties &properties, int index) :
    Name(name), Properties(properties), CalorimeterIndex(index)
{
    Data = new AHistogram3Dfixed(properties.Origin, properties.Step, properties.Bins);

    VoxelVolume_mm3 = Properties.Step[0] * Properties.Step[1] * Properties.Step[2]; // in mm3

    if (properties.CollectDepoOverEvent) EventDepoData = new AHistogram1D(properties.EventDepoBins, properties.EventDepoFrom, properties.EventDepoTo);
}

CalorimeterSensitiveDetectorWrapper::~CalorimeterSensitiveDetectorWrapper()
{
    delete Data;
}

G4bool CalorimeterSensitiveDetectorWrapper::ProcessHits(G4Step * step, G4TouchableHistory *)
{
    const G4double depo = step->GetTotalEnergyDeposit();
    if (depo == 0.0) return false;

    const G4ThreeVector & fromGlobal = step->GetPreStepPoint()->GetPosition();
    const G4ThreeVector & toGlobal   = step->GetPostStepPoint()->GetPosition();

    G4ThreeVector global;
    if (Properties.RandomizeBin)
    {
        const double rnd = 0.00001 + 0.99999 * ARandomHub::getInstance().uniform();
        global = fromGlobal + rnd * (toGlobal - fromGlobal);
    }
    else
        global = 0.5 * (fromGlobal + toGlobal);

    const G4ThreeVector local = step->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(global);

    //std::cout << global << local << "\n";

    registerHit(depo, local, step);

    return true;
}

void CalorimeterSensitiveDetectorWrapper::registerHit(double depo, const G4ThreeVector & local, G4Step * step)
{
    if (Properties.DataType == ACalorimeterProperties::Dose)
    {
        const G4Material * material = step->GetPreStepPoint()->GetMaterial();  // on material change step always ends anyway
        if (!material) return; // paranoic
        const double density_kgPerMM3 = material->GetDensity() / (kg/mm3);
        if (density_kgPerMM3 > 1e-10 * g/cm3 / (kg/mm3)) // ignore vacuum
        {
            const double deltaDose = (depo / joule) / (density_kgPerMM3 * VoxelVolume_mm3);
            Data->fill({local[0], local[1], local[2]}, deltaDose);
        }
    }
    else Data->fill({local[0], local[1], local[2]}, depo / keV);

    if (Properties.CollectDepoOverEvent) SumDepoOverEvent += depo / keV;
}

void CalorimeterSensitiveDetectorWrapper::writeToJson(json11::Json::object & json)
{
    json["CalorimeterIndex"] = CalorimeterIndex;

    json11::Json::object jsDepo;
    {
        std::vector<std::vector< std::vector<double> >> vSpatial = Data->getContent(); //[x][y][z]
        json11::Json::array ar;
        for (int iz = 0; iz < Properties.Bins[2]; iz++)
            for (int iy = 0; iy < Properties.Bins[1]; iy++)
            {
                json11::Json::array el;
                for (int ix = 0; ix < Properties.Bins[0]; ix++)
                    el.push_back(vSpatial[ix][iy][iz]);
                ar.push_back(el);
            }
        jsDepo["Data"] = ar;

        const std::vector<double> vec = Data->getStat();
        json11::Json::array sjs;
        for (const double & d : vec)
            sjs.push_back(d);
        jsDepo["Stat"] = sjs;

        jsDepo["Entries"] = Data->getEntries();
    }
    json["XYZDepo"] = jsDepo;

    if (Properties.CollectDepoOverEvent)
    {
        json11::Json::object js;

        const std::vector<double> & data = EventDepoData->getContent();
        double from, to;
        EventDepoData->getLimits(from, to);
        Properties.EventDepoFrom = from;
        Properties.EventDepoTo = to;
        double delta = (to - from) / Properties.EventDepoBins;
        json11::Json::array ar;
        for (size_t i = 0; i < data.size(); i++)  // 0=underflow and last=overflow
        {
            json11::Json::array el;
                el.push_back(from + (i - 0.5)*delta); // i - 1 + 0.5
                el.push_back(data[i]);
            ar.push_back(el);
        }
        js["Data"] = ar;

        const std::vector<double> vec = EventDepoData->getStat();
        json11::Json::array sjs;
        for (const double & d : vec)
            sjs.push_back(d);
        js["Stat"] = sjs;

        json["DepoOverEvent"] = js;
    }

    json11::Json::object jsProps;
    Properties.writeToJson(jsProps);
    json["Properties"] = jsProps;
}

// -------------------------

DelegatingCalorimeterSensitiveDetector::DelegatingCalorimeterSensitiveDetector(const std::string & name) :
    G4VSensitiveDetector(name) {}

G4bool DelegatingCalorimeterSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory * history)
{
    const G4double depo = step->GetTotalEnergyDeposit();
    if (depo == 0.0) return false;

    G4VSensitiveDetector * parentSensDet = nullptr;
    const G4TouchableHandle & touch = step->GetPreStepPoint()->GetTouchableHandle();
    int depth = 0;
    int index = 0;
    do
    {
        depth++; // starting with 1, do not want to inc during the last step
        G4VPhysicalVolume * physVol = touch->GetVolume(depth);
        if (!physVol) return false; // !!!***

        parentSensDet = physVol->GetLogicalVolume()->GetSensitiveDetector();
        index = physVol->GetCopyNo();
    }
    while (parentSensDet == this);

    SessionManager & SM = SessionManager::getInstance();
    if (index >= SM.Calorimeters.size()) SM.terminateSession("Bad index in delegating calorimeter hit");
    CalorimeterSensitiveDetectorWrapper * masterSensDet = SM.Calorimeters[index]; // dynamic_cast<CalorimeterSensitiveDetector*>(parentSensDet);
    if (!masterSensDet) SM.terminateSession("Cannot find main sensitive detector for composite calorimeter");

    const G4ThreeVector & fromGlobal = step->GetPreStepPoint()->GetPosition();
    const G4ThreeVector & toGlobal   = step->GetPostStepPoint()->GetPosition();
    G4ThreeVector global;
    if (masterSensDet->Properties.RandomizeBin)
    {
        const double rnd = 0.00001 + 0.99999 * ARandomHub::getInstance().uniform();
        global = fromGlobal + rnd * (toGlobal - fromGlobal);
    }
    else
        global = 0.5 * (fromGlobal + toGlobal);

    int currentHistoryDepth = touch->GetHistory()->GetDepth();

    //G4NavigationHistory * hist = touch->GetHistory()->GetTransform(depth).TransformPoint(global);
    //const G4ThreeVector local = touch->GetHistory()->GetTransform(depth).TransformPoint(global);
    const G4ThreeVector local = touch->GetHistory()->GetTransform(currentHistoryDepth - depth).TransformPoint(global);

    //std::cout << "CalorDepth:" << depth << " HistDepth"<< currentHistoryDepth << " Glob:"<< global << " Loc:" << local << "\n";

    masterSensDet->registerHit(depo, local, step);
    return true;
}

// -------------------------

AnalyzerSensitiveDetector::AnalyzerSensitiveDetector(const std::string & name) :
    G4VSensitiveDetector(name) {}

G4bool AnalyzerSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory *)
{
    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    const G4VProcess * proc = preStepPoint->GetProcessDefinedStep();
    if (!proc) return true;
    //std::cout << step->GetPreStepPoint()->GetProcessDefinedStep()->GetProcessName() << std::endl;
    //std::cout << step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() << std::endl;
    if (proc->GetProcessType() != fTransportation || preStepPoint->GetStepStatus() != fGeomBoundary) return true;

    int index = preStepPoint->GetPhysicalVolume()->GetCopyNo();
    // !!!*** error processing

    SessionManager & SM = SessionManager::getInstance();
    size_t iUnique = SM.Settings.RunSet.AnalyzerSettings.GlobalToUniqueLUT[index]; // !!!*** error processing

    bool isStopped = SM.Analyzers[iUnique].processParticle(step);
    if (isStopped)
    {
        SensitiveDetectorTools::stopAndKill(step);

        SessionManager & SM = SessionManager::getInstance();
        if (SM.Settings.RunSet.SaveTrackingHistory)
        {
            const G4ThreeVector & pos = preStepPoint->GetPosition();
            const double kinE = preStepPoint->GetKineticEnergy()/keV;
            //const double depoE = step->GetTotalEnergyDeposit()/keV;
            SM.saveTrackRecord("AnalyzerStop",
                               pos, preStepPoint->GetGlobalTime()/ns,
                               kinE, 0);
            // bug in Geant4.10.5.1? Tracking reports one more step - transportation from the monitor to the next volume
            //the next is the fix:
            SM.bStoppedOnMonitor = true;
        }
        return false;
    }

    return true;
}
