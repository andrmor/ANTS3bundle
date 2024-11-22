#include "asurfacesettings.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>

#include "TH1D.h"
#include "TVectorD.h"

ASurfaceSettings::ASurfaceSettings()
{
    delete NormalDistributionHist; NormalDistributionHist = nullptr;
}

QString ASurfaceSettings::checkRuntimeData()
{
    if (Model == CustomNormal)
    {
        delete NormalDistributionHist; NormalDistributionHist = nullptr;
        if (NormalDeviation.size() < 2) return "Custom distribution of microfacet normal for rough surface should contain at least two points";

        TVectorD edges(NormalDeviation.size());
        for (size_t i = 0; i < NormalDeviation.size(); i++)
        {
            edges[i] = NormalDeviation[i].first;
            if (i > 0 && edges[i-1] >= edges[i]) return "Custom distribution of microfacet normal for rough surface should be sorted in increasing order";
        }

        NormalDistributionHist = new TH1D(edges);
        for (size_t i = 0; i < NormalDeviation.size()-1; i++) // ignore last bin, it is used to define the edge only
            NormalDistributionHist->SetBinContent(i+1, NormalDeviation[i].second);
    }

    return "";
}

void ASurfaceSettings::writeToJson(QJsonObject & json) const
{
    QString str;
    switch (Model)
    {
    default:
    case Polished :
        str = "Polished";
        break;
    case Glisur :
        str = "Glisur";
        json["Polish"] = Polish;
        break;
    case Unified :
        str = "Unified";
        json["SigmaAlpha"] = SigmaAlpha;
        break;
    case CustomNormal :
        str = "CustomNormal";
        {
            QJsonArray ar;
            jstools::writeDPairVectorToArray(NormalDeviation, ar);
            json["NormalDeviation"] = ar;
        }
        break;
    }
    json["Model"] = str;
}

void ASurfaceSettings::readFromJson(const QJsonObject & json)
{
    QString str;
    jstools::parseJson(json, "Model", str);
    if      (str == "Polished")        Model = Polished;
    else if (str == "Glisur")
    {
        Model = Glisur;
        jstools::parseJson(json, "Polish", Polish);
    }
    else if (str == "Unified")
    {
        Model = Unified;
        jstools::parseJson(json, "SigmaAlpha", SigmaAlpha);
    }
    else if (str == "CustomNormal")
    {
        Model = CustomNormal;
        {
            QJsonArray ar;
            jstools::parseJson(json, "NormalDeviation", ar);
            jstools::readDPairVectorFromArray(ar, NormalDeviation);
        }
    }
    else
    {
        qWarning() << "Unknown optical surface model!";
        AErrorHub::addQError("Unknown optical surface model!");
        Model = Polished;
        return;
    }
}
