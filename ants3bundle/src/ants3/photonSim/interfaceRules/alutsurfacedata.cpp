#include "alutsurfacedata.h"
#include "ajsontools.h"

#include <QJsonObject>
#include <QJsonArray>

#include <algorithm>
#include <cmath>

void ALutSurfaceData::clear()
{
    Launched.clear();
    ReflectedCounts.clear();
    TransmittedCounts.clear();
    AbsorbedCounts.clear();
    ReflectedHist.clear();
    TransmittedHist.clear();

    SourceHeightmap.clear();
    GenerationDate.clear();
    Comment.clear();
    PixelSizeX = 0; PixelSizeY = 0;
    GridSizeX  = 0; GridSizeY  = 0;
    PhotonsPerThetaBin = 0;
    MeanBounces  = 0;
    AnomalyCount = 0;

    R.clear();
    T.clear();
    ReflCdf.clear();
    TransCdf.clear();
    RuntimeReady = false;
}

static QJsonArray intVectorToArray(const std::vector<int> & vec)
{
    QJsonArray ar;
    for (int v : vec) ar.append(v);
    return ar;
}

static bool arrayToIntVector(const QJsonArray & ar, std::vector<int> & vec)
{
    vec.clear();
    vec.reserve(ar.size());
    for (int i = 0; i < ar.size(); i++)
    {
        if (!ar[i].isDouble()) return false;
        vec.push_back(ar[i].toInt());
    }
    return true;
}

static QJsonArray intVectorOfVectorsToArray(const std::vector<std::vector<int>> & vec)
{
    QJsonArray ar;
    for (const std::vector<int> & v : vec) ar.append(intVectorToArray(v));
    return ar;
}

static bool arrayToIntVectorOfVectors(const QJsonArray & ar, std::vector<std::vector<int>> & vec)
{
    vec.clear();
    vec.resize(ar.size());
    for (int i = 0; i < ar.size(); i++)
    {
        if (!ar[i].isArray()) return false;
        if (!arrayToIntVector(ar[i].toArray(), vec[i])) return false;
    }
    return true;
}

void ALutSurfaceData::writeToJson(QJsonObject & json) const
{
    json["Type"] = "ANTS3_SurfaceLUT";
    json["FormatVersion"] = CurrentFormatVersion;

    QJsonObject jsMeta;
        jsMeta["n1"] = n1;
        jsMeta["n2"] = n2;
        jsMeta["Wavelength"] = Wavelength;
        jsMeta["SourceHeightmap"] = SourceHeightmap;
        jsMeta["PixelSize"] = QJsonArray{PixelSizeX, PixelSizeY};
        jsMeta["GridSize"]  = QJsonArray{GridSizeX, GridSizeY};
        jsMeta["PhotonsPerThetaBin"] = PhotonsPerThetaBin;
        jsMeta["Seed"] = Seed;
        jsMeta["MaxBounces"] = MaxBounces;
        jsMeta["MeanBounces"] = MeanBounces;
        jsMeta["AnomalyCount"] = AnomalyCount;
        jsMeta["GenerationDate"] = GenerationDate;
        jsMeta["Comment"] = Comment;
    json["Meta"] = jsMeta;

    QJsonObject jsBin;
        jsBin["ThetaIncBins"] = ThetaIncBins;
        jsBin["ThetaOutBins"] = ThetaOutBins;
        jsBin["PhiOutBins"]   = PhiOutBins;
    json["Binning"] = jsBin;

    json["Launched"]          = intVectorToArray(Launched);
    json["ReflectedCounts"]   = intVectorToArray(ReflectedCounts);
    json["TransmittedCounts"] = intVectorToArray(TransmittedCounts);
    json["AbsorbedCounts"]    = intVectorToArray(AbsorbedCounts);
    json["ReflectedHist"]     = intVectorOfVectorsToArray(ReflectedHist);
    json["TransmittedHist"]   = intVectorOfVectorsToArray(TransmittedHist);
}

