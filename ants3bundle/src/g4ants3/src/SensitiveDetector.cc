#include "SensitiveDetector.hh"
#include "SessionManager.hh"
#include "ahistogram.hh"

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

    const int&           iPart = SM.findParticle( aStep->GetTrack()->GetParticleDefinition()->GetParticleName() );
    const int&           iMat = SM.findMaterial( aStep->GetPreStepPoint()->GetMaterial()->GetName() ); //will terminate session if not found!
    const G4ThreeVector& G4pos = aStep->GetPostStepPoint()->GetPosition();
    const double&        time = aStep->GetPostStepPoint()->GetGlobalTime()/ns;

    double pos[3];
    pos[0] = G4pos.x();
    pos[1] = G4pos.y();
    pos[2] = G4pos.z();

    SM.saveDepoRecord(iPart, iMat, edep, pos, time);

    if (iPart < 0) SM.DepoByNotRegistered += edep;
    else SM.DepoByRegistered += edep;

    return true;
}

// ---------------------------------------------------------------------------

#include "G4VProcess.hh"

MonitorSensitiveDetector::MonitorSensitiveDetector(const G4String &name)
    : G4VSensitiveDetector(name) {}

MonitorSensitiveDetector::~MonitorSensitiveDetector()
{
    std::cout << "Deleting monitor object" << std::endl;
}

G4bool MonitorSensitiveDetector::ProcessHits(G4Step *step, G4TouchableHistory *)
{
    const G4VProcess * proc = step->GetPostStepPoint()->GetProcessDefinedStep();
    if (proc && proc->GetProcessType() == fTransportation)
        if (step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary)
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
            G4StepPoint* p1 = step->GetPreStepPoint();
            const G4ThreeVector & coord1 = p1->GetPosition();
            const G4AffineTransform & transformation = p1->GetTouchable()->GetHistory()->GetTopTransform();
            const G4ThreeVector localPosition = transformation.TransformPoint(coord1);
            //std::cout << "Local position: " << localPosition[0] << " " << localPosition[1] << " " << localPosition[2] << " " << std::endl;
            if ( localPosition[2] > 0  && !bAcceptUpper ) return true;
            if ( localPosition[2] < 0  && !bAcceptLower ) return true;
            const double x = localPosition[0] / mm;
            const double y = localPosition[1] / mm;
            hPosition->Fill(x, y);

            // time info
            double time = step->GetPostStepPoint()->GetGlobalTime()/ns;
            hTime->Fill(time);

            // angle info
            G4ThreeVector vec = step->GetTrack()->GetMomentumDirection();
            //std::cout << "Global dir: "<< vec[0] << ' ' << vec[1] << ' '<< vec[2] << std::endl;
            transformation.ApplyAxisTransform(vec);
            double angle = 180.0/3.14159265358979323846*acos(vec[2]);
            if (angle > 90.0) angle = 180.0 - angle;
            //std::cout << "Local vector: " << vec[0] << " " << vec[1] << " " << vec[2] << " "<< angle << std::endl;
            hAngle->Fill(angle);

            //energy
            double energy = step->GetPostStepPoint()->GetKineticEnergy() / keV;
            hEnergy->Fill(energy);

            //stop tracking?
            if (bStopTracking)
            {
                step->GetTrack()->SetTrackStatus(fStopAndKill);

                SessionManager & SM = SessionManager::getInstance();
                if (SM.CollectHistory != SessionManager::NotCollecting)
                {
                    const G4ThreeVector & pos = step->GetPostStepPoint()->GetPosition();
                    const double kinE = step->GetPostStepPoint()->GetKineticEnergy()/keV;
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

void MonitorSensitiveDetector::readFromJson(const json11::Json &json)
{
    Name =              json["Name"].string_value();
    ParticleName =      json["ParticleName"].string_value();
    MonitorIndex =      json["MonitorIndex"].int_value();

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
    energyUnits =       json["energyUnitsInHist"].int_value(); // 0,1,2,3 -> meV, eV, keV, MeV;
    double multipler = 1.0;
    switch (energyUnits)
    {
    case 0: multipler *= 1e-6; break;
    case 1: multipler *= 1e-3; break;
    case 3: multipler *= 1e3;  break;
    default:;
    }
    energyFrom *= multipler;
    energyTo   *= multipler;

    timeBins =          json["timeBins"].int_value();
    timeFrom =          json["timeFrom"].number_value();
    timeTo =            json["timeTo"].number_value();

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

    /*
    vTime.resize(timeBins+2);
    timeDelta = (timeTo - timeFrom) / timeBins;
    vAngle.resize(angleBins+2);
    angleDelta = (angleTo - angleFrom) / angleBins;
    vEnergy.resize(energyBins+2);
    energyDelta = (energyTo - energyFrom) / energyBins;
    vSpatial.resize(ybins+2);
    for (auto & v : vSpatial)
        v.resize(xbins+2);
    xDelta = 2.0 * size1 / xbins;
    yDelta = 2.0 * size2 / ybins;
    */
}

void MonitorSensitiveDetector::writeToJson(json11::Json::object &json)
{
    json["MonitorIndex"] = MonitorIndex;

    json11::Json::object jsTime;
    writeHist1D(hTime, jsTime);
    json["Time"] = jsTime;

    json11::Json::object jsAngle;
    writeHist1D(hAngle, jsAngle);
    json["Angle"] = jsAngle;

    json11::Json::object jsEnergy;
    writeHist1D(hEnergy, jsEnergy);
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
