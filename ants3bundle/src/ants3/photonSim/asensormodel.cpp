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
    AngularBinned.clear();

    StepX = 1.0;
    StepY = 1.0;
    AreaFactors.clear();

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
        QJsonArray aar;
        jstools::parseJson(angj, "Data", aar);
        ok = jstools::readDPairVectorFromArray(aar, AngularFactors);
        if (!ok) return false; // !!!***

    QJsonObject areaj = json["AreaResponse"].toObject();
        jstools::parseJson(areaj, "StepX", StepX);
        jstools::parseJson(areaj, "StepY", StepY);
        QJsonArray arar;
        jstools::parseJson(areaj, "Data", arar);
        ok = jstools::readDVectorOfVectorsFromArray(arar, AreaFactors);
        if (!ok) return false; // !!!***

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
