#include "aparticlesim_si.h"
#include "aparticlesimmanager.h"
#include "aerrorhub.h"
#include "acalorimeterhub.h"
#include "acalorimeter.h"
#include "amonitorhub.h"
#include "amonitor.h"
#include "ath.h"
#include "ajsontools.h"
#include "arandomhub.h"

#include <QDebug>
#include <QVariant>

#include "TH1D.h"

AParticleSim_SI::AParticleSim_SI() :
    SimMan(AParticleSimManager::getInstance())
{
    Help["getCalorimeterData"] = "Returns array of [x,val] or [x,y,val] or [x,y,z,val],\n"
                                 "options: 'x' or 'y' or 'z' for 1D,\n,"
                                 "'xy' or 'xz' or 'yx' or 'yz' or 'zx' or 'zy' for 2D,\n"
                                 "'xyz' or empty for 3D";
    Help["getCalorimeterBinning"] = "Returns array of 3 arrays, [Bins, Origin, Step], each one is for x,y and z axis";

    Help["loadAnalyzerData"] = "Load file with particle analyzer data";
    Help["countAnalyzers"] = "Count unique analyzers ('global' number of analyzers can be different fro 'unique' if merging of copies was selected at least for one analyzer)";
    Help["getAnalyzerDataAll"] = "Return array with the data for all unique analyzers. The array element is an array of [particleName, number, EnergyData], "
                                 "where EnergyData is an array of bins [Energy_centerOfBin_keV, Number], excluding under- and overflow";
    Help["getAnalyzerUniqueToGlobalIndex"] = "Get global analyzer indexes (returns -1 for all non-unique ones). "
                                             "Use getAnalyzerPositionsByGlobalIndex() to get positions of analyzers by the global index";
}

void AParticleSim_SI::simulate()
{
    SimMan.simulate();

    QString err = AErrorHub::getQError();
    if (!err.isEmpty()) abort(err);
}

#include "aparticlesimsettings.h"
void AParticleSim_SI::setSeed(double seed)
{
    SimMan.SimSet.RunSet.Seed = seed;
}

int AParticleSim_SI::countCalorimeters()
{
    return ACalorimeterHub::getConstInstance().countCalorimeters();
}

void AParticleSim_SI::loadCalorimeterData(QString fileName)
{
    ACalorimeterHub & CalHub = ACalorimeterHub::getInstance();
    if (CalHub.countCalorimeters() == 0)
    {
        abort("There are no calorimeters in the loaded config!");
        return;
    }

    CalHub.clearData();

    QJsonArray jsar;
    bool ok = jstools::loadJsonArrayFromFile(jsar, fileName);
    if (!ok)
    {
        abort("Could not open: " + fileName);
        return;
    }

    QString err = CalHub.appendDataFromJson(jsar);
    if (!err.isEmpty()) abort(err);
}

QVariantList AParticleSim_SI::getCalorimeterGlobalPositionsAll()
{
    QVariantList vl;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();
    for (const ACalorimeterData & cd : CalHub.Calorimeters)
        vl.push_back(QVariantList{cd.Position[0], cd.Position[1], cd.Position[2]});
    return vl;
}

