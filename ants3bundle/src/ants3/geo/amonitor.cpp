#include "amonitor.h"
#include "ageoobject.h"
#include "ageotype.h"
#include "ajsontools.h"
#include "ajsontoolsroot.h"
#include "aroothistappenders.h"

#include <QDebug>

#include <vector>

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

void AMonitor::fillForPhoton(double x, double y, double Time, double Angle, int waveIndex)
{
    if (xy)    xy->Fill(x,y);
    if (time)  time->Fill(Time * timeFactor);
    if (angle) angle->Fill(Angle); // !!!*** check num bins == 1 then skip, put angle from cos calculation here
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
    // time
    {
        QJsonObject js;
            js["Data"]  = jstools::regularTh1dToJson(time);
            js["Units"] = config.timeUnits;
        json["Time"] = js;
    }

    json["Wave"] = jstools::regularTh1dToJson(wave);

    json["Angle"] = jstools::regularTh1dToJson(angle);

    // Energy
    {
        QJsonObject js;
            js["Data"]  = jstools::regularTh1dToJson(energy);
            js["Units"] = config.energyUnits;
        json["Energy"] = js;
    }

    json["XY"] = jstools::regularTh2dToJson(xy);
}

void AMonitor::readDataFromJson(const QJsonObject &json)
{
    clearData();

    // time
    {
        QJsonObject js;
        jstools::parseJson(json, "Time", js);
            jstools::parseJson(js, "Data", time);
            jstools::parseJson(js, "Units", config.timeUnits);
    }

    jstools::parseJson(json, "Wave", wave);

    jstools::parseJson(json, "Angle", angle);

    // energy
    {
        QJsonObject js;
        jstools::parseJson(json, "Energy", js);
            jstools::parseJson(js, "Data", energy);
            jstools::parseJson(js, "Units", config.energyUnits);
    }

    jstools::parseJson(json, "XY", xy);
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
    TString title = "Time, ";
    title += config.timeUnits.toLatin1().data();
    time->SetXTitle(title);

    timeFactor = 1.0;
    if      (config.timeUnits == "ns") timeFactor = 1.0;
    else if (config.timeUnits == "us") timeFactor = 1e-3;
    else if (config.timeUnits == "ms") timeFactor = 1e-6;
    else if (config.timeUnits == "s")  timeFactor = 1e-9;
    else qWarning() << "Bad timeUnits in monitor:" << config.timeUnits;
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
    energy = new TH1D("", "", config.energyBins, from, to);
    TString title = "Energy, ";
    title += config.energyUnits.toLatin1().data();
    energy->SetXTitle(title);
}

#include "ath.h"
#include <QJsonObject>
#include <QJsonArray>
void AMonitor::overrideDataFromJson(const QJsonObject &json)
{
    QJsonObject jEnergy = json["Energy"].toObject();
    jstools::parseJson(jEnergy, "Units", config.energyUnits);
    update1D(jEnergy, energy);

    QJsonObject jAngle = json["Angle"].toObject();
    update1D(jAngle, angle);

    QJsonObject jTime = json["Time"].toObject();
    jstools::parseJson(jTime, "Units", config.timeUnits);
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

    ATH1D * hist;
    if (old) hist = new ATH1D(*old); //to inherit all properties, including the axis titles
    else hist = new ATH1D("", "", 10,0,1);

    hist->Import(from, to, dataVec, statVec);
    delete old;
    old = hist;
}
