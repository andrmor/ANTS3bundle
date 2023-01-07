#ifndef APHOTONSIMHUB_H
#define APHOTONSIMHUB_H

#include "aphotonsimsettings.h"

#include <QObject>
#include <QString>

class APhotonSimHub final : public QObject
{
    Q_OBJECT

public:
    static       APhotonSimHub & getInstance();
    static const APhotonSimHub & getConstInstance();

private:
    APhotonSimHub(){}
    ~APhotonSimHub(){}

    APhotonSimHub(const APhotonSimHub&)            = delete;
    APhotonSimHub(APhotonSimHub&&)                 = delete;
    APhotonSimHub& operator=(const APhotonSimHub&) = delete;
    APhotonSimHub& operator=(APhotonSimHub&&)      = delete;

public:
    APhotonSimSettings Settings;

    void    writeToJson(QJsonObject & json, bool saveRunSet) const;
    QString readFromJson(const QJsonObject & json);

    void    clear();

signals:
    void settingsChanged();
};

#endif // APHOTONSIMHUB_H