QVariantList AParticleSim_SI::getCalorimeterDataProjection(int calorimeterIndex, QString mode)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();

    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }

    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }

    ATH3D * data = cal->DataHistogram;
    if (!data)
    {
        abort("Calorimeter has no data!");
        return res;
    }

    mode = mode.toLower();
    if (mode == "x" || mode == "y" || mode == "z")
    {
        TH1D * h = (TH1D*)data->Project3D(mode.toLatin1().data());
        const int num = h->GetXaxis()->GetNbins();
        for (int i = 0; i < num; i++)
            res.push_back( QVariantList{h->GetBinCenter(i+1), h->GetBinContent(i+1)} );
    }
    else if (mode == "xy" || mode == "xz" || mode == "yx" || mode == "yz" || mode == "zx" || mode == "zy")
    {
        TH2D * h = (TH2D*)data->Project3D(mode.toLatin1().data());
        const int numX = h->GetXaxis()->GetNbins();
        const int numY = h->GetYaxis()->GetNbins();
        for (int ix = 0; ix < numX; ix++)
            for (int iy = 0; iy < numY; iy++)
                res.push_back( QVariantList{h->GetXaxis()->GetBinCenter(ix+1), h->GetYaxis()->GetBinCenter(iy+1), h->GetBinContent(ix+1, iy+1)} );
    }
    else if (mode == "xyz" || mode == "")
    {
        const int numX = data->GetXaxis()->GetNbins();
        const int numY = data->GetYaxis()->GetNbins();
        const int numZ = data->GetZaxis()->GetNbins();
        for (int ix = 0; ix < numX; ix++)
            for (int iy = 0; iy < numY; iy++)
                for (int iz = 0; iz < numZ; iz++)
                    res.push_back( QVariantList{data->GetXaxis()->GetBinCenter(ix+1),
                                                data->GetYaxis()->GetBinCenter(iy+1),
                                                data->GetZaxis()->GetBinCenter(iz+1),
                                                data->GetBinContent(ix+1, iy+1, iz+1)} );
    }
    else abort("Undefined option in getCalorimeterData(), it should be one of the following:\nx y z xy xz yx yz zx zy");

    return res;
}

QVariantList AParticleSim_SI::getCalorimeterData(int calorimeterIndex)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();
    
    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }
    
    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }
    
    ATH3D * data = cal->DataHistogram;
    if (!data)
    {
        abort("Calorimeter has no data!");
        return res;
    }
    
    const int numX = data->GetXaxis()->GetNbins();
    const int numY = data->GetYaxis()->GetNbins();
    const int numZ = data->GetZaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
        for (int iy = 0; iy < numY; iy++)
            for (int iz = 0; iz < numZ; iz++)
                res.push_back( QVariantList{data->GetXaxis()->GetBinCenter(ix+1),
                                           data->GetYaxis()->GetBinCenter(iy+1),
                                           data->GetZaxis()->GetBinCenter(iz+1),
                                           data->GetBinContent(ix+1, iy+1, iz+1)} );
    
    return res;
}

QVariantList AParticleSim_SI::getCalorimeterOverEventData(int calorimeterIndex)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();

    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }

    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }

    ATH1D * data = cal->EventDepoData;
    if (!data)
    {
        abort("Calorimeter has no data!");
        return res;
    }

    const int num = data->GetXaxis()->GetNbins();
    for (int i = 0; i < num; i++)
                res.push_back( QVariantList{data->GetXaxis()->GetBinCenter(i+1),
                                            data->GetBinContent(i+1)} );

    return res;
}

QVariantList AParticleSim_SI::getCalorimeterBinning(int calorimeterIndex)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();

    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }

    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }

    const ACalorimeterProperties & prop = cal->Properties;

    QVariantList vlB;
    QVariantList vlO;
    QVariantList vlS;
    for (int i=0; i<3; i++)
    {
        vlB << prop.Bins[i];
        vlO << prop.Origin[i];
        vlS << prop.Step[i];
    }
    res.push_back(vlB);
    res.push_back(vlO);
    res.push_back(vlS);
    return res;
}

void AParticleSim_SI::clearCalorimeterData()
{
    ACalorimeterHub::getInstance().clearData();
}

int AParticleSim_SI::countMonitors()
{
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    return MonHub.countMonitors(AMonitorHub::Particle);
}

void AParticleSim_SI::loadMonitorData(QString fileName)
{
    AMonitorHub & MonHub = AMonitorHub::getInstance();
    if (MonHub.countMonitors(AMonitorHub::Particle) == 0)
    {
        abort("There are no monitors in the loaded config!");
        return;
    }

    MonHub.clearData(AMonitorHub::Particle);

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        abort("Could not open: " + fileName);
        return;
    }

    QString err = MonHub.appendDataFromJson(json, AMonitorHub::Particle);
    if (!err.isEmpty()) abort(err);
}

QVariantList AParticleSim_SI::getMonitorHitsAll()
{
    QVariantList vl;
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    for (const AMonitorData & md : MonHub.ParticleMonitors)
        vl.push_back(md.Monitor->getHits());
    return vl;
}

