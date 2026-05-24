#include "asensormodel.h"
#include "aphotonsimhub.h"
#include "arandomhub.h"
#include "ajsontools.h"

#include <tuple>

#include "TH1D.h"
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

    PDE_model = 0;
    PDE_effective = 1.0;
    PDE_spectral.clear();
    PDEbinned.clear();

    AngularFactors.clear();
    AngularBinned.clear();
    Angular_Wavelength = 600.0;

    AreaFactors.clear();
    StepX = 1.0;
    StepY = 1.0;

    DarkCountRate = 0;
    IntegrationTime = 1e-6;
    ElectronicNoiseSigma = 0;
    //ElectronicGainFactor = 1.0;

    delete _PHS; _PHS = nullptr;
    _AverageDarkCounts = 0;
    _PixelDarkFiringProbability = 0;
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
            js["PDE_model"] = PDE_model;
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
            js["Angular_Wavelength"] = Angular_Wavelength;
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
            //js["ElectronicGainFactor"] = ElectronicGainFactor;
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
                jstools::writeDPairVectorToArray(SinglePhElPHS, ar);
            js["SinglePhElPHS"] = ar;
        json["PhElToSignals"] = js;
    }
}

QString ASensorModel::readFromJson(const QJsonObject & json)
{
    clear();

    if (!json.contains("Name") || !json.contains("SiPM")) return "Bad json for a sensor model"; // simple check of format

    jstools::parseJson(json, "Name", Name);

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

    QString errStub = "Sensor model " + Name + " : ";

    {
        QJsonObject js = json["PDE"].toObject();
            jstools::parseJson(js, "PDE_model", PDE_model);
            jstools::parseJson(js, "Effective", PDE_effective);
            QJsonArray ar;
            jstools::parseJson(js, "Spectral", ar);
            bool ok = jstools::readDPairVectorFromArray(ar, PDE_spectral);
            if (!ok) return errStub + "Failed to read the array with PDE data for a sensor model";
            QString err = checkPDE_spectral();
            if (!err.isEmpty()) return errStub + err;
    }

    {
        QJsonObject js = json["AngularResponse"].toObject();
            QJsonArray ar;
            jstools::parseJson(js, "Data", ar);
            bool ok = jstools::readDPairVectorFromArray(ar, AngularFactors);
            if (!ok) return errStub + "Failed to the array with angular response";
            QString err = checkAngularFactors();
            if (!err.isEmpty()) return errStub + err;

            jstools::parseJson(js, "Angular_Wavelength", Angular_Wavelength);
    }

    {
        QJsonObject js = json["AreaResponse"].toObject();
            jstools::parseJson(js, "StepX", StepX);
            jstools::parseJson(js, "StepY", StepY);
            QJsonArray ar;
            jstools::parseJson(js, "Data", ar);
            bool ok = jstools::readDVectorOfVectorsFromArray(ar, AreaFactors);
            if (!ok) return errStub + "Failed to read the file with area response factors";
            QString err = checkAreaFactors();
            if (!err.isEmpty()) return errStub + err;
    }

    jstools::parseJson(json, "DarkCountRate", DarkCountRate);
    jstools::parseJson(json, "IntegrationTime", IntegrationTime);

    jstools::parseJson(json, "PreampSigma", ElectronicNoiseSigma);

    {
        QJsonObject js = json["PhElToSignals"].toObject();
            QString str;
            jstools::parseJson(js, "Model", str);
            if      (str == "Constant") PhElToSignalModel = Constant;
            else if (str == "Normal")   PhElToSignalModel = Normal;
            else if (str == "Gamma")    PhElToSignalModel = Gamma;
            else if (str == "Custom")   PhElToSignalModel = Custom;
            else return errStub + "Unknown model of PhEl to signal convertion: " + str;
            jstools::parseJson(js, "AverageSignalPerPhEl", AverageSignalPerPhEl);
            jstools::parseJson(js, "NormalSigma", NormalSigma);
            jstools::parseJson(js, "GammaShape", GammaShape);
            QJsonArray ar;
            jstools::parseJson(js, "SinglePhElPHS", ar);
            bool ok = jstools::readDPairVectorFromArray(ar, SinglePhElPHS);
            if (!ok) return errStub + "Failed to read the array with SinglePhElPHS";
            QString err = checkPhElToSignals();
            if (!err.isEmpty()) return errStub + err;
    }

    return "";
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
    //if (ElectronicGainFactor <= 0) return "Electronic gain factor should be positive";

    if (PhElToSignalModel == Custom)
    {
        if (SinglePhElPHS.size() < 2) return "Custom pulse height distribution for single ph.e- should contain at leats two data points";
        auto prevVal = SinglePhElPHS.front();
        for (size_t i = 1; i < SinglePhElPHS.size(); i++)
        {
            const auto thisVal = SinglePhElPHS[i];
            if (prevVal.first >= thisVal.first) return "Custom pulse height distribution for single ph.e- data should be sorted (increasing and not-repeating signal values)";
            //if (thisVal.second < 0) return "Custom pulse height distribution for single ph.e- should contain non-negative signal values";
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
        if (x <= - _HalfSensitiveSizeX || x >= _HalfSensitiveSizeX) return false;
        if (y <= - _HalfSensitiveSizeY || y >= _HalfSensitiveSizeY) return false;

        x += _HalfSensitiveSizeX;  // [0, FullSizeX]
        y += _HalfSensitiveSizeY;  // [0, FullSizeY]
        //qDebug() << "  shifted x,y:" << x << y;

        if (std::fmod(x, _PixelPitchX) >= PixelSizeX) return false;
        if (std::fmod(y, _PixelPitchY) >= PixelSizeY) return false;

        binX = x / _PixelPitchX;
        binY = y / _PixelPitchY;
        //qDebug() << "  -->Hit detected; xBin,yBin,index:"<< binX << binY;
        return true;
    }
    else return false;
}

double ASensorModel::getPDE(int iWave, int iSensorMat) const
{
    if (PDE_model == 0)
    {
        if (iWave == -1 || PDEbinned.empty()) return PDE_effective;
        return PDEbinned[iWave];
    }

    int iRecord = -1;
    for (size_t i = 0; i < _InterfaceAwarePDE.size(); i++)
    {
        if (_InterfaceAwarePDE[i].first == iSensorMat)
        {
            iRecord = i;
            break;
        }
    }

    if (iRecord != -1)
    {
        const AInterfaceAwareRuntimeProps & props = _InterfaceAwarePDE[iRecord].second;
        if (iWave == -1 || props.PdeBinnedFactor.empty()) return PDE_effective * props.EffectivePdeFactor;
        return PDEbinned[iWave] * props.PdeBinnedFactor[iWave];
    }
    else
    {
        // !!!*** report error
        qCritical() << "_InterfaceAware PDE data for sensor material " <<  iSensorMat << "not found!";
        exit(666);
    }
}

double ASensorModel::getAngularFactor(double angle, int iSensorMat) const
{
    if (PDE_model == 0)
    {
        if (AngularBinned.empty()) return 1.0;

        int bin = fabs(angle) * (_NumAngularBins - 1) / 90.0;
        if (bin >= _NumAngularBins) return 0;
        return AngularBinned[bin];
    }

    int iRecord = -1;
    for (size_t i = 0; i < _InterfaceAwarePDE.size(); i++)
    {
        if (_InterfaceAwarePDE[i].first == iSensorMat)
        {
            iRecord = i;
            break;
        }
    }

    if (iRecord != -1)
    {
        const AInterfaceAwareRuntimeProps & props = _InterfaceAwarePDE[iRecord].second;
        if (props.AngularBinned.empty()) return 1.0;

        int bin = fabs(angle) * (_NumAngularBins - 1) / 90.0;
        if (bin >= _NumAngularBins) return 0;
        return props.AngularBinned[bin];
    }
    else
    {
        // !!!*** report error
        qCritical() << "_InterfaceAware PDE data for sensor material " <<  iSensorMat << "not found!";
        exit(666);
    }
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

double ASensorModel::getMaxQE(bool bWaveRes) const
{
    if (PDE_model == 0)
    {
        double maxQE = (bWaveRes ? _MaxPDE_spectral : PDE_effective);
        maxQE *= _MaxAngularFactor;
        maxQE *= _MaxAreaFactor;
        return maxQE;
    }
    else
    {
        double maxQE = (bWaveRes ? _MaxPDE_spectral : _MaxPDE_effective);
        maxQE *= _MaxAngularFactor;
        maxQE *= _MaxAreaFactor;
        return maxQE;
    }
}

QString ASensorModel::updateRuntimeProperties(const std::vector<int> & seenSensorMats)
{
    if (SiPM)
    {
        _PixelPitchX = PixelSizeX + PixelSpacingX;
        _PixelPitchY = PixelSizeY + PixelSpacingY;

        _HalfSensitiveSizeX = 0.5 * (PixelSizeX * PixelsX + PixelSpacingX * (PixelsX - 1));
        _HalfSensitiveSizeY = 0.5 * (PixelSizeY * PixelsY + PixelSpacingY * (PixelsY - 1));
    }

    _AverageDarkCounts = DarkCountRate * IntegrationTime;
    if (SiPM) _PixelDarkFiringProbability = _AverageDarkCounts / PixelsX / PixelsY;

    PDEbinned.clear();
    AngularBinned.clear();

    const APhotonSimSettings & SimSet = APhotonSimHub::getConstInstance().Settings;

    _MaxPDE_spectral = 1.0;
    if (!PDE_spectral.empty())
    {
        QString err = checkPDE_spectral();
        if (!err.isEmpty()) return err;

        SimSet.WaveSet.toStandardBins(PDE_spectral, PDEbinned, AWaveResSettings::ExpandWithZero);
        _MaxPDE_spectral = *std::max_element(PDEbinned.begin(), PDEbinned.end());
    }

    if (AngularFactors.empty()) _MaxAngularFactor = 1.0;
    else
    {
        QString err = checkAngularFactors();
        if (!err.isEmpty()) return err;

        _MaxAngularFactor = 0;
        AngularBinned.resize(_NumAngularBins);
        for (int i = 0; i < _NumAngularBins; i++)
        {
            const double angle = 90.0 * i / (_NumAngularBins - 1);
            const double sens = AWaveResSettings::getInterpolatedValue(angle, AngularFactors);
            AngularBinned[i] = sens;
            if (sens > _MaxAngularFactor) _MaxAngularFactor = sens;
        }

        if (SimSet.WaveSet.Enabled)
            if (Angular_Wavelength < SimSet.WaveSet.From || Angular_Wavelength > SimSet.WaveSet.To)
                return "Wavelength at which the angular distribution was measured: cannot be outside of the defined wavelength range";
    }

    if (AreaFactors.empty()) _MaxAreaFactor = 1.0;
    else
    {
        QString err = checkAreaFactors();
        if (!err.isEmpty()) return err;

        _MaxAreaFactor = 0;
        for (const std::vector<double> & vec : AreaFactors)
            for (double val : vec)
                if (val > _MaxAreaFactor) _MaxAreaFactor = val;
    }

    delete _PHS; _PHS = nullptr;
    if (PhElToSignalModel == Custom)
    {
        QString err = checkPhElToSignals();
        if (!err.isEmpty()) return err;

        const int bins = 1000;
        const double start = SinglePhElPHS.front().first;
        const double stop  = SinglePhElPHS.back().first;
        const double delta = (stop - start) / bins;
        _PHS = new TH1D("", "", 1000, start, stop);
        for (int j = 1; j < bins+1; j++)
        {
            const double pos = start + delta * (j - 0.5);
            const double binVal = AWaveResSettings::getInterpolatedValue(pos, SinglePhElPHS);
            _PHS->SetBinContent(j, binVal);
        }

        _PHS->GetIntegral();
    }

    _InterfaceAwarePDE.clear();
    if (PDE_model == 1) updateInterfaceAwareRuntimeProps(seenSensorMats);

    return "";
}

#include "amaterialhub.h"
void ASensorModel::updateInterfaceAwareRuntimeProps(const std::vector<int> & seenSensorMats)
{
    _InterfaceAwarePDE.resize(seenSensorMats.size());
    const APhotonSimSettings & SimSet = APhotonSimHub::getConstInstance().Settings;

    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();

    _MaxPDE_effective = 0;
    _MaxPDE_spectral  = 0; // set to 1.0 if remains zero at the end (PDE spectral data are not provided)
    _MaxAngularFactor = 0;

    for (size_t index = 0; index < seenSensorMats.size(); index++)
    {
        int iMat = seenSensorMats[index];
        _InterfaceAwarePDE[index].first = iMat;

        AInterfaceAwareRuntimeProps & props = _InterfaceAwarePDE[index].second;

        // normal incidence, -1 waveindex:  R = ((n1 - n2) / (n1 + n2))^2
        // n1 = 1 (air)
        double n2 = MatHub[iMat]->RefIndex;
        double R = (1.0 - n2) / (1.0 + n2); R *= R;
        props.EffectivePdeFactor = 1.0 / (1.0 - R);
        double factorAtNormal = props.EffectivePdeFactor;
        qDebug() << "Factor for EffectivePDE = " << props.EffectivePdeFactor;

        double effVal = props.EffectivePdeFactor * PDE_effective;
        if (effVal > _MaxPDE_effective) _MaxPDE_effective = effVal;

        bool havePdeSpectrum = !PDEbinned.empty();
        bool haveRefIndexSpectrum = !MatHub[iMat]->_RefIndex_WaveBinned.empty();
        int iWave = SimSet.WaveSet.toIndex(Angular_Wavelength);
        if (havePdeSpectrum)
        {
            props.PdeBinnedFactor.resize(PDEbinned.size());
            for (size_t i = 0; i < PDEbinned.size(); i++)
            {
                if (haveRefIndexSpectrum) n2 = MatHub[iMat]->_RefIndex_WaveBinned[i]; // else effective value
                double R = (1.0 - n2) / (1.0 + n2); R *= R;
                props.PdeBinnedFactor[i] = 1.0 / (1.0 - R);

                double val = props.PdeBinnedFactor[i] * PDEbinned[i];
                if (val > _MaxPDE_spectral) _MaxPDE_spectral = val;
            }
            if (iWave != -1) factorAtNormal = props.PdeBinnedFactor[iWave];
        }
        qDebug() << "Factor for PDEbinned: " << props.PdeBinnedFactor;
        qDebug() << "Normal incidence factor = " << factorAtNormal;

        bool haveAngular = !AngularBinned.empty();
        if (haveAngular)
        {
            // have to redo angular completely
            double nSensor; // need use the value at the wavelength of th emeasurement
            if (SimSet.WaveSet.Enabled) nSensor = MatHub[iMat]->getRefractiveIndex(iWave);
            else                        nSensor = MatHub[iMat]->RefIndex;
            std::vector<std::pair<double,double>> dataAngular(_NumAngularBins);
            for (int i = 0; i < _NumAngularBins; i++)
            {
                double insAngle = 90.0 * i / (_NumAngularBins - 1);
                // Snell: n1*sinI = n2*sinR
                // n1 = 1.0; n2 = nSensor
                double sinI = sin(insAngle * 3.1415926535/180.0);
                double sinR = 1.0 * sinI / nSensor;
                dataAngular[i].first = asin(sinR) * 180.0/3.1415926535; // refracted angle
                qDebug() << insAngle << "-->" << dataAngular[i].first;

                // Fresnel:
                // Rs = ((n1*cosI - n2*cosR)/(n1*cosI + n2*cosR))^2
                // Rp = ((n1*cosR - n2*cosI)/(n1*cosR + n2*cosI))^2
                // R = 0.5*(Rs+Rp)
                double cosI = sqrt(1.0 - sinI * sinI);
                double cosR = sqrt(1.0 - sinR * sinR);
                double Rs = (1.0*cosI - nSensor*cosR) / (1.0*cosI + nSensor*cosR); Rs *= Rs;
                double Rp = (1.0*cosR - nSensor*cosI) / (1.0*cosR + nSensor*cosI); Rp *= Rp;
                double R = 0.5 * (Rs + Rp);
                double factor;
                if (R < 1.0) factor = 1.0 / (1.0 - R) / factorAtNormal;
                else         factor = 0;
                qDebug() << insAngle << cosI << cosR << "R" << R << "factor:" << factor;
                dataAngular[i].second = AngularBinned[i] * factor;
            }

            //qDebug() << "Angular for" << MatHub.getMaterialName(iMat) << dataAngular;

            props.AngularBinned.resize(_NumAngularBins);
            double lastNonZero = 0;
            for (int i = 0; i < _NumAngularBins; i++)
            {
                const double angle = 90.0 * i / (_NumAngularBins - 1);
                double sens = AWaveResSettings::getInterpolatedValue(angle, dataAngular);
                if (sens > 0) lastNonZero = sens;
                else          sens = lastNonZero;
                props.AngularBinned[i] = sens;

                if (sens > _MaxAngularFactor) _MaxAngularFactor = sens; // over all sensor material options
            }
            //qDebug() << "AngularBinned: " << props.AngularBinned;
        }
    }

    if (_MaxPDE_effective == 0) _MaxPDE_effective = 1.0; // paranoid
    qDebug() << "_MaxPDE_effective:" << _MaxPDE_effective;
    if (_MaxPDE_spectral == 0) _MaxPDE_spectral = 1.0;
    qDebug() << "_MaxPDE_spectral:" << _MaxPDE_spectral;
    qDebug() << "_MaxAngularFactor:" << _MaxAngularFactor;
}

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
            signal = 0;
            if (_PHS)
            {
                const int num = (int)phel;
                for (int j = 0; j < num; j++)
                    signal += _PHS->GetRandom();
            }
            break;
        }
    }

    if (ElectronicNoiseSigma != 0) signal += RandomHub.gauss(0, ElectronicNoiseSigma);

    //signal *= ElectronicGainFactor;  // no more a part of the sensor model, see SensorHub (standalone now!)

    return signal;
}

/*
double ASensorModel::simulateDigitalization(double signal) const
{
    // ADC simulation
    if (PMs->isDoADC())
    {
        if (pmSignals[ipm] < 0) pmSignals[ipm] = 0;
        else
        {
            if (pmSignals[ipm] > pm.ADCmax) pmSignals[ipm] = pm.ADClevels;
            else pmSignals[ipm] = static_cast<int>( pmSignals.at(ipm) / PMs->at(ipm).ADCstep );
        }
    }
}
*/
