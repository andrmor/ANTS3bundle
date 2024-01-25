#ifndef APHOTONFUNCTIONALMODEL_H
#define APHOTONFUNCTIONALMODEL_H

#include <QString>

class QJsonObject;
class AGeoObject;

class APhotonExchangeData
{
public:
    double LocalPosition[3];
    double LocalDirection[3];
    int    WaveIndex;
    int    Time;
};

class APhotonFunctionalModel
{
public:
    APhotonFunctionalModel(){}
    virtual ~APhotonFunctionalModel(){}

    virtual QString getType() = 0;

    virtual void writeToJson(QJsonObject & json) const = 0;
    virtual void readFromJson(const QJsonObject & json) = 0;

    virtual bool applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target) = 0;
    // photonData on call contains Trigger data, on return should contain data for Target
    // false = kill photon
};

class APFM_OpticalFiber : public APhotonFunctionalModel
{
public:
    APFM_OpticalFiber(){}

    QString getType() override {return QStringLiteral("OpticalFiber");}

    void writeToJson(QJsonObject & json) const override;
    void readFromJson(const QJsonObject & json) override;

    bool applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target) override;

    // length
    // refractive index?
    // attenuation per meter
    // acceptance angle
};

#endif // APHOTONFUNCTIONALMODEL_H