QVariantList AParticleSim_SI::getMonitorGlobalPositionsAll()
{
    QVariantList vl;
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    for (const AMonitorData & md : MonHub.ParticleMonitors)
        vl.push_back(QVariantList{md.Position[0], md.Position[1], md.Position[2]});
    return vl;
}

QVariantList AParticleSim_SI::getMonitorEnergy(int monitorIndex, QString units)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Particle);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.ParticleMonitors[monitorIndex].Monitor;
    if (!mon || !mon->energy)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    QString monEnergyUnits = mon->config.energyUnits;
    double factor = 1.0;
    if      (monEnergyUnits == "meV") factor = 1e-6; // meV -> keV
    else if (monEnergyUnits == "eV")  factor = 1e-3; // eV -> keV
    else if (monEnergyUnits == "keV") factor = 1.0;
    else if (monEnergyUnits == "MeV") factor = 1000.0; // MeV -> keV
    else
    {
        abort("Unrecognoized energy units of the monitor: " + monEnergyUnits);
        return vl;
    }

    if      (units == "meV") factor *= 1e6;
    else if (units == "eV")  factor *= 1e3;
    else if (units == "keV") ;
    else if (units == "MeV") factor *= 1e-3;
    else
    {
        abort("Unrecognized energy units: " + units);
        return vl;
    }

    TH1D * data = mon->energy;
    const int numX = data->GetXaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
    {
        double thisEn = data->GetXaxis()->GetBinCenter(ix+1) * factor;
        vl.push_back( QVariantList{thisEn, data->GetBinContent(ix+1)} );
    }

    return vl;
}

QVariantList AParticleSim_SI::getMonitorTime(int monitorIndex, QString units)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Particle);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.ParticleMonitors[monitorIndex].Monitor;
    if (!mon || !mon->time)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    QString monTimeUnits = mon->config.timeUnits;
    double factor = 1.0;
    if      (monTimeUnits == "ns") ;
    else if (monTimeUnits == "us") factor = 1e3; // us -> ns
    else if (monTimeUnits == "ms") factor = 1e6; // ms -> ns
    else if (monTimeUnits == "s")  factor = 1e9; // s  -> ns
    else
    {
        abort("Unrecognoized time units of the monitor: " + monTimeUnits);
        return vl;
    }

    if      (units == "ns") ;
    else if (units == "us") factor *= 1e-3;
    else if (units == "ms") factor *= 1e-6;
    else if (units == "s")  factor *= 1e-9;
    else
    {
        abort("Unrecognized time units: " + units);
        return vl;
    }

    TH1D * data = mon->time;
    const int numX = data->GetXaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
    {
        double thisTime = data->GetXaxis()->GetBinCenter(ix+1) * factor;
        vl.push_back( QVariantList{thisTime, data->GetBinContent(ix+1)} );
    }

    return vl;
}

QVariantList AParticleSim_SI::getMonitorAngle(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Particle);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.ParticleMonitors[monitorIndex].Monitor;
    if (!mon || !mon->angle)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    TH1D * data = mon->angle;
    const int numX = data->GetXaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
    {
        double thisAngle = data->GetXaxis()->GetBinCenter(ix+1);
        vl.push_back( QVariantList{thisAngle, data->GetBinContent(ix+1)} );
    }

    return vl;
}

QVariantList AParticleSim_SI::getMonitorXY(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Particle);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.ParticleMonitors[monitorIndex].Monitor;
    if (!mon || !mon->xy)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    TH2D * data = mon->xy;
    const int numX = data->GetXaxis()->GetNbins();
    const int numY = data->GetYaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
        for (int iy = 0; iy < numY; iy++)
            vl.push_back( QVariantList{data->GetXaxis()->GetBinCenter(ix+1),
                                       data->GetYaxis()->GetBinCenter(iy+1),
                                       data->GetBinContent(ix+1, iy+1)} );

    return vl;
}

