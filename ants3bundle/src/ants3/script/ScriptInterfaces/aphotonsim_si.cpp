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
    //qDebug() << "Destr for APhotonSim_SI";
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

#include "aphotonstatistics.h"
#include "astatisticshub.h"
void APhotonSim_SI::loadStatistics(QString fileName)
{
    QJsonObject json;
    bool ok = jstools::loadJsonFromFile(json, fileName);
    if (!ok)
    {
        abort("Could not open file with photon statistics: " + fileName);
        return;
    }

    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    Stat.clear();
    Stat.readFromJson(json);
}

QVariantList APhotonSim_SI::getStatistics_SensorAngular()
{
    QVariantList vl;
    APhotonStatistics & Stat = AStatisticsHub::getInstance().SimStat;
    if (!Stat.AngularDistr)
    {
        abort("Angular distribution is not loaded!");
        return vl;
    }

    TH1D * data = Stat.AngularDistr;
    const int numX = data->GetXaxis()->GetNbins();
    for (int ix = 0; ix < numX; ix++)
    {
        double point = data->GetXaxis()->GetBinCenter(ix+1);
        vl.push_back( QVariantList{point, data->GetBinContent(ix+1)} );
    }

    return vl;
}

#include "ageomeshhandler.h"
#include <chrono>
#include <random>
static AGeoMeshHandler handler;
int APhotonSim_SI::mesh_build(int N_target)
{
    handler.buildHemisphereMesh(N_target);
    AGeoMeshHandler::EdgeStats es = handler.edgeLengthStats();

    qDebug() << "Requested N        : " << N_target;
    qDebug() << "Subdivision freq v : " << handler.v;
    qDebug() << "Actual triangles   : " << handler.triangles.size() << "  (= 10*v^2)";
    qDebug() << "Edge spread std/mean: " << (100.0 * es.std / es.mean) << "%\n";

/*
    // --- correctness: fast lookup vs brute force, on random hemisphere points ---
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> unif(-1.0, 1.0);
    std::uniform_real_distribution<double> unifPos(0.0, 1.0);

    auto randomHemispherePoint = [&]() -> TVector3
    {
        TVector3 p;
        do {
            p = TVector3(unif(rng), unif(rng), unifPos(rng));
        } while (p.Mag() < 1e-6);
        return p.Unit();
    };

    const int NTEST = 2000;
    int mismatches = 0;
    std::vector<TVector3> testPts;
    testPts.reserve(NTEST);
    for (int i = 0; i < NTEST; ++i) testPts.push_back(randomHemispherePoint());
    for (const auto& p : testPts) {
        int a = handler.mesh.FindTriangleIndex(p);
        int b = handler.mesh.FindTriangleIndexBruteForce(p);
        if (a != b) ++mismatches;
    }
    qDebug() << "Correctness check (fast vs brute force), " << NTEST << " random points: "
              << (NTEST - mismatches) << "/" << NTEST << " match\n";

    // centroid round-trip using the FAST lookup
    int ok = 0;
    for (size_t i = 0; i < handler.mesh.triangles.size(); ++i) {
        const auto& t = handler.mesh.triangles[i];
        TVector3 c = (handler.mesh.vertices[t[0]] + handler.mesh.vertices[t[1]] + handler.mesh.vertices[t[2]]) * (1.0 / 3.0);
        if (handler.mesh.FindTriangleIndex(c) == static_cast<int>(i)) ++ok;
    }
    qDebug() << "Centroid round-trip (fast lookup): " << ok << "/" << handler.mesh.triangles.size()
              << " correct\n\n";

    // --- benchmark: millions of points ---
    const long NQ = 3'000'000;
    std::vector<TVector3> bigBatch;
    bigBatch.reserve(NQ);
    for (long k = 0; k < NQ; ++k) bigBatch.push_back(randomHemispherePoint());

    auto t0 = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    for (long k = 0; k < NQ; ++k) sum += handler.mesh.FindTriangleIndex(bigBatch[k]);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    qDebug() << "Fast lookup:        " << NQ << " points in " << ms << " ms  ("
              << (ms * 1000.0 / NQ) << " us/point)  [checksum " << sum << "]\n";

    const long NQ_BF = 20000;  // brute force is O(T); keep small
    t0 = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (long k = 0; k < NQ_BF; ++k) sum += handler.mesh.FindTriangleIndexBruteForce(bigBatch[k]);
    t1 = std::chrono::high_resolution_clock::now();
    ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    qDebug() << "Brute force lookup: " << NQ_BF << " points in " << ms << " ms  ("
              << (ms * 1000.0 / NQ_BF) << " us/point)  [checksum " << sum << "]\n";
*/
    return handler.triangles.size();
}

QVariantList APhotonSim_SI::mesh_getVertices()
{
    QVariantList vertsVL;
    for (const AGeoMeshHandler::Vec3 & vert : handler.vertices)
    {
        QVariantList el;
        el << vert[0] << vert[1] << vert[2];
        vertsVL.push_back(el);
    }
    return vertsVL;
}

QVariantList APhotonSim_SI::mesh_getTriangles()
{
    QVariantList triangsVL;
    for (const AGeoMeshHandler::Triangle & tri : handler.triangles)
    {
        QVariantList el;
        el << tri[0] << tri[1] << tri[2];
        triangsVL.push_back(el);
    }
    return triangsVL;
}

int APhotonSim_SI::mesh_findIndex(double x, double y, double z)
{
    return handler.findTriangleIndex({x, y, z});
}

/*
int APhotonSim_SI::findIndexSlow(double x, double y, double z)
{
    return handler.findTriangleIndexBruteForce({x, y, z});
}
*/
