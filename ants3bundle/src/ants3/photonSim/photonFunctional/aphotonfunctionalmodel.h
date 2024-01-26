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

    virtual QString getType() const = 0;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    virtual void writeSettingsToJson(QJsonObject & json) const = 0;
    virtual void readSettingsFromJson(const QJsonObject & json) = 0;

    virtual QString printSettingsToString() const = 0; // used in gui / scripting

    virtual void updateRuntimeProperties() {} // can report errors throuh AErrorHub

    virtual bool applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target) = 0;
    // photonData on call contains Trigger data, on return should return data for Target
    // true = continue tracing; false = kill this photon
};

class APFM_OpticalFiber : public APhotonFunctionalModel
{
public:
    APFM_OpticalFiber(){}

    QString getType() const override {return QStringLiteral("OpticalFiber");}

    void writeSettingsToJson(QJsonObject & json) const override;
    void readSettingsFromJson(const QJsonObject & json) override;

    QString printSettingsToString() const override;

    bool applyModel(APhotonExchangeData & photonData, const AGeoObject * trigger, const AGeoObject * target) override;

    double Length_mm = 100.0;
    double MaxAngle_deg = 30.0;
    // refractive index and and attenuation data are taken from the target material   --> !!!*** in check enforce same material target and trigger

    // runtime
};

#endif // APHOTONFUNCTIONALMODEL_H