// ----------
#include "ageometryhub.h"
#include "aparticleanalyzerhub.h"
void AParticleSim_SI::loadAnalyzerData(QString fileName)
{
    const AGeometryHub & GeoHub = AGeometryHub::getInstance();
    if (GeoHub.countParticleAnalyzers() == 0)
    {
        abort("There are no particle analyzers in the loaded config!");
        return;
    }

    AErrorHub::clear();
    AParticleAnalyzerHub & AnHub = AParticleAnalyzerHub::getInstance();
    AnHub.loadAnalyzerFiles({fileName});
    if (AErrorHub::isError())
    {
        abort(AErrorHub::getQError());
        return;
    }
}

int AParticleSim_SI::countAnalyzers()
{
    return AParticleAnalyzerHub::getConstInstance().UniqueAnalyzers.size();
}

QVariantList AParticleSim_SI::getAnalyzerDataAll()
{
    const AParticleAnalyzerHub & AnHub = AParticleAnalyzerHub::getInstance();
    QVariantList allAnalayzersList;

    for (const AAnalyzerData & an : AnHub.UniqueAnalyzers)
    {
        const QString enUnits = an.EnergyDataUnits;
        double energyFactor = 1.0;
        if      (enUnits == "MeV") energyFactor = 1e3;
        else if (enUnits == "eV")  energyFactor = 1e-3;
        else if (enUnits == "meV")  energyFactor = 1e-6;

        QVariantList oneAnalyzerList;
        for (const auto & pair : an.ParticleMap)
        {
            QVariantList particleList;

            const AAnalyzerParticle & particleRec = pair.second;
            QVariantList histVL;
            for (size_t i = 1; i < particleRec.EnergyBins+1; i++)
            {
                QVariantList el;
                    el.push_back(particleRec.EnergyHist->GetBinCenter(i));
                    el.push_back(particleRec.EnergyHist->GetBinContent(i) * energyFactor);
                histVL.push_back(el);
            }

            // [name, number, [energyDataExcludingUnderOver:[E_center,Number]]]
            particleList.push_back(pair.first);
            particleList.push_back( (int)particleRec.getNumber() );
            particleList.push_back(histVL);

            oneAnalyzerList.push_back(particleList);
        }

        allAnalayzersList.push_back(oneAnalyzerList);
    }

    return allAnalayzersList;
}

QVariantList AParticleSim_SI::getAnalyzerUniqueToGlobalIndex()
{
    const AParticleAnalyzerHub & AnHub = AParticleAnalyzerHub::getInstance();
    const std::vector<AAnalyzerData> UniqueAnalyzers = AnHub.UniqueAnalyzers;

    QVariantList vl;

    for (const AAnalyzerData & ad : UniqueAnalyzers)
        vl << ad.GlobalIndexIfNoMerge;

    return vl;
}

QVariantList AParticleSim_SI::getAnalyzerPositionsByGlobalIndex()
{
    const AGeometryHub & GeoHub = AGeometryHub::getInstance();

    QVariantList vl;

    for (const auto & ad : GeoHub.ParticleAnalyzers)
    {
        QVariantList el;
            const AVector3 & pos = std::get<2>(ad);
            for (size_t i = 0; i < 3; i++)
            {
                double res = pos[i];
                if (abs(res) < 1e-300) res = 0;
                el << res;
            }
        vl.push_back(el);
    }

    return vl;
}

// ----------

void AParticleSim_SI::setTrackingHistoryFileName(QString fileName)
{
    TrackingHistoryFileName = fileName;
}

void AParticleSim_SI::buildTracks(int maxTracks)
{
    buildTracks(false, false, false, QVariantList(), QVariantList(), maxTracks);
}

#include "atrackingdataexplorer.h"
void AParticleSim_SI::buildTracks(bool skipPrimaries, bool skipPrimNoInter, bool skipSecondaries, QVariantList limitToParticleList, QVariantList excludeParticles, int maxTracks)
{
    if (TrackingHistoryFileName.isEmpty())
    {
        abort("File name for track import is not set. Configure it using psim.setTrackingHistoryFileName");
        return;
    }

    QStringList LimitTo;
    for (int i = 0; i < limitToParticleList.size(); i++) LimitTo << limitToParticleList[i].toString();
    QStringList Exclude;
    for (int i = 0; i < excludeParticles.size(); i++)    Exclude << excludeParticles[i].toString();

    ATrackingDataExplorer explorer;
    QString err = explorer.buildTracks(TrackingHistoryFileName, LimitTo, Exclude,
                                       skipPrimaries, skipPrimNoInter, skipSecondaries,
                                       maxTracks, -1);

    if (!err.isEmpty())
    {
        abort("Error whil ebuilding tracks:\n" + err);
        return;
    }
}