QString ALutSurfaceData::readFromJson(const QJsonObject & json)
{
    clear();

    QString type;
    jstools::parseJson(json, "Type", type);
    if (type != "ANTS3_SurfaceLUT") return "Not an ANTS3 surface LUT (\"Type\" is not \"ANTS3_SurfaceLUT\")";

    int version = 0;
    jstools::parseJson(json, "FormatVersion", version);
    if (version > CurrentFormatVersion)
        return QString("Surface LUT format version %1 is newer than the supported version %2").arg(version).arg(CurrentFormatVersion);

    QJsonObject jsMeta;
    if (jstools::parseJson(json, "Meta", jsMeta))
    {
        jstools::parseJson(jsMeta, "n1", n1);
        jstools::parseJson(jsMeta, "n2", n2);
        jstools::parseJson(jsMeta, "Wavelength", Wavelength);
        jstools::parseJson(jsMeta, "SourceHeightmap", SourceHeightmap);
        QJsonArray ar;
        if (jstools::parseJson(jsMeta, "PixelSize", ar) && ar.size() == 2)
        {
            PixelSizeX = ar[0].toDouble();
            PixelSizeY = ar[1].toDouble();
        }
        if (jstools::parseJson(jsMeta, "GridSize", ar) && ar.size() == 2)
        {
            GridSizeX = ar[0].toInt();
            GridSizeY = ar[1].toInt();
        }
        jstools::parseJson(jsMeta, "PhotonsPerThetaBin", PhotonsPerThetaBin);
        jstools::parseJson(jsMeta, "Seed", Seed);
        jstools::parseJson(jsMeta, "MaxBounces", MaxBounces);
        jstools::parseJson(jsMeta, "MeanBounces", MeanBounces);
        jstools::parseJson(jsMeta, "AnomalyCount", AnomalyCount);
        jstools::parseJson(jsMeta, "GenerationDate", GenerationDate);
        jstools::parseJson(jsMeta, "Comment", Comment);
    }

    QJsonObject jsBin;
    if (!jstools::parseJson(json, "Binning", jsBin)) return "Surface LUT: \"Binning\" object not found";
    if (!jstools::parseJson(jsBin, "ThetaIncBins", ThetaIncBins)) return "Surface LUT: missing ThetaIncBins";
    if (!jstools::parseJson(jsBin, "ThetaOutBins", ThetaOutBins)) return "Surface LUT: missing ThetaOutBins";
    if (!jstools::parseJson(jsBin, "PhiOutBins",   PhiOutBins))   return "Surface LUT: missing PhiOutBins";

    QJsonArray ar;
    if (!jstools::parseJson(json, "Launched", ar) || !arrayToIntVector(ar, Launched))
        return "Surface LUT: bad or missing \"Launched\" array";
    if (!jstools::parseJson(json, "ReflectedCounts", ar) || !arrayToIntVector(ar, ReflectedCounts))
        return "Surface LUT: bad or missing \"ReflectedCounts\" array";
    if (!jstools::parseJson(json, "TransmittedCounts", ar) || !arrayToIntVector(ar, TransmittedCounts))
        return "Surface LUT: bad or missing \"TransmittedCounts\" array";
    if (!jstools::parseJson(json, "AbsorbedCounts", ar) || !arrayToIntVector(ar, AbsorbedCounts))
        return "Surface LUT: bad or missing \"AbsorbedCounts\" array";
    if (!jstools::parseJson(json, "ReflectedHist", ar) || !arrayToIntVectorOfVectors(ar, ReflectedHist))
        return "Surface LUT: bad or missing \"ReflectedHist\" array";
    if (!jstools::parseJson(json, "TransmittedHist", ar) || !arrayToIntVectorOfVectors(ar, TransmittedHist))
        return "Surface LUT: bad or missing \"TransmittedHist\" array";

    return check();
}

