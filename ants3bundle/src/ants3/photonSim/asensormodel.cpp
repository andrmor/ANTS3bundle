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

    AngleSensitive = false;
    AngularFactors.clear();
    AngularSensitivityCosRefracted.clear();
    InterfaceN = 1.0;

    XYSensitive = false;
    XYFactors.clear();
    StepX = 1.0;
    StepY = 1.0;

    DarkCountRate = 0;
}

void ASensorModel::writeToJson(QJsonObject & json) const
{
    QJsonObject genj;
        genj["Name"] = Name;
        genj["DarkCountRate"] = DarkCountRate;
    json["General"] = genj;

    QJsonObject sij;
        sij["SiPM"]    = SiPM;
        sij["PixelsX"] = PixelsX;
        sij["PixelsY"] = PixelsY;
    json["SiPMproperties"] = sij;

    QJsonObject pdej;
        pdej["Effective"] = PDE_effective;
        pdej["WaveSensitive"] = WaveSensitive;
        QJsonArray ar;
//            writeTwoQVectorsToJArray(PDE_lambda, PDE, ar);
        pdej["Spectral"] = ar;
    json["PDE"] = pdej;

    QJsonObject angj;
        angj["AngleSensitive"] = AngleSensitive;
        angj["InterfaceN"] = InterfaceN;
        QJsonArray ar1;
//        writeTwoQVectorsToJArray(AngularSensitivity_lambda, AngularSensitivity, ar1);
        angj["Data"] = ar1;
    json["AngularResponse"] = angj;

    QJsonObject areaj;
        areaj["XYSensitive"] = XYSensitive;
        areaj["StepX"] = StepX;
        areaj["StepY"] = StepY;
        QJsonArray arar;
//        write2DQVectorToJArray(AreaSensitivity, arar);
        areaj["Data"] = arar;
    json["AreaResponse"] = areaj;
}

void ASensorModel::readFromJson(const QJsonObject &json)
{
    clear();

    QJsonObject genj = json["General"].toObject();
    jstools::parseJson(genj, "Name", Name);
    jstools::parseJson(genj, "SiPM", SiPM);

    QJsonObject sij = json["SiPMproperties"].toObject();
    jstools::parseJson(sij, "PixelsX", PixelsX);
    jstools::parseJson(sij, "PixelsY", PixelsY);
    jstools::parseJson(sij, "DarkCountRate", DarkCountRate);

    QJsonObject pdej = json["PDE"].toObject();
    jstools::parseJson(pdej, "Effective", PDE_effective);
    jstools::parseJson(pdej, "WaveSensitive", WaveSensitive);
    if (pdej.contains("Spectral"))
    {
        QJsonArray ar = pdej["Data"].toArray();
//        readTwoQVectorsFromJArray(ar, PDE_lambda, PDE);
    }

    if (json.contains("AngularResponse"))
    {
        QJsonObject angj = json["AngularResponse"].toObject();
        jstools::parseJson(angj, "AngleSensitive", AngleSensitive);
        jstools::parseJson(angj, "InterfaceN", InterfaceN);
        QJsonArray ar =  angj["Data"].toArray();
//        readTwoQVectorsFromJArray(ar, AngularSensitivity_lambda, AngularSensitivity);
    }

    if (json.contains("AreaResponse"))
    {
        QJsonObject areaj = json["AreaResponse"].toObject();
        jstools::parseJson(areaj, "XYSensitive", XYSensitive);
        jstools::parseJson(areaj, "AreaStepX", StepX);
        jstools::parseJson(areaj, "AreaStepY", StepY);
        QJsonArray ar = areaj["Data"].toArray();
//        read2DQVectorFromJArray(ar, AreaSensitivity);
    }
}
