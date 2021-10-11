#ifndef AMONITOR_H
#define AMONITOR_H

#include "amonitorconfig.h"

#include <QString>
#include <vector>

class TH1D;
class ATH1D;
class TH2D;
class AGeoObject;
class QJsonObject;

class AMonitor
{
public:
  AMonitor();
  AMonitor(const AGeoObject* MonitorGeoObject);
  ~AMonitor();

//runtime functions
  void fillForParticle(double x, double y, double Time, double Angle, double Energy);
  void fillForPhoton(double x, double y, double Time, double Angle, int waveIndex);

  bool isForPhotons() const         {return config.PhotonOrParticle == 0;}
  bool isForParticles() const       {return config.PhotonOrParticle != 0;}
  bool isUpperSensitive() const     {return config.bUpper;}
  bool isLowerSensitive() const     {return config.bLower;}
  bool isStopsTracking() const      {return config.bStopTracking;}
  QString getParticleName() const   {return config.Particle;}
  bool isAcceptingPrimary() const   {return config.bPrimary;}
  bool isAcceptingSecondary() const {return config.bSecondary;}
  bool isAcceptingDirect() const    {return config.bDirect;}
  bool isAcceptingIndirect() const  {return config.bIndirect;}

//configuration
  bool readFromGeoObject(const AGeoObject* MonitorRecord);

  void writeDataToJson(QJsonObject & json) const;
  void readDataFromJson(const QJsonObject & json);

// stat data handling
  void clearData();

  QString name;

  TH1D * time   = nullptr;
  TH2D * xy     = nullptr;
  TH1D * angle  = nullptr;
  TH1D * wave   = nullptr;
  TH1D * energy = nullptr;

  AMonitorConfig config;

  int getHits() const;

  void append(const AMonitor & from);

  void overrideDataFromJson(const QJsonObject & json);

private:
  void initXYHist();
  void initTimeHist();
  void initWaveHist();
  void initAngleHist();
  void initEnergyHist();

  void update1D(const QJsonObject &json, TH1D *&old);
};

#endif // AMONITOR_H