void AParticleSim_SI::buildTracksSingleEvent(int eventIndex)
{
    if (TrackingHistoryFileName.isEmpty())
    {
        abort("File name for track import is not set. Configure it using psim.setTrackingHistoryFileName");
        return;
    }

    ATrackingDataExplorer explorer;
    QString err = explorer.buildTracks(TrackingHistoryFileName, QStringList(), QStringList(),
                                       false, false, false,
                                       10000, eventIndex);

    if (!err.isEmpty())
    {
        abort("Error whil ebuilding tracks:\n" + err);
        return;
    }
}

double getOrthoPsM(double w1, double w2, double w3)
{
    constexpr double electronMass = 2.0 * 0.510998910;
    return pow( ( electronMass - w1 ) / ( w2 * w3 ), 2 ) + pow( ( electronMass - w2 ) / ( w1 * w3 ), 2 ) + pow( ( electronMass - w3 ) / ( w1 * w2 ), 2 );
}

QVariantList AParticleSim_SI::getThreeGammasForPositronium()
{
    ARandomHub & RandHub = ARandomHub::getInstance();

    std::array<AVector3, 3> unitVectors;
    std::array<double, 3> energies;

    constexpr double orthoPsMMax = 7.65928;
    double weight, random_weight;
    do
    {
        makeCandidateVectors(unitVectors, energies);

        weight = getOrthoPsM(energies[0], energies[1], energies[2]);
        random_weight = orthoPsMMax * RandHub.uniform();
    }
    while (random_weight > weight);

    QVariantList res;
    for (int iG = 0; iG < 3; iG++)
    {
        //qDebug() << "#"<<i<<energies[iG] << "-->" <<unitVectors[iG][0]<<unitVectors[iG][1]<<unitVectors[iG][2];
        QVariantList gvl;
            for (int i = 0; i < 3; i++) gvl.push_back(unitVectors[iG][i]);
            gvl.push_back(energies[iG]);
        res.push_back(gvl);
    }
    return res;
}

