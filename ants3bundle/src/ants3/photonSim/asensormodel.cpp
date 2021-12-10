#include "asensormodel.h"
#include "ajsontools.h"

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

    QJsonObject sij;
        sij["isSiPM"]  = SiPM;
        sij["PixelsX"] = PixelsX;
        sij["PixelsY"] = PixelsY;
    json["SiPM"] = sij;

    QJsonObject pdej;
        pdej["Effective"] = PDE_effective;
        QJsonArray ar;
//            writeTwoQVectorsToJArray(PDE_lambda, PDE, ar);
        pdej["Spectral"] = ar;
    json["PDE"] = pdej;

    QJsonObject angj;
        angj["InterfaceN"] = InterfaceN;
        QJsonArray ar1;
//        writeTwoQVectorsToJArray(AngularSensitivity_lambda, AngularSensitivity, ar1);
        angj["Data"] = ar1;
    json["AngularResponse"] = angj;

    QJsonObject areaj;
        areaj["StepX"] = StepX;
        areaj["StepY"] = StepY;
        QJsonArray arar;
//        write2DQVectorToJArray(AreaSensitivity, arar);
        areaj["Data"] = arar;
    json["AreaResponse"] = areaj;
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
//        readTwoQVectorsFromJArray(par, PDE_lambda, PDE);

    QJsonObject angj = json["AngularResponse"].toObject();
        jstools::parseJson(angj, "InterfaceN", InterfaceN);
        QJsonArray aar;
        jstools::parseJson(angj, "Data", aar);
//        readTwoQVectorsFromJArray(aar, AngularSensitivity_lambda, AngularSensitivity);

    QJsonObject areaj = json["AreaResponse"].toObject();
        jstools::parseJson(areaj, "AreaStepX", StepX);
        jstools::parseJson(areaj, "AreaStepY", StepY);
        QJsonArray arar;
        jstools::parseJson(areaj, "Data", arar);
//        read2DQVectorFromJArray(ar, AreaSensitivity);

    return true;
}

double ASensorModel::getPDE(int iWave) const
{
    if (iWave == -1 || PDEbinned.empty()) return PDE_effective;
    return PDEbinned[iWave];
}

void ASensorModel::updateRuntimeProperties()
{
    PDEbinned.clear();
    AngularSensitivityCosRefracted.clear();
}
