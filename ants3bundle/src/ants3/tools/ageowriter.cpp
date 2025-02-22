#include "ageowriter.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "acalorimeterhub.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ajsontools.h"

#include <QDebug>

#include "TVirtualGeoTrack.h"
#include "TGeoManager.h"

AGeoSymbol::AGeoSymbol(std::vector<double> x, std::vector<double> y)
{
    if (x.size() != y.size())
    {
        qWarning() << "Mismatch in x and y vector size in AGeoSymbol constr";
        return;
    }

    for (size_t i = 0; i < x.size(); i++)
        Coordinates.push_back({x[i], y[i]});
}

AGeoWriter::AGeoWriter()
{
    generateSymbolMap();
}

void AGeoWriter::generateSymbolMap()
{
    SymbolMap["0"] = { {-1,    1,     1,    -1,   -1},
                       { 1.62, 1.62, -1.62, -1.62, 1.62} };
    SymbolMap["1"] = { {-0.3,  0.3,   0.3},
                       { 0.42, 1.62, -1.62} };
    SymbolMap["2"] = { {-1,   1,    1, -1, -1,     1},
                       { 1.62,1.62, 0,  0, -1.62, -1.62} };
    SymbolMap["3"] = { {-1,    1,    1, -1, 1,  1,    -1},
                       { 1.62, 1.62, 0,  0, 0, -1.62, -1.62} };
    SymbolMap["4"] = { { -1,   -1, 1, 1,    1},
                       {  1.62, 0, 0, 1.62,-1.62} };
    SymbolMap["5"] = { {1,    -1,  -1, 1,  1,   -1},
                       {1.62, 1.62, 0, 0, -1.62,-1.62} };
    SymbolMap["6"] = { {1,   -1,    -1,     1,    1, -1},
                       {1.62, 1.62, -1.62, -1.62, 0,  0} };
    SymbolMap["7"] = { { -1,    1,     1},
                       {  1.62, 1.62, -1.62} };
    SymbolMap["8"] = { {-1, 1,  1,    -1,   -1,    1,    1},
                       { 0, 0, -1.62, -1.62, 1.62, 1.62, 0} };
    SymbolMap["9"] = { {-1,     1,    1,   -1,   -1, 1},
                       {-1.62, -1.62, 1.62, 1.62, 0, 0} };
    SymbolMap["."] = { {-0.2, 0.2, 0.2, -0.2, -0.2},
                       {-1.2,-1.2,-1.6, -1.6, -1.2} };
    SymbolMap["-"] = { {-1, 1},
                       { 0, 0} };
}

