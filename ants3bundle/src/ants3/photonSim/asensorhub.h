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

    int countModels() const {return Models.size();}
    int countSensors() const {return SensorData.size();}
    int countSensorsOfModel(int iModel) const;

    ASensorModel       * model(int iModel);               // can return nullptr
    const ASensorModel * model(int iModel) const;         // can return nullptr

    QStringList getListOfModelNames() const;

    const ASensorModel * sensorModel(int iSensor) const; // can return nullptr
    const ASensorModel * sensorModelFast(int iSensor) const {return & Models[SensorData[iSensor].ModelIndex];}

    int     getModelIndex(int iSensor) const;

    void    clearSensors();
    void    registerNextSensor(ASensorData & sr);

    int     addNewModel();
    int     cloneModel(int iModel);

    void    clearAssignment();
    void    setSensorModel(int iSensor, int iModel);

    QString removeModel(int iModel);

    AVector3 getPosition(int iSensor) const;
    AVector3 getPositionFast(int iSensor) const;

    // !!!*** obsolete!
    double   getMinSize(int iSensor) const;
    double   getMinSizeFast(int iSensor) const;

    AGeoObject * getGeoObject(int iSensor) const;

    double  getMaxQE() const;                // !!!***
    double  getMaxQEvsWave(int iWave) const; // !!!***

    bool    isPersistentModelAssignment() const {return PersistentModelAssignment;}
    void    exitPersistentMode();

    bool    updateRuntimeProperties();

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

    bool PersistentModelAssignment = false;
    std::vector<int> LoadedModelAssignment;

    // runtime - populated together with GeoManager, updated by updateRuntimeProperties()
    std::vector<ASensorData> SensorData;

};

#endif // ASENSORHUB_H
