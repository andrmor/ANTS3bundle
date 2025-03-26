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

    bool isPolished() const {return Model == Polished;}
    bool isRough() const {return Model != Polished;}

    QString checkRuntimeData(); // also populates NormalDistributionHist   !!!*** separate funcionality: check vs updateRuntimeProperties

    QString getDescription() const;

    EModel Model = Glisur;

    // Glisur model settings
    double Polish = 0.85;

    // Unified model settings
    double SigmaAlpha = 0.1;

    // CustomNormal settings
    std::vector<std::pair<double,double>> NormalDeviation;

    // General settings
    bool OrientationProbabilityCorrection = true;
    bool KillPhotonsRefractedBackward = false; // affects nly rough surface. There is a possibility for a photon to refract through a microfacet in the backward direction

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    // Run-time data
    TH1D * NormalDistributionHist = nullptr;
};

#endif // ASURFACESETTINGS_H
