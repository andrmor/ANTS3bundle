#ifndef ACALORIMETERHUB_H
#define ACALORIMETERHUB_H

#include "avector.h"

#include <QString>
#include <QJsonObject>

#include <vector>

class ACalorimeter;
class QString;
class AGeoObject;

struct ACalorimeterData
{
    QString        Name;
    ACalorimeter * Calorimeter;
    AGeoObject   * GeoObj   = nullptr;
    AVector3       Position = {0, 0, 0};
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
    int  countCalorimetersWithHits() const;

    std::vector<const ACalorimeterData*> getCalorimeters(const AGeoObject * obj) const;  // returns nullptr if not found

    bool mergeCalorimeterFiles(const std::vector<QString> & inFiles, const QString & outFile);

    void writeDataToJson(QJsonObject & json) const;

    QString appendDataFromJson(const QJsonObject & json);
};

#endif // ACALORIMETERHUB_H
