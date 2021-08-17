#ifndef ASENSORHUB_H
#define ASENSORHUB_H

#include "asensormodel.h"
#include "avector.h"

#include <QString>
#include <QStringList>

#include <vector>

class AGeoObject;
class QJsonObject;

struct ASensorData
{
    AGeoObject * GeoObj     = nullptr;
    int          ModelIndex = 0;
    AVector3     Position   = {0, 0, 0};
};

class ASensorHub
{
public:
    static ASensorHub & getInstance();
    static const ASensorHub & getConstInstance();

    int countSensors() const {return SensorData.size();}
    int countSensorModels() const {return Models.size();}

    const QStringList getListOfModelNames() const;

    //temporary stubs
    const ASensorModel & getModelFast(int iModel) const {return Models.front();}
    bool isSiPM(int index) const {return false;}// !!!***
    bool isAngularResolvedPDE(int index) const {return false;}// !!!***
    bool isAreaResolvedPDE(int index) const {return false;}// !!!***

    double getMaxQE() const;                // !!!***
    double getMaxQEvsWave(int iWave) const; // !!!***

    void    writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

private:
    ASensorHub();
    ~ASensorHub(){}

    ASensorHub(const ASensorHub&)            = delete;
    ASensorHub(ASensorHub&&)                 = delete;
    ASensorHub& operator=(const ASensorHub&) = delete;
    ASensorHub& operator=(ASensorHub&&)      = delete;

private:
    std::vector<ASensorModel> Models;

public:
    // runtime - populated together with GeoManager
    std::vector<ASensorData> SensorData;

};

#endif // ASENSORHUB_H
