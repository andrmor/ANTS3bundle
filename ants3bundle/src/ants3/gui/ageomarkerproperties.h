#ifndef AGEOMARKERPROPERTIES_H
#define AGEOMARKERPROPERTIES_H

#include "ageomarkerclass.h"

#include <map>
#include <QString>

class QJsonObject;

class AGeoMarkerProperties
{
public:
    int   Style = 1;
    float Size  = 2.0;
    int   Color = 1;
    int   LineWidth = 1;
};

class AGeoMarkerPropDatabase
{
public:
    std::map<QString,AGeoMarkerProperties> Data;
    double SizeMultiplier = 1.0;

    typedef std::map<int,int> AMarkConvMap;
    static AMarkConvMap cMap;

    void fillDefault();

    void applyProperties(AGeoMarkerClass * gm);

    AGeoMarkerProperties getProperties(EGeoMarkerType type);
    void                 setProperties(EGeoMarkerType type, AGeoMarkerProperties properties);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    QString getInfo(QString type);



};

#endif // AGEOMARKERPROPERTIES_H
