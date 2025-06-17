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
    json["ApplyScaling"] = ApplyScaling;
    json["ScalingFactor"] = ScalingFactor;
    json["Palette"] = Palette;
    json["SuppressZero"] = SuppressZero;

    json["ApplyAdjacentAveraging"] = ApplyAdjacentAveraging;
    QJsonArray ar;
    for (const auto & pair : AdjacentBeforeAfter)
    {
        QJsonArray el;
            el.push_back(pair.first);
            el.push_back(pair.second);
        ar.push_back(el);
    }
    json["AdjacentBeforeAfter"] = ar;

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
    jstools::parseJson(json, "ApplyScaling", ApplyScaling);
    jstools::parseJson(json, "ScalingFactor", ScalingFactor);
    jstools::parseJson(json, "Palette", Palette);
    jstools::parseJson(json, "SuppressZero", SuppressZero);

    jstools::parseJson(json, "ApplyAdjacentAveraging", ApplyAdjacentAveraging);
    QJsonArray ar;
    jstools::parseJson(json, "AdjacentBeforeAfter", ar);
    for (int i = 0; i < 3 && i < ar.size(); i++)
    {
        QJsonArray el = ar[i].toArray();
        if (el.size() == 2)
            AdjacentBeforeAfter[i] = {el[0].toInt(), el[1].toInt()};
        else
            AdjacentBeforeAfter[i] = {0,0};
    }

    jstools::parseJson(json, "TitleVisible", TitleVisible);
    jstools::parseJson(json, "ShowPositionLines", ShowPositionLines);
}