void AParticleSim_SI::makeCandidateVectors(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies)
{
    ARandomHub & RandHub = ARandomHub::getInstance();

    constexpr double parentmass = 2.0 * 0.510998910;

    // https://apc.u-paris.fr/~franco/g4doxy/html/classG4GeneralPhaseSpaceDecay.html

    double daughtermomentum[3];
    double momentummax = 0;
    double momentumsum = 0;
    do
    {
        double rd1 = RandHub.uniform();
        double rd2 = RandHub.uniform();
        if (rd2 > rd1)
        {
            double rd  = rd1;
            rd1 = rd2;
            rd2 = rd;
        }
        momentummax = 0;
        momentumsum = 0;

        // daughter 0
        double energy = rd2 * parentmass;
        daughtermomentum[0] = energy;
        if (daughtermomentum[0] > momentummax) momentummax = daughtermomentum[0];
        momentumsum += daughtermomentum[0];

        // daughter 1
        energy = (1.0 - rd1) * parentmass;
        daughtermomentum[1] = energy;
        if (daughtermomentum[1] > momentummax) momentummax = daughtermomentum[1];
        momentumsum += daughtermomentum[1];

        // daughter 2
        energy = (rd1 - rd2) * parentmass;
        daughtermomentum[2] = energy;
        if ( daughtermomentum[2] >momentummax )momentummax = daughtermomentum[2];
        momentumsum += daughtermomentum[2];
    }
    while (momentummax > momentumsum - momentummax);

    //qDebug() << "     daughter 0:" << daughtermomentum[0];// /GeV << "[GeV/c]" <<G4endl;
    //qDebug() << "     daughter 1:" << daughtermomentum[1];
    //qDebug() << "     daughter 2:" << daughtermomentum[2];
    //qDebug() << "   momentum sum:" << momentumsum;//  /GeV << "[GeV/c]" <<G4endl;

    // first
    double costheta = 2.0 * RandHub.uniform() - 1.0;
    double sintheta = std::sqrt((1.0 - costheta)*(1.0 + costheta));
    double phi = 2.0*3.1415926535 * RandHub.uniform(); // [rad]
    double sinphi = std::sin(phi);
    double cosphi = std::cos(phi);
    unitVectors[0] = AVector3(sintheta*cosphi, sintheta*sinphi, costheta);
    energies[0] = daughtermomentum[0];
    //AVector3 momentumVector = direction * daughtermomentum[0];

    // second
    double costhetan = (daughtermomentum[1]*daughtermomentum[1] - daughtermomentum[2]*daughtermomentum[2] - daughtermomentum[0]*daughtermomentum[0]) / (2.0*daughtermomentum[2]*daughtermomentum[0]);
    double sinthetan = std::sqrt( (1.0 - costhetan) * (1.0 + costhetan) );
    double phin  = 2.0 * 3.1415926535 * RandHub.uniform(); // [rad]
    double sinphin = std::sin(phin);
    double cosphin = std::cos(phin);
    unitVectors[1] = AVector3(sinthetan*cosphin*costheta*cosphi - sinthetan*sinphin*sinphi + costhetan*sintheta*cosphi,
                              sinthetan*cosphin*costheta*sinphi + sinthetan*sinphin*cosphi + costhetan*sintheta*sinphi,
                              -sinthetan*cosphin*sintheta + costhetan*costheta);
    energies[1] = std::sqrt(daughtermomentum[2]*daughtermomentum[2]/unitVectors[1].mag2());
    unitVectors[1].toUnitVector();

    // third
    AVector3 mom(unitVectors[0]*daughtermomentum[0] + unitVectors[1]*daughtermomentum[2]);
    mom *= -1.0;
    energies[2] = std::sqrt(mom.mag2());
    unitVectors[2] = mom.toUnitVector();

}

/*
QVariantList AParticleSim_SI::getMonitorStats1D(int index, ASim_SI::dataType type) const
{
    QVariantList vl;
    if (!EventsDataHub->SimStat) return vl;
    if (index < 0 || index >= EventsDataHub->SimStat->Monitors.size()) return vl;

    const AMonitor* mon = EventsDataHub->SimStat->Monitors.at(index);
    TH1D* h = nullptr;
    switch (type)
    {
    case dat_time:   h = mon->getTime(); break;
    case dat_angle:  h = mon->getAngle(); break;
    case dat_wave:   h = mon->getWave(); break;
    case dat_energy: h = mon->getEnergy(); break;
    }
    if (!h) return vl;

    double stat[4];
    h->GetStats(stat); // stats[0] = sumw  stats[1] = sumw2   stats[2] = sumwx  stats[3] = sumwx2
    for (int i=0; i<4; i++)
        vl.push_back(stat[i]);
    return vl;
}

QVariantList AParticleSim_SI::getMonitorData1D(int index, dataType type) const
{
    QVariantList vl;
    if (!EventsDataHub->SimStat) return vl;
    if (index < 0 || index >= EventsDataHub->SimStat->Monitors.size()) return vl;

    const AMonitor* mon = EventsDataHub->SimStat->Monitors.at(index);
    TH1D* h = nullptr;
    switch (type)
    {
    case dat_time:   h = mon->getTime(); break;
    case dat_angle:  h = mon->getAngle(); break;
    case dat_wave:   h = mon->getWave(); break;
    case dat_energy: h = mon->getEnergy(); break;
    }
    if (!h) return vl;

    TAxis* axis = h->GetXaxis();
    for (int i=1; i<axis->GetNbins()+1; i++)
    {
        QVariantList el;
        el << axis->GetBinCenter(i);
        el << h->GetBinContent(i);
        vl.push_back(el);
    }
    return vl;
}
*/
