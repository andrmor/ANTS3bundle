#include "asensormodel.h"
#include "ajsontools.h"

#include <tuple>

#include "TMath.h"

void ASensorModel::clear()
{
    Name = "_Undefined_";

    SiPM = false;
    PixelsX = 50;
    PixelsY = 50;

    PDE_effective = 1.0;
    PDE_spectral.clear();
    PDEbinned.clear();

    AngularFactors.clear();
    InterfaceN = 1.0;
    AngularSensitivityCosRefracted.clear();

    StepX = 1.0;
    StepY = 1.0;
    XYFactors.clear();

    DarkCountRate = 0;
}

void ASensorModel::writeToJson(QJsonObject & json) const
{
    json["Name"] = Name;

    json["DarkCountRate"] = DarkCountRate;

    {
        QJsonObject js;
            js["isSiPM"]  = SiPM;
            js["PixelsX"] = PixelsX;
            js["PixelsY"] = PixelsY;
        json["SiPM"] = js;
    }

    {
        QJsonObject js;
            js["Effective"] = PDE_effective;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(PDE_spectral, ar);
            js["Spectral"] = ar;
        json["PDE"] = js;
    }

    {
        QJsonObject js;
            js["InterfaceN"] = InterfaceN;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(AngularFactors, ar);
            js["Data"] = ar;
        json["AngularResponse"] = js;
    }

    {
        QJsonObject js;
            js["StepX"] = StepX;
            js["StepY"] = StepY;
            QJsonArray ar;
                jstools::writeDVectorOfVectorsToArray(XYFactors, ar);
            js["Data"] = ar;
        json["AreaResponse"] = js;
    }
}

bool ASensorModel::readFromJson(const QJsonObject & json)
{
    clear();

    if (!json.contains("Name") || !json.contains("SiPM")) return false; // simple check of format

    jstools::parseJson(json, "Name",          Name);
    jstools::parseJson(json, "DarkCountRate", DarkCountRate);

    QJsonObject sij = json["SiPM"].toObject();
        jstools::parseJson(sij, "isSiPM",  SiPM);
        jstools::parseJson(sij, "PixelsX", PixelsX);
        jstools::parseJson(sij, "PixelsY", PixelsY);

    QJsonObject pdej = json["PDE"].toObject();
        jstools::parseJson(pdej, "Effective", PDE_effective);
        QJsonArray par;
        jstools::parseJson(pdej, "Spectral", par);
        bool ok = jstools::readDPairVectorFromArray(par, PDE_spectral);
        if (!ok) return false; // !!!***

    QJsonObject angj = json["AngularResponse"].toObject();
        jstools::parseJson(angj, "InterfaceN", InterfaceN);
        QJsonArray aar;
        jstools::parseJson(angj, "Data", aar);
        ok = jstools::readDPairVectorFromArray(aar, AngularFactors);
        if (!ok) return false; // !!!***

    QJsonObject areaj = json["AreaResponse"].toObject();
        jstools::parseJson(areaj, "AreaStepX", StepX);
        jstools::parseJson(areaj, "AreaStepY", StepY);
        QJsonArray arar;
        jstools::parseJson(areaj, "Data", arar);
        ok = jstools::readDVectorOfVectorsFromArray(arar, XYFactors);
        if (!ok) return false; // !!!***

    return true;
}

double ASensorModel::getPDE(int iWave) const
{
    if (iWave == -1 || PDEbinned.empty()) return PDE_effective;
    return PDEbinned[iWave];
}

#include "aphotonsimhub.h"
void ASensorModel::updateRuntimeProperties()
{
    PDEbinned.clear();
    AngularSensitivityCosRefracted.clear();

    const APhotonSimSettings SimSet = APhotonSimHub::getConstInstance().Settings;

    if (!PDE_spectral.empty()) SimSet.WaveSet.toStandardBins(PDE_spectral, PDEbinned);
}
