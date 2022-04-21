#ifndef ACALORIMETERHUB_H
#define ACALORIMETERHUB_H

#include "avector.h"

#include <QString>
#include <QStringList>

#include <vector>

class ACalorimeter;
class QString;
class AGeoObject;
class QJsonArray;
class TGeoNode;

struct ACalorimeterData
{
    QString        Name;
    ACalorimeter * Calorimeter;
    AGeoObject   * GeoObj   = nullptr;
    AVector3       Position = {0, 0, 0};

    double         UnitXMaster[3];
    double         UnitYMaster[3];
    double         UnitZMaster[3];
};

class ACalorimeterHub
{
public:
    static ACalorimeterHub & getInstance();
    static const ACalorimeterHub & getConstInstance();

private:
    ACalorimeterHub(){}
    ~ACalorimeterHub();

    ACalorimeterHub(const ACalorimeterHub&)            = delete;
    ACalorimeterHub(ACalorimeterHub&&)                 = delete;
    ACalorimeterHub& operator=(const ACalorimeterHub&) = delete;
    ACalorimeterHub& operator=(ACalorimeterHub&&)      = delete;

public:
    std::vector<ACalorimeterData> Calorimeters;

    void clear();
    void clearData();

    int  countCalorimeters() const;
    int  countCalorimetersWithData() const;

    QStringList getCalorimeterNames() const;

    std::vector<const ACalorimeterData*> getCalorimeters(const AGeoObject * obj) const;  // returns nullptr if not found

    bool mergeCalorimeterFiles(const std::vector<QString> & inFiles, const QString & outFile);

    void writeDataToJson(QJsonArray & ar) const;

    QString appendDataFromJson(const QJsonArray & ar); // !!!*** AErrorHub
};

#endif // ACALORIMETERHUB_H
