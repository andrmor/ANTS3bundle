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
    PixelSizeX = 3.0;
    PixelSizeY = 3.0;
    PixelSpacingX = 0;
    PixelSpacingY = 0;

    PDE_effective = 1.0;
    PDE_spectral.clear();
    PDEbinned.clear();

    AngularFactors.clear();
    AngularBinned.clear();

    AreaFactors.clear();
    StepX = 1.0;
    StepY = 1.0;

    DarkCountRate = 0;
    IntegrationTime = 1e-6;
    ElectronicNoiseSigma = 0;
    ElectronicGainFactor = 1.0;
}

void ASensorModel::writeToJson(QJsonObject & json) const
{
    json["Name"] = Name;

    {
        QJsonObject js;
            js["isSiPM"]        = SiPM;
            js["PixelsX"]       = PixelsX;
            js["PixelsY"]       = PixelsY;
            js["PixelSizeX"]    = PixelSizeX;
            js["PixelSizeY"]    = PixelSizeY;
            js["PixelSpacingX"] = PixelSpacingX;
            js["PixelSpacingY"] = PixelSpacingY;
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
                jstools::writeDVectorOfVectorsToArray(AreaFactors, ar);
            js["Data"] = ar;
        json["AreaResponse"] = js;
    }

    json["DarkCountRate"] = DarkCountRate;
    json["IntegrationTime"] = IntegrationTime;

    json["PreampSigma"] = ElectronicNoiseSigma;

    {
        QJsonObject js;
            js["ElectronicGainFactor"] = ElectronicGainFactor;
            QString str;
                switch (PhElToSignalModel)
                {
                default       : qWarning() << "Not implemented json write for selected PhElToSignal model"; // fall-through!
                case Constant : str = "Constant"; break;
                case Normal   : str = "Normal";   break;
                case Gamma    : str = "Gamma";    break;
                case Custom   : str = "Custom";   break;
                }
            js["Model"] = str;
            js["AverageSignalPerPhEl"] = AverageSignalPerPhEl;
            js["NormalSigma"] = NormalSigma;
            js["GammaShape"] = GammaShape;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(CustomSignalPerPhEl, ar);
            js["CustomSignalPerPhEl"] = ar;
        json["PhElToSignals"] = js;
    }
}

bool ASensorModel::readFromJson(const QJsonObject & json)
{
    clear();

    if (!json.contains("Name") || !json.contains("SiPM")) return false; // simple check of format

    jstools::parseJson(json, "Name",          Name);

    {
        QJsonObject js = json["SiPM"].toObject();
        jstools::parseJson(js, "isSiPM",        SiPM);
        jstools::parseJson(js, "PixelsX",       PixelsX);
        jstools::parseJson(js, "PixelsY",       PixelsY);
        jstools::parseJson(js, "PixelSizeX",    PixelSizeX);
        jstools::parseJson(js, "PixelSizeY",    PixelSizeY);
        jstools::parseJson(js, "PixelSpacingX", PixelSpacingX);
        jstools::parseJson(js, "PixelSpacingY", PixelSpacingY);
    }

    {
        QJsonObject js = json["PDE"].toObject();
        jstools::parseJson(js, "Effective", PDE_effective);
        QJsonArray ar;
        jstools::parseJson(js, "Spectral", ar);
        bool ok = jstools::readDPairVectorFromArray(ar, PDE_spectral);
        if (!ok) return false; // !!!***
    }

    {
        QJsonObject js = json["AngularResponse"].toObject();
        QJsonArray ar;
        jstools::parseJson(js, "Data", ar);
        bool ok = jstools::readDPairVectorFromArray(ar, AngularFactors);
        if (!ok) return false; // !!!***
    }

    {
        QJsonObject js = json["AreaResponse"].toObject();
        jstools::parseJson(js, "StepX", StepX);
        jstools::parseJson(js, "StepY", StepY);
        QJsonArray ar;
        jstools::parseJson(js, "Data", ar);
        bool ok = jstools::readDVectorOfVectorsFromArray(ar, AreaFactors);
        if (!ok) return false; // !!!***
    }

    jstools::parseJson(json, "DarkCountRate", DarkCountRate);
    jstools::parseJson(json, "IntegrationTime", IntegrationTime);

    jstools::parseJson(json, "PreampSigma", ElectronicNoiseSigma);

    {
        QJsonObject js = json["PhElToSignals"].toObject();
        jstools::parseJson(js, "ElectronicGainFactor", ElectronicGainFactor);
        QString str;
        jstools::parseJson(js, "Model", str);
        if      (str == "Constant") PhElToSignalModel = Constant;
        else if (str == "Normal")   PhElToSignalModel = Normal;
        else if (str == "Gamma")    PhElToSignalModel = Gamma;
        else if (str == "Custom")   PhElToSignalModel = Custom;
        else
        {
            qWarning() << "Unknown model of PhEl to signal convertion:" << str;  // !!!***
            PhElToSignalModel = Constant;
        }
        jstools::parseJson(js, "AverageSignalPerPhEl", AverageSignalPerPhEl);
        jstools::parseJson(js, "NormalSigma", NormalSigma);
        jstools::parseJson(js, "GammaShape", GammaShape);
        QJsonArray ar;
        jstools::parseJson(js, "CustomSignalPerPhEl", ar);
        bool ok = jstools::readDPairVectorFromArray(ar, CustomSignalPerPhEl);
        if (!ok) return false; // !!!***
    }

    return true;
}

