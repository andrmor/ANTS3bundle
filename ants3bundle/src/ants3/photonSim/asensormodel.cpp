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
    //InterfaceN = 1.0;
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
            //js["InterfaceN"] = InterfaceN;
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
        //jstools::parseJson(angj, "InterfaceN", InterfaceN);
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

    if (!AngularFactors.empty())
    {
        /*
        //transforming degrees to cos(Refracted)
        QVector<double> x, y;
        double n1 = 1.0; // InterfaceN;
        double n2 = (*MaterialCollection)[PMtypes[typ]->MaterialIndex]->n;
        //      qDebug()<<"n1 n2 "<<n1<<n2;

        for (int i=PMtypes[typ]->AngularSensitivity_lambda.size()-1; i>=0; i--) //if i++ - data span from 1 down to 0
        {
            double Angle = PMtypes[typ]->AngularSensitivity_lambda[i] * TMath::Pi()/180.0;
            //calculating cos of the transmitted angle
            double sinI = sin(Angle);
            double cosI = cos(Angle);

            double sinT = sinI * n1 / n2;
            double cosT = sqrt(1 - sinT*sinT);
            x.append(cosT);

            //         qDebug()<<"Angle: "<<Angle<< " sinI="<<sinI<<" cosI="<<cosI;
            //         qDebug()<<"sinT="<<sinT<<"cosT="<<cosT;

            //correcting for the reflection loss
            double Rs = (n1*cosI-n2*cosT)*(n1*cosI-n2*cosT) / ( (n1*cosI+n2*cosT) * (n1*cosI+n2*cosT) );
            double Rp = (n1*cosT-n2*cosI)*(n1*cosT-n2*cosI) / ( (n1*cosT+n2*cosI) * (n1*cosT+n2*cosI) );
            double R = 0.5*(Rs+Rp);
            //         qDebug()<<"Rs Rp"<<Rs<<Rp;
            //         qDebug()<<"Reflection "<<R<<"ReflectionforNormal="<<(n1-n2)*(n1-n2)/(n1+n2)/(n1+n2);

            const double correction = R < 0.99999 ? 1.0/(1.0-R) : 1.0; //T = (1-R)
            //meaning: have to take into account that during measurements part of the light was reflected

            y.append(PMtypes[typ]->AngularSensitivity[i]*correction);
            //         qDebug()<<cosT<<correction<<PMtypeProperties[typ].AngularSensitivity[i]*correction;
        }
        x.replace(0, x[1]); //replace with the last available value
        y.replace(0, y[1]);
        //reusing the function:
        ConvertToStandardWavelengthes(&x, &y, 0, 1.0/(CosBins-1), CosBins, &PMtypes[typ]->AngularSensitivityCosRefracted);
        */
    }
}
