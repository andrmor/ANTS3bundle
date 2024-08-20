#include "SensitiveDetector.hh"
#include "SessionManager.hh"
#include "ahistogram.h"
#include "arandomg4hub.h"

#include <sstream>
#include <iomanip>

#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

SensitiveDetector::SensitiveDetector(const G4String& name)
    : G4VSensitiveDetector(name) {}

SensitiveDetector::~SensitiveDetector() {}

G4bool SensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
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

MonitorSensitiveDetector::MonitorSensitiveDetector(const std::string & name, const std::string & particle, int index) :
    G4VSensitiveDetector(name),
    Name(name), ParticleName(particle), MonitorIndex(index) {}

MonitorSensitiveDetector::~MonitorSensitiveDetector()
{
    //std::cout << "Deleting monitor object" << std::endl;
}

G4bool MonitorSensitiveDetector::ProcessHits(G4Step *step, G4TouchableHistory *)
{
    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    const G4VProcess * proc = preStepPoint->GetProcessDefinedStep();
    if (proc && proc->GetProcessType() == fTransportation)
        if (preStepPoint->GetStepStatus() == fGeomBoundary)
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
                step->GetTrack()->SetTrackStatus(fStopAndKill);

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
        }

    return true;
}

bool MonitorSensitiveDetector::readFromJson(const json11::Json & json)
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

void MonitorSensitiveDetector::writeToJson(json11::Json::object &json)
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

void MonitorSensitiveDetector::writeHist1D(AHistogram1D *hist, json11::Json::object &json) const
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

CalorimeterSensitiveDetector::CalorimeterSensitiveDetector(const std::string & name, ACalorimeterProperties &properties, int index) :
    G4VSensitiveDetector(name),
    Name(name), Properties(properties), CalorimeterIndex(index)
{
    Data = new AHistogram3Dfixed(properties.Origin, properties.Step, properties.Bins);

    VoxelVolume_mm3 = Properties.Step[0] * Properties.Step[1] * Properties.Step[2]; // in mm3

    if (properties.CollectDepoOverEvent) EventDepoData = new AHistogram1D(properties.EventDepoBins, properties.EventDepoFrom, properties.EventDepoTo);
}

CalorimeterSensitiveDetector::~CalorimeterSensitiveDetector()
{
    delete Data;
}

G4bool CalorimeterSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory *)
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

    if (Properties.DataType == ACalorimeterProperties::Dose)
    {
        const G4Material * material = step->GetPreStepPoint()->GetMaterial();  // on material change step always ends anyway
        if (!material) return false; // paranoic
        const double density_kgPerMM3 = material->GetDensity() / (kg/mm3);
        if (density_kgPerMM3 > 1e-10 * g/cm3 / (kg/mm3)) // ignore vacuum
        {
            const double deltaDose = (depo / joule) / (density_kgPerMM3 * VoxelVolume_mm3);
            Data->fill({local[0], local[1], local[2]}, deltaDose);
        }
    }
    else Data->fill({local[0], local[1], local[2]}, depo / MeV);

    if (Properties.CollectDepoOverEvent) SumDepoOverEvent += depo / MeV;

    return true;
}

void CalorimeterSensitiveDetector::writeToJson(json11::Json::object & json)
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

AnalyzerSensitiveDetector::AnalyzerSensitiveDetector(const AParticleAnalyzerRecord & properties) :
    G4VSensitiveDetector(properties.VolumeBaseName.empty() ? properties.VolumeNames.front() : properties.VolumeBaseName),
    Properties(properties)
{
    if      (properties.EnergyUnits == "MeV") EnergyFactor = 1 / MeV;
    else if (properties.EnergyUnits == "keV") EnergyFactor = 1 / keV;
    else if (properties.EnergyUnits == "eV")  EnergyFactor = 1 / eV;
}

AnalyzerSensitiveDetector::~AnalyzerSensitiveDetector() {}

