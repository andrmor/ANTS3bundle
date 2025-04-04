#ifndef APHOTONFUNCTIONALMODEL_H
#define APHOTONFUNCTIONALMODEL_H

#include <QString>

#include <vector>

class QJsonObject;
class AGeoObject;

class APhotonExchangeData
{
public:
    double LocalPosition[3];
    double LocalDirection[3];
    int    WaveIndex;
    double Time;
};

class APhotonFunctionalModel
{
public:
    APhotonFunctionalModel(){}
    virtual ~APhotonFunctionalModel(){}

    virtual QString getType() const = 0;
    virtual bool isLink() const {return false;}  // true: teleports photons to another object, so needs two indexes

    virtual QString checkModel() const {return "";}

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    virtual void writeSettingsToJson(QJsonObject & /*json*/) const {}
    virtual void readSettingsFromJson(const QJsonObject & /*json*/) {}

    virtual QString printSettingsToString() const = 0; // used in gui / scripting

    virtual QString updateRuntimeProperties() {return "";}

    virtual bool applyModel(APhotonExchangeData & photonData, int index, int linkedToIndex) = 0;
    // photonData on call contains Trigger data, on return should return data for Target
    // true = continue tracing; false = kill this photon

    static QStringList getKnownModels();
    static APhotonFunctionalModel * factory(const QString & type);
    static APhotonFunctionalModel * factory(QJsonObject & json);
    static APhotonFunctionalModel * clone(const APhotonFunctionalModel * other);
};

class APFM_Dummy : public APhotonFunctionalModel
{
    QString getType() const override {return QStringLiteral("Dummy");}

    QString printSettingsToString() const override {return "";}

    bool applyModel(APhotonExchangeData &, int, int) override {return true;}
};

class APFM_OpticalFiber : public APhotonFunctionalModel
{
public:
    APFM_OpticalFiber(){}

    QString getType() const override {return QStringLiteral("OpticalFiber");}
    bool isLink() const override {return true;}

    QString checkModel() const override;

    void writeSettingsToJson(QJsonObject & json) const override;
    void readSettingsFromJson(const QJsonObject & json) override;

    QString printSettingsToString() const override;

    QString updateRuntimeProperties() override;

    bool applyModel(APhotonExchangeData & photonData, int index, int linkedToIndex) override;

    double Length_mm = 100.0;

    double MaxAngle_deg = 30.0;
    std::vector<std::pair<double,double>> MaxAngleSpectrum_deg;
    // refractive index and and attenuation data are taken from the target material   --> !!!*** in check enforce same material target and trigger

    // runtime
    double _TanMaxAngle;
    std::vector<double> _TanMaxAngleSpectrumBinned;
};

class APFM_ThinLens : public APhotonFunctionalModel
{
public:
    QString getType() const override {return QStringLiteral("ThinLens");}

    QString checkModel() const override;

    void writeSettingsToJson(QJsonObject & json) const override;
    void readSettingsFromJson(const QJsonObject & json) override;

    QString printSettingsToString() const override;

    QString updateRuntimeProperties() override;

    bool applyModel(APhotonExchangeData & photonData, int index, int linkedToIndex) override;

    double FocalLength_mm = 10.0;
    std::vector<std::pair<double,double>> FocalLengthSpectrum_mm;

    // consider adding loss factor to the model

    // !!!*** isValid() ?

    // runtime
    std::vector<double> _FocalLengthBinned;
};

class APFM_Filter : public APhotonFunctionalModel
{
public:
    QString getType() const override {return QStringLiteral("Filter");}

    QString checkModel() const override;

    void writeSettingsToJson(QJsonObject & json) const override;
    void readSettingsFromJson(const QJsonObject & json) override;

    QString printSettingsToString() const override;

    QString updateRuntimeProperties() override;

    bool applyModel(APhotonExchangeData & photonData, int index, int linkedToIndex) override;

    bool   Gray = true;
    double GrayTransmission = 0.5;
    std::vector<std::pair<double,double>> TransmissionSpectrum;

    // runtime
    std::vector<double> _TransmissionBinned;
};

#endif // APHOTONFUNCTIONALMODEL_H
