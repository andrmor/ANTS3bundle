#ifndef AVIEWER3DSETTINGS_H
#define AVIEWER3DSETTINGS_H

#include <vector>
#include <array>

#include <QString>

class QJsonObject;

class AViewer3DSettings
{
public:
    AViewer3DSettings();

    enum EMaximumMode {IndividualMax, GlobalMax, FixedMax};

    EMaximumMode MaximumMode = IndividualMax;
    double       FixedMaximum = 0;
    int          PercentFieldOfView = 90;

    bool         ApplyScaling = false;
    double       ScalingFactor = 1.0;
    bool         SuppressZero = false;

    QString      Palette = "Default ROOT";

    bool         TitleVisible = false;
    bool         ShowPositionLines = false;

    bool         ApplyAdjacentAveraging = false;
    std::array<std::pair<int,int>, 3> AdjacentBeforeAfter;

    std::vector<std::pair<QString,int>> DefinedPalettes;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // AVIEWER3DSETTINGS_H
