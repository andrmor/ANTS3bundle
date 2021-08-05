#include "asensormodel.h"
#include "ajsontools.h"

#include "TMath.h"

void ASensorModel::clear()
{
    PDE.clear();
    PDEbinned.clear();
    EffectivePDE = 1.0;

    EnableAngularSensitivity = false;
    AngularFactors.clear();
    AngularSensitivityCosRefracted.clear();
    AngularN1 = 1.0;

    EnablePositionSensitivity = false;
    PositionFactors.clear();
    StepX = 1.0;
    StepY = 1.0;
}

void ASensorModel::writeToJson(QJsonObject &json) const
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
        pdej["PDE"] = EffectivePDE;
        QJsonArray ar;
//            writeTwoQVectorsToJArray(PDE_lambda, PDE, ar);
        pdej["PDEwave"] = ar;
    json["PDEproperties"] = pdej;

    QJsonObject angj;
        angj["RefrIndexMeasure"] = AngularN1;
        QJsonArray ar1;
//        writeTwoQVectorsToJArray(AngularSensitivity_lambda, AngularSensitivity, ar1);
        angj["ResponseVsAngle"] = ar1;
    json["AngularResponse"] = angj;

    QJsonObject areaj;
        areaj["EnablePositionSensitivity"] = EnablePositionSensitivity;
        areaj["StepX"] = StepX;
        areaj["StepY"] = StepY;
        QJsonArray arar;
//        write2DQVectorToJArray(AreaSensitivity, arar);
        areaj["ResponseVsXY"] = arar;
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

    QJsonObject pdej = json["PDEproperties"].toObject();
    jstools::parseJson(pdej, "PDE", EffectivePDE);
    if (pdej.contains("PDEwave"))
    {
        QJsonArray ar = pdej["PDEwave"].toArray();
//        readTwoQVectorsFromJArray(ar, PDE_lambda, PDE);
    }

    if (json.contains("AngularResponse"))
    {
        QJsonObject angj = json["AngularResponse"].toObject();
        jstools::parseJson(angj, "RefrIndexMeasure", AngularN1);
        QJsonArray ar =  angj["ResponseVsAngle"].toArray();
//        readTwoQVectorsFromJArray(ar, AngularSensitivity_lambda, AngularSensitivity);
    }

    if (json.contains("AreaResponse"))
    {
        QJsonObject areaj = json["AreaResponse"].toObject();
        jstools::parseJson(areaj, "AreaStepX", StepX);
        jstools::parseJson(areaj, "AreaStepY", StepY);
        QJsonArray ar = areaj["ResponseVsXY"].toArray();
//        read2DQVectorFromJArray(ar, AreaSensitivity);
    }
}
