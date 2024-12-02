#ifndef ASURFACESETTINGS_H
#define ASURFACESETTINGS_H

#include <vector>

#include <QString>

class QJsonObject;
class TH1D;

class ASurfaceSettings
{
public:
    enum EModel {Polished, Glisur, Unified, CustomNormal};

    ASurfaceSettings();

    bool isPolished()    const {return Model == Polished;}
    bool isNotPolished() const {return Model != Polished;}

    QString checkRuntimeData(); // also populates NormalDistributionHist

    QString getDescription() const;

    EModel Model = Glisur;

    //Glisur model settings
    double Polish = 0.85;

    //Unified model settings
    double SigmaAlpha = 0.1;

    //CustomNormal settings
    std::vector<std::pair<double,double>> NormalDeviation;
    bool OrientationProbabilityCorrection = true;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    // Run-time data
    TH1D * NormalDistributionHist = nullptr;
};

#endif // ASURFACESETTINGS_H
