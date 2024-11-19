#include "aphotonsim_si.h"
#include "aphotonsimmanager.h"
#include "aerrorhub.h"
#include "amonitor.h"
#include "amonitorhub.h"
#include "ajsontools.h"

#include <QDebug>

#include "TH1D.h"
#include "TH2D.h"

APhotonSim_SI::APhotonSim_SI() :
    AScriptInterface(), SimMan(APhotonSimManager::getInstance()) {}

APhotonSim_SI::~APhotonSim_SI()
{
    qDebug() << "Dest for APhotonSim_SI";
}

void APhotonSim_SI::simulate()
{
    bool ok = SimMan.simulate(-1);

    QString err = AErrorHub::getQError();
    if (!ok || !err.isEmpty())
    {
        if (err.isEmpty()) err = "Unknown simulation error";
        abort(err);
    }
}

void APhotonSim_SI::setSeed(double seed)
{
    SimMan.setSeed(seed);
}

int APhotonSim_SI::countMonitors()
{
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    return MonHub.countMonitors(AMonitorHub::Photon);
}

void APhotonSim_SI::loadMonitorData(QString fileName)
{
    AMonitorHub & MonHub = AMonitorHub::getInstance();
    if (MonHub.countMonitors(AMonitorHub::Photon) == 0)
    {
        abort("There are no photon monitors in the loaded config!");
        return;
    }

    MonHub.clearData(AMonitorHub::Photon);

    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        abort("Could not open: " + fileName);
        return;
    }

    QString err = MonHub.appendDataFromJson(json, AMonitorHub::Photon);
    if (!err.isEmpty()) abort(err);
}

QVariantList APhotonSim_SI::getMonitorHitsAll()
{
    QVariantList vl;
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    for (const AMonitorData & md : MonHub.PhotonMonitors)
        vl.push_back(md.Monitor->getHits());
    return vl;
}

QVariantList APhotonSim_SI::getMonitorGlobalPositionsAll()
{
    QVariantList vl;
    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    for (const AMonitorData & md : MonHub.PhotonMonitors)
        vl.push_back(QVariantList{md.Position[0], md.Position[1], md.Position[2]});
    return vl;
}

QVariantList APhotonSim_SI::getMonitorWaveIndex(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Photon);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.PhotonMonitors[monitorIndex].Monitor;
    if (!mon || !mon->wave)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    TH1D * data = mon->wave;
    const int numX = data->GetXaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
    {
        double iWave = data->GetXaxis()->GetBinLowEdge(ix+1);
        vl.push_back( QVariantList{iWave, data->GetBinContent(ix+1)} );
    }

    return vl;
}

#include "aphotonsimhub.h"
QVariantList APhotonSim_SI::getMonitorWavelength(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Photon);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.PhotonMonitors[monitorIndex].Monitor;
    if (!mon || !mon->wave)
    {
        abort("Monitor data are not initialized!");
        return vl;
    }

    TH1D * data = mon->wave;
    const int numX = data->GetXaxis()->GetNbins();
    const AWaveResSettings & simSet = APhotonSimHub::getConstInstance().Settings.WaveSet;
    for (int ix = 0; ix < numX; ix++)
    {
        double iWave = data->GetXaxis()->GetBinLowEdge(ix+1);
        if (iWave <= 0) continue;
        double wave = simSet.toWavelength(iWave);
        vl.push_back( QVariantList{wave, data->GetBinContent(ix+1)} );
    }

    return vl;
}

QVariantList APhotonSim_SI::getMonitorTime(int monitorIndex, QString units)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Photon);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.PhotonMonitors[monitorIndex].Monitor;
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

QVariantList APhotonSim_SI::getMonitorAngle(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Photon);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.PhotonMonitors[monitorIndex].Monitor;
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

QVariantList APhotonSim_SI::getMonitorXY(int monitorIndex)
{
    QVariantList vl;

    const AMonitorHub & MonHub = AMonitorHub::getConstInstance();
    int numMon = MonHub.countMonitors(AMonitorHub::Photon);
    if (monitorIndex < 0 || monitorIndex >= numMon)
    {
        abort("bad monitor index");
        return vl;
    }

    AMonitor * mon = MonHub.PhotonMonitors[monitorIndex].Monitor;
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
