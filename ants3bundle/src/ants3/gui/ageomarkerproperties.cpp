#include "ageomarkerproperties.h"

#include <QDebug>

void AGeoMarkerPropDatabase::fillDefault()
{
    Data.clear();

    //enum class EGeoMarkerType {Undefined, PrimarySource, PointOfOrigin, PosTrue, PosReconstructed};

    Data["Undefined"]        = {1,2,1};
    Data["PrimarySource"]    = {1,2,1};
    Data["PointOfOrigin"]    = {1,2,1};
    Data["PosTrue"]          = {1,2,1};
    Data["PosReconstructed"] = {1,2,1};
}

AGeoMarkerProperties AGeoMarkerPropDatabase::getProperties(EGeoMarkerType type)
{
    switch (type)
    {
    case EGeoMarkerType::Undefined        : return Data["Undefined"];
    case EGeoMarkerType::PrimarySource    : return Data["PrimarySource"];
    case EGeoMarkerType::PointOfOrigin    : return Data["PointOfOrigin"];
    case EGeoMarkerType::PosTrue          : return Data["PosTrue"];
    case EGeoMarkerType::PosReconstructed : return Data["PosReconstructed"];
    default: qCritical() << "AGeoMarkerPropDatabase::getProperties: not implemented type"; break;
    }

    return {1,2,1};
}

void AGeoMarkerPropDatabase::setProperties(EGeoMarkerType type, AGeoMarkerProperties properties)
{
    switch (type)
    {
    case EGeoMarkerType::Undefined        : Data["Undefined"]        = properties; return;
    case EGeoMarkerType::PrimarySource    : Data["PrimarySource"]    = properties; return;
    case EGeoMarkerType::PointOfOrigin    : Data["PointOfOrigin"]    = properties; return;
    case EGeoMarkerType::PosTrue          : Data["PosTrue"]          = properties; return;
    case EGeoMarkerType::PosReconstructed : Data["PosReconstructed"] = properties; return;
    default: qCritical() << "AGeoMarkerPropDatabase::setProperties: not implemented type";
    }
}

void AGeoMarkerPropDatabase::writeToJson(QJsonObject & json) const
{

}

void AGeoMarkerPropDatabase::readFromJson(const QJsonObject & json)
{

}
