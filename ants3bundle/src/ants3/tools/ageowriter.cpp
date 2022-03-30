#include "ageowriter.h"
#include "asensorhub.h"
#include "amonitorhub.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ageoshape.h"

#include "TVirtualGeoTrack.h"
#include "TGeoManager.h"

AGeoWriter::AGeoWriter()
{
    generateSymbolMap();
}

QString AGeoWriter::showText(const QVector<QString> & strData, int color, EDraw onWhat)
{
    const ASensorHub  & SensorHub  = ASensorHub ::getConstInstance();
    const AMonitorHub & MonitorHub = AMonitorHub::getConstInstance();
    TGeoManager * GeoManager = AGeometryHub::getInstance().GeoManager;

    int numObj = 0;
    switch (onWhat)
    {
    case PMs      : numObj = SensorHub.countSensors();                        break;
    case PhotMons : numObj = MonitorHub.countMonitors(AMonitorHub::Photon);   break;
    case PartMons : numObj = MonitorHub.countMonitors(AMonitorHub::Particle); break;
    }

    if (strData.size() != numObj) return "Show text: mismatch in vector sizes";


    //max number of symbols to show
    int MaxSymbols = 0;
    for (int i=0; i<numObj; i++)
        if (strData[i].size() > MaxSymbols)
            MaxSymbols = strData[i].size();
    if (MaxSymbols == 0) MaxSymbols = 1;

    for (int iObj = 0; iObj < numObj; iObj++)
    {
        QString str = strData[iObj];
        if (str.isEmpty()) continue;

        int numDigits = str.size();
        if (str.right(1) == "F") numDigits--;
        AVector3 centerPos;
        double size;
        switch (onWhat)
        {
        case PMs :
          {
            const ASensorModel * sensorModel = SensorHub.sensorModel(iObj);
            if (sensorModel)
            {
                centerPos = SensorHub.SensorData[iObj].Position;
                size      = SensorHub.SensorData[iObj].GeoObj->Shape->minSize();    // !!!*** expand minSize for other shapes!!!
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
        }
        if (size == 0) size = 2.0;
        size = size / 3.0 / (0.5 + 0.5*MaxSymbols);
        int lineWidth = 2;
        //if (size<2) lineWidth = 1;

        qDebug() <<"("<< centerPos[0] << centerPos[1]<< centerPos[2]<< ")" << size << str;

        for (int iDig = 0; iDig < numDigits; iDig++)
        {
            QString str1 = str.mid(iDig, 1);

            int isymbol = -1;
            for (int i = 0; i < SymbolMap.size(); i++)
                if (str1 == SymbolMap[i])
                    isymbol = i;

            Int_t track_index = GeoManager->AddTrack(2, 22);
            TVirtualGeoTrack * track = GeoManager->GetTrack(track_index);
            if (str.right(1) == "F") track->SetLineColor(kRed);
            else                     track->SetLineColor(color);
            track->SetLineWidth(lineWidth);

            if (isymbol > -1)
                for (int i=0; i<numbersX[isymbol].size(); i++)
                {
                    double x = centerPos[0] - 2.6 * size * (0.5 * (numDigits-1) - 1.0 * iDig) + size * numbersX[isymbol][i];
                    double y = centerPos[1] + size * numbersY[isymbol][i];
                    track->AddPoint(x, y, centerPos[2], 0);
                }
        }
    }

    return QString();
}

void AGeoWriter::generateSymbolMap()
{
    //0
    SymbolMap << "0";
    numbersX.append({-1,1,1,-1,-1});
    numbersY.append({1.62,1.62,-1.62,-1.62,1.62});
    //1
    SymbolMap << "1";
    numbersX.append({-0.3,0.3,0.3});
    numbersY.append({0.42,1.62,-1.62});
    //2
    SymbolMap << "2";
    numbersX.append({-1,1,1,-1,-1,1});
    numbersY.append({1.62,1.62,0,0,-1.62,-1.62});
    //3
    SymbolMap << "3";
    numbersX.append({-1,1,1,-1,1,1,-1});
    numbersY.append({1.62,1.62,0,0,0,-1.62,-1.62});
    //4
    SymbolMap << "4";
    numbersX.append({-1,-1,1,1,1});
    numbersY.append({1.62,0,0,1.62,-1.62});
    //5
    SymbolMap << "5";
    numbersX.append({1,-1,-1,1,1,-1});
    numbersY.append({1.62,1.62,0,0,-1.62,-1.62});
    //6
    SymbolMap << "6";
    numbersX.append({1,-1,-1,1,1,-1});
    numbersY.append({1.62,1.62,-1.62,-1.62,0,0});
    //7
    SymbolMap << "7";
    numbersX.append({-1,1,1});
    numbersY.append({1.62,1.62,-1.62});
    //8
    SymbolMap << "8";
    numbersX.append({-1,1,1,-1,-1,1,1});
    numbersY.append({0,0,-1.62,-1.62,1.62,1.62,0});
    //9
    SymbolMap << "9";
    numbersX.append({-1   , 1   ,   1,  -1, -1,1});
    numbersY.append({-1.62,-1.62,1.62,1.62,  0,0});
    //.
    SymbolMap << ".";
    numbersX.append({-0.2, 0.2, 0.2,-0.2,-0.2});
    numbersY.append({-1.2,-1.2,-1.6,-1.6,-1.2});
    //-
    SymbolMap << "-";
    numbersX.append({-1, 1});
    numbersY.append({ 0, 0});
}