QString ALutSurfaceData::check() const
{
    if (ThetaIncBins < 1 || ThetaIncBins > 1000) return "Surface LUT: ThetaIncBins should be in [1, 1000]";
    if (ThetaOutBins < 1 || ThetaOutBins > 1000) return "Surface LUT: ThetaOutBins should be in [1, 1000]";
    if (PhiOutBins   < 1 || PhiOutBins   > 1000) return "Surface LUT: PhiOutBins should be in [1, 1000]";

    const size_t numBins = ThetaIncBins;
    if (Launched.size()          != numBins) return "Surface LUT: size of \"Launched\" does not match ThetaIncBins";
    if (ReflectedCounts.size()   != numBins) return "Surface LUT: size of \"ReflectedCounts\" does not match ThetaIncBins";
    if (TransmittedCounts.size() != numBins) return "Surface LUT: size of \"TransmittedCounts\" does not match ThetaIncBins";
    if (AbsorbedCounts.size()    != numBins) return "Surface LUT: size of \"AbsorbedCounts\" does not match ThetaIncBins";
    if (ReflectedHist.size()     != numBins) return "Surface LUT: size of \"ReflectedHist\" does not match ThetaIncBins";
    if (TransmittedHist.size()   != numBins) return "Surface LUT: size of \"TransmittedHist\" does not match ThetaIncBins";

    const size_t histSize = (size_t)ThetaOutBins * PhiOutBins;
    for (size_t iBin = 0; iBin < numBins; iBin++)
    {
        if (Launched[iBin] < 1)
            return QString("Surface LUT: no photons were launched for incidence bin %1").arg(iBin);
        if (ReflectedCounts[iBin] < 0 || TransmittedCounts[iBin] < 0 || AbsorbedCounts[iBin] < 0)
            return QString("Surface LUT: negative counts in incidence bin %1").arg(iBin);
        if (ReflectedCounts[iBin] + TransmittedCounts[iBin] + AbsorbedCounts[iBin] != Launched[iBin])
            return QString("Surface LUT: count conservation violated in incidence bin %1").arg(iBin);

        if (ReflectedHist[iBin].size()   != histSize) return QString("Surface LUT: bad ReflectedHist size in incidence bin %1").arg(iBin);
        if (TransmittedHist[iBin].size() != histSize) return QString("Surface LUT: bad TransmittedHist size in incidence bin %1").arg(iBin);

        long long sumR = 0, sumT = 0;
        for (int v : ReflectedHist[iBin])
        {
            if (v < 0) return QString("Surface LUT: negative ReflectedHist content in incidence bin %1").arg(iBin);
            sumR += v;
        }
        for (int v : TransmittedHist[iBin])
        {
            if (v < 0) return QString("Surface LUT: negative TransmittedHist content in incidence bin %1").arg(iBin);
            sumT += v;
        }
        if (sumR != ReflectedCounts[iBin])   return QString("Surface LUT: ReflectedHist does not sum to ReflectedCounts in incidence bin %1").arg(iBin);
        if (sumT != TransmittedCounts[iBin]) return QString("Surface LUT: TransmittedHist does not sum to TransmittedCounts in incidence bin %1").arg(iBin);
    }
    return "";
}

QString ALutSurfaceData::buildRuntime()
{
    RuntimeReady = false;

    QString err = check();
    if (!err.isEmpty()) return err;

    R.resize(ThetaIncBins);
    T.resize(ThetaIncBins);
    ReflCdf.assign(ThetaIncBins, {});
    TransCdf.assign(ThetaIncBins, {});

    for (int iBin = 0; iBin < ThetaIncBins; iBin++)
    {
        const double launched = Launched[iBin];
        R[iBin] = ReflectedCounts[iBin]   / launched;
        T[iBin] = TransmittedCounts[iBin] / launched;

        if (ReflectedCounts[iBin] > 0)
        {
            std::vector<double> & cdf = ReflCdf[iBin];
            cdf.resize(ReflectedHist[iBin].size());
            double sum = 0;
            for (size_t i = 0; i < cdf.size(); i++)
            {
                sum += ReflectedHist[iBin][i];
                cdf[i] = sum;
            }
            for (double & v : cdf) v /= sum;
        }
        if (TransmittedCounts[iBin] > 0)
        {
            std::vector<double> & cdf = TransCdf[iBin];
            cdf.resize(TransmittedHist[iBin].size());
            double sum = 0;
            for (size_t i = 0; i < cdf.size(); i++)
            {
                sum += TransmittedHist[iBin][i];
                cdf[i] = sum;
            }
            for (double & v : cdf) v /= sum;
        }
    }

    RuntimeReady = true;
    return "";
}

int ALutSurfaceData::selectThetaBin(double thetaDeg, double rnd) const
{
    const double binWidth = 90.0 / ThetaIncBins;
    const double x = thetaDeg / binWidth - 0.5;   // distance in bins from the center of bin 0

    const int    k0 = (int)std::floor(x);
    if (k0 <  0)                return 0;
    if (k0 >= ThetaIncBins - 1) return ThetaIncBins - 1;

    const double frac = x - k0;
    return (rnd < frac ? k0 + 1 : k0);
}

bool ALutSurfaceData::sampleOutgoing(bool reflected, int iThetaBin, double rnd1, double rnd2, double rnd3,
                                     double & thetaOut, double & phiOut) const
{
    const std::vector<double> & cdf = (reflected ? ReflCdf[iThetaBin] : TransCdf[iThetaBin]);
    if (cdf.empty()) return false;

    const size_t index = std::upper_bound(cdf.begin(), cdf.end(), rnd1) - cdf.begin();
    const size_t flatIndex = std::min(index, cdf.size() - 1);

    const int iTheta = flatIndex / PhiOutBins;
    const int iPhi   = flatIndex % PhiOutBins;

    thetaOut = (iTheta + rnd2) * (90.0  / ThetaOutBins);
    phiOut   = (iPhi   + rnd3) * (360.0 / PhiOutBins);

    // protect the photon tracer's direction sign checks at the 90 degrees bin edge
    if (thetaOut > 89.9) thetaOut = 89.9;

    return true;
}
