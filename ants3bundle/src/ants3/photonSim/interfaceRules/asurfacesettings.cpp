#include "asurfacesettings.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>

#include "TH1D.h"

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

        std::vector<double> edges(NormalDeviation.size() + 1);
        for (size_t i = 0; i < NormalDeviation.size(); i++)
        {
            edges[i] = NormalDeviation[i].first;
            if (i > 0 && edges[i-1] >= edges[i]) return "Custom distribution of microfacet normal for rough surface should be sorted in increasing order";
        }
        edges[NormalDeviation.size()] = edges[NormalDeviation.size()-1] + (edges[NormalDeviation.size()-1] - edges[NormalDeviation.size()-2]);

        NormalDistributionHist = new TH1D("", "", NormalDeviation.size(), edges.data());
        for (size_t i = 0; i < NormalDeviation.size(); i++)
            NormalDistributionHist->SetBinContent(i+1, NormalDeviation[i].second);

        NormalDistributionHist->ComputeIntegral();
    }

    return "";
}

QString ASurfaceSettings::getDescription() const
{
    QString txt;
    switch (Model)
    {
        case Polished :
            txt = "Polished surface:\n"
                  "all rougness details of the surface are ignored,\n"
                  "the global normal of the surface is used for photon tracing.";
            break;
        case Glisur :
            txt = "Glisur:\n"
                  "The surface is assumed to consist of spherical \"bulges\".\n"
                  "The roughness level is defined by the non-negative \"polish factor\":\n"
                  "when it is < 1, then a random point is generated on a sphere of\n"
                  "radius (1-polish), and the corresponding vector is added to the normal.\n"
                  "The value 0 means maximum roughness with effective plane of reflection\n"
                  "distributed as cos(α).";
            break;
        case Unified :
            txt = "Unified:\n"
                  "The rough surface is represented by \"microfacetes\" with\n"
                  "a certain distribution of the angle, α, between the microfacet's normal\n"
                  "and that of the average surface.\n"
                  "The model assumes that the probability of microfacet normals populates\n"
                  "the annulus of solid angle sin(α)dα is proportional to a Gaussian of\n"
                  "SigmaAlpha parameter.";
            break;
        case CustomNormal :
            txt = "CustomNormal:\n"
                  "The rough surface is represented by \"microfacetes\" with a\n"
                  "user-defined distribution of the angle, α, between the\n"
                  "microfacet's normal and that of the average surface.\n"
                  "Note that the provided probability distrbution of microfacet\n"
                  "normals should populate the annulus of solid angle sin(α)dα\n"
                  "(that is the effect of the solid angle should be corrected for\n"
                  "in the loaded distribution).";
            break;
    };
    return txt;
}

void ASurfaceSettings::writeToJson(QJsonObject & json) const
{
    json["KillPhotonsRefractedBackward"] = KillPhotonsRefractedBackward;

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
        json["OrientationProbabilityCorrection"] = OrientationProbabilityCorrection;
        break;
    }
    json["Model"] = str;
}

void ASurfaceSettings::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "KillPhotonsRefractedBackward", KillPhotonsRefractedBackward);

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
        jstools::parseJson(json, "OrientationProbabilityCorrection", OrientationProbabilityCorrection);
    }
    else
    {
        qWarning() << "Unknown optical surface model!";
        AErrorHub::addQError("Unknown optical surface model!");
        Model = Polished;
        return;
    }
}