G4bool AnalyzerSensitiveDetector::ProcessHits(G4Step * step, G4TouchableHistory *)
{
    G4StepPoint * preStepPoint = step->GetPreStepPoint();
    const G4VProcess * proc = preStepPoint->GetProcessDefinedStep();
    if (!proc) return true;
    //std::cout << step->GetPreStepPoint()->GetProcessDefinedStep()->GetProcessName() << std::endl;
    //std::cout << step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() << std::endl;
    if (proc->GetProcessType() != fTransportation || preStepPoint->GetStepStatus() != fGeomBoundary) return true;

    const double time = preStepPoint->GetGlobalTime() / ns;
    if (Properties.UseTimeWindow)
        if (time < Properties.TimeWindowFrom || time > Properties.TimeWindowTo) return true;

    const double energy = preStepPoint->GetKineticEnergy() * EnergyFactor;
    const std::string & particleName = step->GetTrack()->GetParticleDefinition()->GetParticleName();
    const auto it = ParticleMap.find(particleName);
    if (it != ParticleMap.end())
    {
        it->second.Number++;
        it->second.Energy->fill(energy);
    }
    else
    {
        AnalyzerParticleEntry rec;
        rec.Energy = new AHistogram1D(Properties.EnergyBins, Properties.EnergyFrom, Properties.EnergyTo);
        rec.Number = 1;
        rec.Energy->fill(energy);
        ParticleMap[particleName] = rec; // !!!*** optimize to avoid second lookup
    }

    if (Properties.StopTracking)
    {
        step->GetTrack()->SetTrackStatus(fStopAndKill);
        // Found cases when secondaries were generated inside the analyzer -> error in processing on ants3 side
        // !!! cannot use fKillTrackAndSecondaries --> there could be secondaries generated before entrance to the analyzer
        //   unfortunately, there is long standing bug in Geant4, the followin gapproach does not work:
        //auto secondaries = step->GetSecondaryInCurrentStep();
        //for (auto sec : *secondaries){
        //    std::cout << "!!!" << sec->GetTrackID() << G4endl;
        //}  // it returns nullptrs -> track ids are not yet assigned
        //
        // Using time info to kill the secondaries, created later than time of entrance
        G4TrackVector * vec = step->GetfSecondary();
        //std::cout << "Total secondaries " << vec->size() << G4endl;
        //std::cout << "Step pre-time:" << time << G4endl;
        if (!vec->empty())
        {
            //tr->SetTrackStatus(fStopAndKill); // ignored if set on a track in vec :(

            size_t iTr = vec->size();
            do
            {
                iTr--;
                G4Track * tr = vec->at(iTr);
                //std::cout << "->" << tr->GetGlobalTime() << G4endl;
                if (tr->GetGlobalTime() > preStepPoint->GetGlobalTime())
                {
                    //std::cout << "->kill" << G4endl;
                    vec->erase(vec->begin() + iTr);
                }
            }
            while (iTr != 0);
        }

        SessionManager & SM = SessionManager::getInstance();
        if (SM.Settings.RunSet.SaveTrackingHistory)
        {

            const G4ThreeVector & pos = preStepPoint->GetPosition();
            const double kinE = preStepPoint->GetKineticEnergy()/keV;
            //const double depoE = step->GetTotalEnergyDeposit()/keV;
            SM.saveTrackRecord("AnalyzerStop",
                               pos, time,
                               kinE, 0);
            // bug in Geant4.10.5.1? Tracking reports one more step - transportation from the monitor to the next volume
            //the next is the fix:
            SM.bStoppedOnMonitor = true;
        }
        return false;
    }

    return true;
}

void AnalyzerSensitiveDetector::writeToJson(json11::Json::object & json)
{
    json["UniqueIndex"] = Properties.UniqueIndex;
    json["VolumeBaseName"] = (Properties.VolumeBaseName.empty() ? Properties.VolumeNames.front() : Properties.VolumeBaseName ); // it will be the base name for non-unique objects

    json11::Json::array arAllParticles;
    for (const auto & pair : ParticleMap)
    {
        json11::Json::object js;
        js["Particle"] = pair.first;

        AHistogram1D * Hist = pair.second.Energy;
        const std::vector<double> & data = Hist->getContent();
        double from, to;
        Hist->getLimits(from, to);
        js["EnergyFrom"] = from;
        js["EnergyTo"] = to;
        double delta = (to - from) / Properties.EnergyBins;
        json11::Json::array ar;
        for (size_t i = 0; i < data.size(); i++)  // 0=underflow and last=overflow
        {
            json11::Json::array el;
            el.push_back(from + (i - 0.5)*delta); // i - 1 + 0.5
            el.push_back(data[i]);
            ar.push_back(el);
        }
        js["EnergyData"] = ar;

        const std::vector<double> vec = Hist->getStat();
        json11::Json::array sjs;
        for (const double & d : vec)
            sjs.push_back(d);
        js["EnergyStats"] = sjs;

        arAllParticles.push_back(js);
    }
    json["ParticleData"] = arAllParticles;
}
