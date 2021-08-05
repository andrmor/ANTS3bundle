#ifndef ASENSORHUB_H
#define ASENSORHUB_H

#include <QString>
#include <QStringList>

#include <vector>

class AGeoObject;
class QJsonObject;

class ASensorModel
{
public:
    QString Name = "Undefined";
    double  PDE  = 1.0;
};

class ASensorHub
{
public:
    static ASensorHub & getInstance();
    static const ASensorHub & getConstInstance();

    int countSensors() const {return Sensors.size();}
    int countSensorModels() const {return Models.size();}

    const ASensorModel * getModelFast(int iModel) const;

    const QStringList getListOfModelNames() const;

    //temporary stubs
    bool isSiPM(int index) const {return false;}
    bool isAngularResolvedPDE(int index) const {return false;}
    bool isAreaResolvedPDE(int index) const {return false;}

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
    std::vector<AGeoObject*> Sensors;

};

#endif // ASENSORHUB_H