#include "TVector3.h"
#include "aphotonfunctionalhub.h"
void AGeoWriter::drawText(const std::vector<QString> & textVector, int color, EDraw onWhat)
{
    const ASensorHub      & SensorHub  = ASensorHub ::getConstInstance();
    const AMonitorHub     & MonitorHub = AMonitorHub::getConstInstance();
    const ACalorimeterHub & CalHub     = ACalorimeterHub::getConstInstance();
    const AGeometryHub    & GeoHub     = AGeometryHub::getConstInstance();

    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;

    size_t numObj = 0;
    switch (onWhat)
    {
    case Sensors          : numObj = SensorHub.countSensors();                        break;
    case PhotMons         : numObj = MonitorHub.countMonitors(AMonitorHub::Photon);   break;
    case PartMons         : numObj = MonitorHub.countMonitors(AMonitorHub::Particle); break;
    case Calorimeters     : numObj = CalHub.countCalorimeters();                      break;
    case Analyzers        : numObj = GeoHub.countParticleAnalyzers();                 break;
    case PhotonFunctional : numObj = GeoHub.PhotonFunctionals.size();                 break;
    case Scints           : numObj = GeoHub.countScintillators();                     break;
    }

    if (textVector.size() != numObj)
    {
        qWarning() << "AGeoWriter::drawText(): mismatch in vector sizes";
        return;
    }

    double longi = Longitude * 3.1415926535 / 180.0;
    double lati  = Latitude  * 3.1415926535 / 180.0;
    TVector3 os(0, 1.0, 0);
    os.RotateZ(longi);

    //max number of symbols to show
    int MaxSymbols = 0;
    for (const QString & txt : textVector)
        if (txt.size() > MaxSymbols)
            MaxSymbols = txt.size();
    if (MaxSymbols == 0) MaxSymbols = 1;

    for (size_t iObj = 0; iObj < numObj; iObj++)
    {
        const QString & str = textVector[iObj];
        if (str.isEmpty()) continue;

        int numDigits = str.size();
        if (str.right(1) == "F") numDigits--;
        AVector3 centerPos;
        double size;
        switch (onWhat)
        {
        case Sensors :
          {
            centerPos = SensorHub.getPositionFast(iObj);
            size = SizeForSensors;
            break;
          }
        case PhotMons :
            centerPos = MonitorHub.PhotonMonitors[iObj].Position;
            size = SizeForMonitors;
            break;
        case PartMons :
            centerPos = MonitorHub.ParticleMonitors[iObj].Position;
            size = SizeForMonitors;
            break;
        case Calorimeters :
            centerPos = CalHub.Calorimeters[iObj].Position;
            size = SizeForCalorimeters;
            break;
        case Analyzers :
            centerPos = std::get<2>(GeoHub.ParticleAnalyzers[iObj]);
            size = SizeForAnalyzers;
            break;
        case PhotonFunctional :
            centerPos = std::get<2>(GeoHub.PhotonFunctionals[iObj]);
            size = SizeForPhotFuncts;
            break;
        case Scints :
            centerPos = GeoHub.getScintillatorPosition(iObj);
            size = SizeForScints;
            break;
        }

        //size = size / 3.0 / (0.5 + 0.5*MaxSymbols);
        size /= (1.62 * 2.0);
        if (size == 0) size = 10.0;
        int lineWidth = 2;
        //if (size<2) lineWidth = 1;

        for (int iDig = 0; iDig < numDigits; iDig++)
        {
            const QString str1 = str.mid(iDig, 1);

            const auto it = SymbolMap.find(str1);
            if (it == SymbolMap.end()) continue;

            int track_index = GeoManager->AddTrack(2, 22);
            TVirtualGeoTrack * track = GeoManager->GetTrack(track_index);
            if (str.right(1) == "F") track->SetLineColor(kRed);
            else                     track->SetLineColor(color);
            track->SetLineWidth(lineWidth);

            for (const auto & pair : it->second.Coordinates)
            {
                const double x = -2.6 * size * (0.5 * (numDigits-1) - 1.0 * iDig) + size * pair.first;
                const double y = size * pair.second;

                TVector3 pos(-y, x, 0);
                pos.RotateZ(longi);
                pos.Rotate(lati, os);

                track->AddPoint(pos[0] + centerPos[0], pos[1] + centerPos[1], pos[2] + centerPos[2], 0);
            }
        }
    }
}

void AGeoWriter::writeToJson(QJsonObject & json) const
{
    json["SizeForScints"]       = SizeForScints;
    json["SizeForSensors"]      = SizeForSensors;
    json["SizeForMonitors"]     = SizeForMonitors;
    json["SizeForCalorimeters"] = SizeForCalorimeters;
    json["SizeForAnalyzers"]    = SizeForAnalyzers;
    json["SizeForPhotFuncts"]   = SizeForPhotFuncts;
}

void AGeoWriter::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "SizeForScints",       SizeForScints);
    jstools::parseJson(json, "SizeForSensors",      SizeForSensors);
    jstools::parseJson(json, "SizeForMonitors",     SizeForMonitors);
    jstools::parseJson(json, "SizeForCalorimeters", SizeForCalorimeters);
    jstools::parseJson(json, "SizeForAnalyzers",    SizeForAnalyzers);
    jstools::parseJson(json, "SizeForPhotFuncts",   SizeForPhotFuncts);
}
