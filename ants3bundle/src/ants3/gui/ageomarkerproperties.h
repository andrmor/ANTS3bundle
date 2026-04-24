#ifndef AGEOMARKERPROPERTIES_H
#define AGEOMARKERPROPERTIES_H

#include "ageomarkerclass.h"

#include <map>
#include <QString>

class QJsonObject;

class AGeoMarkerProperties
{
public:
    int Style = 1;
    int Size  = 2;
    int Color = 1;
};

class AGeoMarkerPropDatabase
{
public:
    std::map<QString,AGeoMarkerProperties> Data;

    void fillDefault();

    void applyProperties(AGeoMarkerClass * gm);

    AGeoMarkerProperties getProperties(EGeoMarkerType type);
    void                 setProperties(EGeoMarkerType type, AGeoMarkerProperties properties);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

#endif // AGEOMARKERPROPERTIES_H
