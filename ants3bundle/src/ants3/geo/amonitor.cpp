#include "amonitor.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ajsontools.h"
#include "ajsontoolsroot.h"
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
    if (!time) return 0;
    return time->GetEntries();
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

//#include "ahistogram.h"
#include "ath.h"
#include <QJsonObject>
#include <QJsonArray>
void AMonitor::overrideDataFromJson(const QJsonObject &json)
{
    QJsonObject jEnergy = json["Energy"].toObject();
    update1D(jEnergy, energy);

    QJsonObject jAngle = json["Angle"].toObject();
    update1D(jAngle, angle);

    QJsonObject jTime = json["Time"].toObject();
    update1D(jTime, time);

    QJsonObject jSpatial = json["Spatial"].toObject();
    double xfrom = jSpatial["xfrom"].toDouble();
    double xto   = jSpatial["xto"].toDouble();
    double yfrom = jSpatial["yfrom"].toDouble();
    double yto   = jSpatial["yto"].toDouble();

    QJsonArray dataAr = jSpatial["data"].toArray();
    int ybins = dataAr.size();
    std::vector<std::vector<double>> dataVec;
    dataVec.resize(ybins);
    for (int iy=0; iy<ybins; iy++)
    {
        QJsonArray row = dataAr[iy].toArray();
        int xbins = row.size();
        dataVec[iy].resize(xbins);
        for (int ix=0; ix<xbins; ix++)
            dataVec[iy][ix] = row[ix].toDouble();
    }
    QJsonArray statAr = jSpatial["stat"].toArray();
    std::vector<double> statVec;
    for (int i=0; i<statAr.size(); i++)
        statVec.push_back(statAr[i].toDouble());
    ATH2D * hist = new ATH2D("", "", 100, 0, 1.0, 100, 0, 1.0);
    hist->Import(xfrom, xto, yfrom, yto, dataVec, statVec);
    delete xy; xy = hist;
}

void AMonitor::update1D(const QJsonObject & json, TH1D* & old)
{
    double from = json["from"].toDouble();
    double to =   json["to"].toDouble();

    QJsonArray dataAr = json["data"].toArray();
    std::vector<double> dataVec;
    for (int i=0; i<dataAr.size(); i++)
        dataVec.push_back(dataAr[i].toDouble());

    QJsonArray statAr = json["stat"].toArray();
    std::vector<double> statVec;
    for (int i=0; i<statAr.size(); i++)
        statVec.push_back(statAr[i].toDouble());

    double multiplier = 1.0;
    if (old == energy)
    {
        switch (config.energyUnitsInHist)
        {
        case 0:  multiplier = 1.0e6;  break;// keV -> meV
        case 1:  multiplier = 1.0e3;  break;// keV -> eV
        default: multiplier = 1.0;    break;// keV -> keV
        case 3:  multiplier = 1.0e-3; break;// keV -> MeV
        }
    }

    ATH1D * hist;
    if (old) hist = new ATH1D(*old); //to inherit all properties, including the axis titles
    else hist = new ATH1D("", "", 10,0,1);

    hist->Import(from * multiplier, to * multiplier, dataVec, statVec);
    delete old;
    old = hist;
}
