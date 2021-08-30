#ifndef AG4SIMULATIONSETTINGS_H
#define AG4SIMULATIONSETTINGS_H

#include <QString>
#include <QStringList>
#include <QMap>

class QJsonObject;

class AG4SimulationSettings
{
public:
    AG4SimulationSettings();

    QString               PhysicsList = "QGSP_BERT_HP";
    QStringList           SensitiveVolumes;
    QStringList           Commands = QStringList({"/run/setCut 0.7 mm"});
    QMap<QString, double> StepLimits;

    bool                  UseTSphys = false;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    void clear();


/*
    const QString getPrimariesFileName(int iThreadNum) const;
    const QString getDepositionFileName(int iThreadNum) const;
    const QString getReceitFileName(int iThreadNum) const;
    const QString getConfigFileName(int iThreadNum) const;
    const QString getTracksFileName(int iThreadNum) const;
    const QString getMonitorDataFileName(int iThreadNum) const;
    const QString getExitParticleFileName(int iThreadNum) const;

    bool  checkPathValid() const;
    bool  checkExecutableExists() const;
    bool  checkExecutablePermission() const;

    const QString checkSensitiveVolumes() const;  // move to sim manager

private:
    const QString getPath() const;
*/

};

#endif // AG4SIMULATIONSETTINGS_H
