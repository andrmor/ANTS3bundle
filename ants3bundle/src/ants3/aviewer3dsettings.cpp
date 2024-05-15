#include "aviewer3dsettings.h"
#include "ajsontools.h"

AViewer3DSettings::AViewer3DSettings()
{
    DefinedPalettes = { {"Default ROOT",57},
                        {"Deep sea",51},
                        {"Grey scale",52},
                        {"Dark body radiator",53},
                        {"Two color hue",54},
                        {"Rainbow",55},
                        {"Inverted dark body",56} };
}

void AViewer3DSettings::writeToJson(QJsonObject & json) const
{
    json["MaximumMode"] = (int)MaximumMode;
    json["FixedMaximum"] = FixedMaximum;
    json["PercentFieldOfView"] = PercentFieldOfView;
    json["ScalingFactor"] = ScalingFactor;
    json["Palette"] = Palette;
    json["SuppressZero"] = SuppressZero;

    json["TitleVisible"] = TitleVisible;
    json["ShowPositionLines"] = ShowPositionLines;
}

void AViewer3DSettings::readFromJson(const QJsonObject & json)
{
    // GUI
    int iMode = 0;
    jstools::parseJson(json, "MaximumMode", iMode);
    MaximumMode = static_cast<EMaximumMode>(iMode);

    jstools::parseJson(json, "FixedMaximum", FixedMaximum);
    jstools::parseJson(json, "PercentFieldOfView", PercentFieldOfView);
    jstools::parseJson(json, "ScalingFactor", ScalingFactor);
    jstools::parseJson(json, "Palette", Palette);
    jstools::parseJson(json, "SuppressZero", SuppressZero);

    jstools::parseJson(json, "TitleVisible", TitleVisible);
    jstools::parseJson(json, "ShowPositionLines", ShowPositionLines);
}
