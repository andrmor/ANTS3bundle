#include "ageowriter.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "acalorimeterhub.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ageoshape.h"

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

QString AGeoWriter::drawText(const std::vector<QString> & textVector, int color, EDraw onWhat)
{
    const ASensorHub      & SensorHub  = ASensorHub ::getConstInstance();
    const AMonitorHub     & MonitorHub = AMonitorHub::getConstInstance();
    const ACalorimeterHub & CalHub     = ACalorimeterHub::getConstInstance();
    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;

    size_t numObj = 0;
    switch (onWhat)
    {
    case Sensors      : numObj = SensorHub.countSensors();                        break;
    case PhotMons     : numObj = MonitorHub.countMonitors(AMonitorHub::Photon);   break;
    case PartMons     : numObj = MonitorHub.countMonitors(AMonitorHub::Particle); break;
    case Calorimeters : numObj = CalHub.countCalorimeters();                      break;
    }

    if (textVector.size() != numObj) return "Show text: mismatch in vector sizes";

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
            const ASensorModel * sensorModel = SensorHub.sensorModel(iObj);
            if (sensorModel)
            {
                centerPos = SensorHub.getPositionFast(iObj);
                size      = SensorHub.getMinSizeFast(iObj);    // !!!*** expand minSize for other shapes!!!
            }
            break;
          }
        case PhotMons :
            centerPos = MonitorHub.PhotonMonitors[iObj].Position;
            size      = MonitorHub.PhotonMonitors[iObj].GeoObj->Shape->minSize();
            break;
        case PartMons :
            centerPos = MonitorHub.ParticleMonitors[iObj].Position;
            size      = MonitorHub.ParticleMonitors[iObj].GeoObj->Shape->minSize();
            break;
        case Calorimeters :
            centerPos = CalHub.Calorimeters[iObj].Position;
            size      = CalHub.Calorimeters[iObj].GeoObj->Shape->minSize();
            break;
        }
        if (size == 0) size = 2.0;
        size = size / 3.0 / (0.5 + 0.5*MaxSymbols);
        int lineWidth = 2;
        //if (size<2) lineWidth = 1;

        //qDebug() <<"("<< centerPos[0] << centerPos[1]<< centerPos[2]<< ")" << size << str;

        for (int iDig = 0; iDig < numDigits; iDig++)
        {
            QString str1 = str.mid(iDig, 1);

            auto it = SymbolMap.find(str1);
            if (it == SymbolMap.end()) continue;

            Int_t track_index = GeoManager->AddTrack(2, 22);
            TVirtualGeoTrack * track = GeoManager->GetTrack(track_index);
            if (str.right(1) == "F") track->SetLineColor(kRed);
            else                     track->SetLineColor(color);
            track->SetLineWidth(lineWidth);

            for (const auto & pair : it->second.Coordinates)
            {
                double x = centerPos[0] - 2.6 * size * (0.5 * (numDigits-1) - 1.0 * iDig) + size * pair.first;
                double y = centerPos[1] + size * pair.second;
                track->AddPoint(x, y, centerPos[2], 0);
            }
        }
    }

    return QString();
}