QString ASensorModel::checkPDE_spectral() const
{
    if (PDE_spectral.empty()) return "";

    if (PDE_spectral.size() < 2) return "Data should contain at least two wavelengths";

    double prevWave = -1e10;
    for (const auto & p : PDE_spectral)
    {
        const double & wave = p.first;
        const double & pde  = p.second;
        if (wave <= 0.0) return "Wavelengths should be positive numbers";
        if (wave <= prevWave) return "Wavelengths should be provided as increasing numbers";
        prevWave = wave;
        if (pde < 0) return "PDE cannot be negative";
    }
    return "";
}

QString ASensorModel::checkAngularFactors() const
{
    if (AngularFactors.empty()) return "";

    if (AngularFactors.size() < 2) return "Data should contain at least two angles: 0 and 90 degrees";
    if (AngularFactors.front().first != 0.0) return "Data should start from 0 degrees";
    if (AngularFactors.back().first != 90.0) return "Data should end with 90 degrees";

    double prevAngle = -1e10;
    for (const auto & p : AngularFactors)
    {
        const double & angle  = p.first;
        const double & factor = p.second;
        if (angle <= prevAngle) return "Angles should be provided as increasing numbers";
        prevAngle = angle;
        if (factor < 0) return "AngularFactor cannot be negative";
    }
    return "";
}

QString ASensorModel::checkAreaFactors() const
{
    if (AreaFactors.empty()) return "";

    if (AreaFactors.size() < 2) return "Data should be at least a 2x2 matrix";
    const size_t numX = AreaFactors.front().size();
    if (numX < 2) return "Data should be at least a 2x2 matrix";

    for (const auto & vec : AreaFactors)
    {
        if (vec.size() != numX) return "Wrong size of a row of the matrix";
        for (const double & val : vec)
            if (val < 0) return "Area factors cannot be negative";
    }

    if (StepX <= 0.0) return "StepX should be a positive number";
    if (StepY <= 0.0) return "StepY should be a positive number";

    return "";
}

QString ASensorModel::checkPhElToSignals() const
{
    if (ElectronicGainFactor <= 0) return "Electronic gain factor should be positive";

    if (PhElToSignalModel == Custom)
    {
        const QString errstr = "CustomSignalPerPhEl data should start from 0 and contain at least two data points";
        if (CustomSignalPerPhEl.size() < 2) return errstr;
        if (CustomSignalPerPhEl.front().first != 0) return errstr;
        auto prevVal = CustomSignalPerPhEl.front();
        for (size_t i = 1; i < CustomSignalPerPhEl.size(); i++)
        {
            const auto thisVal = CustomSignalPerPhEl[i];
            if (prevVal.first >= thisVal.first) return "CustomSignalPerPhEl data should be sorted (increasing and not-repeating ph.el. values)";
            if (thisVal.second < 0) return "CustomSignalPerPhEl data should contain non-negative convertion factors";
            prevVal = thisVal;
        }
    }

    return "";
}

bool ASensorModel::getPixelHit(double x, double y, size_t & binX, size_t & binY) const
{
    if (SiPM)
    {
        //qDebug() << "Checking hit for local x,y:" << x << y << ", showing nothing if there were no pixel hit";
        if (x < - _HalfSensitiveSizeX || x > _HalfSensitiveSizeX) return false;
        if (y < - _HalfSensitiveSizeY || y > _HalfSensitiveSizeY) return false;

        x += _HalfSensitiveSizeX;  // [0, FullSizeX]
        y += _HalfSensitiveSizeY;  // [0, FullSizeY]

        if (std::fmod(x, _PixelPitchX) > PixelSizeX) return false;
        if (std::fmod(y, _PixelPitchY) > PixelSizeY) return false;

        binX = x / _PixelPitchX;
        binY = y / _PixelPitchY;
        //const int index = iy * PixelsX + ix;
        //qDebug() << "-->Hit detected; xBin,yBin,index:"<< xBin << yBin << index;
        return true;
    }
    else return false;
}

