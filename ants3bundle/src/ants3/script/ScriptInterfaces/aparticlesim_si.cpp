#include "aparticlesim_si.h"
#include "aparticlesimmanager.h"
#include "aerrorhub.h"
#include "acalorimeterhub.h"
#include "acalorimeter.h"
#include "amonitorhub.h"
#include "amonitor.h"
#include "ath.h"
#include "ajsontools.h"

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
}

void AParticleSim_SI::simulate(bool updateGui)
{
    AErrorHub::clear();

    SimMan.simulate();

    QString err = AErrorHub::getQError();
    if (err.isEmpty())
    {
        if (updateGui) SimMan.requestUpdateResultsGUI();
    }
    else
    {
        abort(err);
    }
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

/*
QVariantList AParticleSim_SI::getCalorimeterData(int calorimeterIndex, QString mode)
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
    else abort("Undefined option in getCalorimeterData()");

    return res;
}
*/

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
