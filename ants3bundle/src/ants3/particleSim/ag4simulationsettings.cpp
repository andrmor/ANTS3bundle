#include "ag4simulationsettings.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

void AG4SimulationSettings::writeToJson(QJsonObject &json) const
{
    json["PhysicsList"] = QString(PhysicsList.data());

    QJsonArray arSV;
    for (auto & v : SensitiveVolumes)
        arSV.push_back(QString(v.data()));
    json["SensitiveVolumes"] = arSV;

    QJsonArray arC;
    for (auto & c : Commands)
        arC.push_back(QString(c.data()));
    json["Commands"] = arC;

    QJsonArray arSL;
    for (auto & key : StepLimits.keys())
    {
        QJsonArray el;
        el << key << StepLimits.value(key);
        arSL.push_back(el);
    }
    json["StepLimits"] = arSL;

    json["UseTSphys"]     = UseTSphys;
}

void AG4SimulationSettings::readFromJson(const QJsonObject &json)
{
    clear();

    jstools::parseJson(json, "PhysicsList", PhysicsList);

    QJsonArray arSV;
    SensitiveVolumes.clear();
    jstools::parseJson(json, "SensitiveVolumes", arSV);
    for (int i=0; i<arSV.size(); i++)
        SensitiveVolumes.push_back( arSV.at(i).toString().toLatin1().data() );

    QJsonArray arC;
    Commands.clear();
    jstools::parseJson(json, "Commands", arC);
    for (int i=0; i<arC.size(); i++)
        Commands.push_back( arC.at(i).toString().toLatin1().data() );

    QJsonArray arSL;
    StepLimits.clear();
    jstools::parseJson(json, "StepLimits", arSL);
    for (int i=0; i<arSL.size(); i++)
    {
        QJsonArray el = arSL[i].toArray();
        if (el.size() > 1)
        {
            QString vol = el[0].toString();
            double step = el[1].toDouble();
            StepLimits[vol] = step;
        }
    }

    jstools::parseJson(json, "UseTSphys", UseTSphys);
}

void AG4SimulationSettings::clear()
{
    PhysicsList = "QGSP_BERT_HP";
    SensitiveVolumes.clear();
    Commands = {"/run/setCut 0.7 mm"};
    StepLimits.clear();
    UseTSphys = false;
}

/*
const QString AG4SimulationSettings::getPrimariesFileName(int iThreadNum) const
{
    return getPath() + QString("primaries-%1.txt").arg(iThreadNum);
}

const QString AG4SimulationSettings::getDepositionFileName(int iThreadNum) const
{
    return getPath() + QString("deposition-%1.%2").arg(iThreadNum).arg(BinaryOutput ? "bin" : "txt");
}

const QString AG4SimulationSettings::getReceitFileName(int iThreadNum) const
{
    return getPath() + QString("receipt-%1.json").arg(iThreadNum);
}

const QString AG4SimulationSettings::getConfigFileName(int iThreadNum) const
{
    return getPath() + QString("config-%1.json").arg(iThreadNum);
}

const QString AG4SimulationSettings::getTracksFileName(int iThreadNum) const
{
    return getPath() + QString("tracks-%1.%2").arg(iThreadNum).arg(BinaryOutput ? "bin" : "txt");
}

const QString AG4SimulationSettings::getMonitorDataFileName(int iThreadNum) const
{
    return getPath() + QString("monitors-%1.json").arg(iThreadNum);
}

const QString AG4SimulationSettings::getExitParticleFileName(int iThreadNum) const
{
    return getPath() + QString("exits-%1.dat").arg(iThreadNum);
}


bool AG4SimulationSettings::checkPathValid() const
{
    const QString & path = AGlobalSettings::getInstance().G4ExchangeFolder;
    if (path.isEmpty()) return false;
    return QDir(path).exists();
}

bool AG4SimulationSettings::checkExecutableExists() const
{
    const QString & exec = AGlobalSettings::getInstance().G4antsExec;
    return QFile::exists(exec);
}

bool AG4SimulationSettings::checkExecutablePermission() const
{
    const QString & exec = AGlobalSettings::getInstance().G4antsExec;
    return QFileInfo(exec).isExecutable();
}

#include "TGeoManager.h"
const QString AG4SimulationSettings::checkSensitiveVolumes() const
{
    if (SensitiveVolumes.isEmpty()) return ""; //can be empty

    QStringList NotFoundVolumes;
    TObjArray * va = gGeoManager->GetListOfVolumes();
    const int numVol = va->GetEntries();

    for (int i=0; i<SensitiveVolumes.size(); i++)
    {
        QString s = SensitiveVolumes.at(i);

        bool bWild = false;
        if (s.endsWith('*'))
        {
            s.chop(1);
            bWild = true;
        }

        bool bFound = false;
        for (int iV=0; iV<numVol; iV++)
        {
            const TString tname = ((TGeoVolume*)va->At(iV))->GetName();
            const QString sname(tname.Data());

            if (bWild)
            {
                if (sname.startsWith(s))
                {
                    bFound = true;
                    break;
                }
            }
            else
            {
                if (sname == s)
                {
                    bFound = true;
                    break;
                }
            }
        }
        if (!bFound) NotFoundVolumes << s;
    }

    if (NotFoundVolumes.isEmpty()) return "";
    else return QString("The following sensitive volumes/widlcards do not identify any volumes in the geometry:\n%1").arg(NotFoundVolumes.join(", "));
}

const QString AG4SimulationSettings::getPath() const
{
    QString Path = AGlobalSettings::getInstance().G4ExchangeFolder;
    if (!Path.endsWith('/')) Path += '/';
    return Path;
}
*/