double ASensorModel::getPDE(int iWave) const
{
    if (iWave == -1 || PDEbinned.empty()) return PDE_effective;
    return PDEbinned[iWave];
}

double ASensorModel::getAngularFactor(double angle) const
{
    if (AngularBinned.empty()) return 1.0;

    int bin = fabs(angle);
    if (bin > 90) return 0;
    return AngularBinned[bin];
}

double ASensorModel::getAreaFactor(double x, double y) const
{
    if (AreaFactors.empty()) return 1.0;

    const size_t numX = AreaFactors.front().size();
    const size_t numY = AreaFactors.size();

    const int xBin = std::floor( (x + 0.5*numX*StepX) / StepX );
    //qDebug() << x << xBin << StepX;
    if (xBin < 0 || xBin >= numX) return 0;

    int yBin = std::floor( (y + 0.5*numY*StepY) / StepY );
    if (yBin < 0 || yBin >= numY) return 0;
    yBin = numY-1 - yBin; // inverting Y as the matrix holds "top" Y row in the index 0
    qDebug() << "X:" << x << "xBin:" << xBin << "Y:" << y << "yBin:" << yBin << AreaFactors[yBin][xBin];
    return AreaFactors[yBin][xBin];
}

#include "aphotonsimhub.h"
void ASensorModel::updateRuntimeProperties()
{
    if (SiPM)
    {
        _PixelPitchX = PixelSizeX + PixelSpacingX;
        _PixelPitchY = PixelSizeY + PixelSpacingY;

        _HalfSensitiveSizeX = 0.5 * (PixelSizeX * PixelsX + PixelSpacingX * (PixelsX - 1));
        _HalfSensitiveSizeY = 0.5 * (PixelSizeY * PixelsY + PixelSpacingY * (PixelsY - 1));
    }

    PDEbinned.clear();
    AngularBinned.clear();

    const APhotonSimSettings SimSet = APhotonSimHub::getConstInstance().Settings;

    if (!PDE_spectral.empty()) SimSet.WaveSet.toStandardBins(PDE_spectral, PDEbinned);

    if (!AngularFactors.empty())
    {
        AngularBinned.reserve(91);
        for (int i = 0; i < 91; i++)
        {
            const double sens = AWaveResSettings::getInterpolatedValue(i, AngularFactors);
            AngularBinned.push_back(sens);
        }
    }
}

#include "arandomhub.h"
double ASensorModel::generateSignalForOnePhotoelectron()
{
    double val = 0;

    ARandomHub & RandGub = ARandomHub::getInstance();
    switch (PhElToSignalModel)
    {
    case 0:
        val = AverageSignalPerPhEl;
        break;
    case 1:
        val = RandGub.gauss(AverageSignalPerPhEl, NormalSigma);
        break;
    case 2:
        val = RandGub.gamma(GammaShape, AverageSignalPerPhEl / GammaShape);
        break;
    case 3:
        //if ( pm.SPePHShist ) val = pm.SPePHShist->GetRandom();
        break;
    default:
        qWarning() << "Error: unrecognized type in signal per photoelectron generation";
    }

    return val * ElectronicGainFactor;
}

#include "arandomhub.h"
double ASensorModel::convertHitsToSignal(double phel) const
{
    ARandomHub & RandomHub = ARandomHub::getInstance();

    double signal;
    switch (PhElToSignalModel)
    {
    case ASensorModel::Constant :
        signal = phel * AverageSignalPerPhEl;
        break;
    case ASensorModel::Normal :
        {
            const double mean  = AverageSignalPerPhEl * phel;
            const double sigma = NormalSigma          * TMath::Sqrt( phel );
            signal = RandomHub.gauss(mean, sigma);
            break;
        }
    case ASensorModel::Gamma :
        {
            double k = GammaShape;
            const double theta = AverageSignalPerPhEl / k;
            k *= phel; //for sum distribution
            signal = RandomHub.gamma(k, theta);
            break;
        }
    case ASensorModel::Custom :
        {
            /*
            pmSignals[ipm] = 0;
            if ( pm.SPePHShist )
            {
                for (int j = 0; j < pmHits.at(ipm); j++)
                    pmSignals[ipm] += pm.SPePHShist->GetRandom();
            }
            */
            break;
        }
    }

    signal *= ElectronicGainFactor;

    return signal;
}
