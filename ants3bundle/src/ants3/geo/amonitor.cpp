#include "amonitor.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ajsontools.h"
#include "aroothistappenders.h"

#include <QDebug>

#include "TH1D.h"
#include "TH2D.h"
#include "TString.h"

AMonitor::AMonitor() : name("Undefined"), time(0), xy(0), angle(0), wave(0), energy(0) {}

AMonitor::AMonitor(const AGeoObject *MonitorGeoObject) : time(0), xy(0), angle(0), wave(0), energy(0)
{
    readFromGeoObject(MonitorGeoObject);
}

AMonitor::~AMonitor()
{
    clearData();
}

void AMonitor::clearData()
{
    delete time;   time   = nullptr;
    delete xy;     xy     = nullptr;
    delete angle;  angle  = nullptr;
    delete wave;   wave   = nullptr;
    delete energy; energy = nullptr;
}

int AMonitor::getHits() const
{
    if (!xy) return 0;
    return xy->GetEntries();
}

void AMonitor::fillForParticle(double x, double y, double Time, double Angle, double Energy)
{
    if (xy) xy->Fill(x,y);
    if (time) time->Fill(Time);
    if (angle) angle->Fill(Angle);

    if (energy)
    {
        switch (config.energyUnitsInHist)
        {
        case 0: Energy *= 1.0e6; break;
        case 1: Energy *= 1.0e3; break;
        case 2: break;
        case 3: Energy *= 1.0e-3;break;
        }
        energy->Fill(Energy);
    }
}

void AMonitor::fillForPhoton(double x, double y, double Time, double Angle, int waveIndex)
{
    if (xy)    xy->Fill(x,y);
    if (time)  time->Fill(Time);
    if (angle) angle->Fill(Angle);
    if (wave)  wave->Fill(waveIndex);
}

bool AMonitor::readFromGeoObject(const AGeoObject *MonitorRecord)
{
    const ATypeMonitorObject* mon = dynamic_cast<const ATypeMonitorObject*>(MonitorRecord->Type);
    if (!mon)
    {
        qWarning() << "This is not a monitor type AGeoObject!";
        return false;
    }

    config = mon->config;

    name = MonitorRecord->Name;

    initXYHist();
    initTimeHist();
    initAngleHist();
    if (config.PhotonOrParticle == 0) initWaveHist();
    else initEnergyHist();

    return true;
}

void AMonitor::writeDataToJson(QJsonObject & json) const
{
    json["Time"]   = jstools::regularTh1dToJson(time);
    json["Wave"]   = jstools::regularTh1dToJson(wave);
    json["Angle"]  = jstools::regularTh1dToJson(angle);
    json["Energy"] = jstools::regularTh1dToJson(energy);

    json["XY"]     = jstools::regularTh2dToJson(xy);
}

void AMonitor::readDataFromJson(const QJsonObject &json)
{
    clearData();

    jstools::parseJson(json, "Time",   time);
    jstools::parseJson(json, "Wave",   wave);
    jstools::parseJson(json, "Angle",  angle);
    jstools::parseJson(json, "Energy", energy);

    jstools::parseJson(json, "XY",     xy);
}

void AMonitor::append(const AMonitor & from)
{
    appendTH1D(time,   from.time);
    appendTH1D(angle,  from.angle);
    appendTH1D(wave,   from.wave);
    appendTH1D(energy, from.energy);

    appendTH2D(xy,     from.xy);
}

void AMonitor::initXYHist()
{
    delete xy;
    const double limit2 = ( config.shape == 0 ? config.size2 : config.size1 ); // 0 - rectangular, 1 - round
    xy = new TH2D("", "", config.xbins, -config.size1, config.size1, config.ybins, -limit2, limit2);
    xy->SetXTitle("X, mm");
    xy->SetYTitle("Y, mm");
}

void AMonitor::initTimeHist()
{
    delete time;
    time = new TH1D("", "", config.timeBins, config.timeFrom, config.timeTo);
    time->SetXTitle("Time, ns");
}

void AMonitor::initWaveHist()
{
    delete wave;
    wave = new TH1D("", "", config.waveBins, config.waveFrom, config.waveTo);
    wave->SetXTitle("Wave index");
}

void AMonitor::initAngleHist()
{
    delete angle;
    angle = new TH1D("", "", config.angleBins, config.angleFrom, config.angleTo);
    angle->SetXTitle("Angle, degrees");
}

void AMonitor::initEnergyHist()
{
    delete energy;

    double from = config.energyFrom;
    double to = config.energyTo;
    TString title = "";
    switch (config.energyUnitsInHist)
    {
    case 0: title = "Energy, meV"; break;
    case 1: title = "Energy, eV"; break;
    case 2: title = "Energy, keV"; break;
    case 3: title = "Energy, MeV"; break;
    }

    energy = new TH1D("", "", config.energyBins, from, to);
    energy->SetXTitle(title);
}
