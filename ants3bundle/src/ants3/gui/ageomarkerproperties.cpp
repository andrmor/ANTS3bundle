#include "ageomarkerproperties.h"

#include <QDebug>

void AGeoMarkerPropDatabase::fillDefault()
{
    Data.clear();

    //enum class EGeoMarkerType {Undefined, PrimarySource, PointOfOrigin, PosTrue, PosReconstructed};

    Data["Undefined"]        = {1,  2.0, 1};
    Data["PrimarySource"]    = {3,  3.0, 51};
    Data["PointOfOrigin"]    = {2,  1.5, 860};
    Data["PosTrue"]          = {5,  2.0, 860};
    Data["PosReconstructed"] = {2,  2.0, 2};
}

void AGeoMarkerPropDatabase::applyProperties(AGeoMarkerClass * gm)
{
    AGeoMarkerProperties props;
    switch (gm->Type)
    {
    case EGeoMarkerType::Undefined        : return;
    case EGeoMarkerType::PrimarySource    : props = Data["PrimarySource"];    break;
    case EGeoMarkerType::PointOfOrigin    : props = Data["PointOfOrigin"];    break;
    case EGeoMarkerType::PosTrue          : props = Data["PosTrue"];          break;
    case EGeoMarkerType::PosReconstructed : props = Data["PosReconstructed"]; break;
    default: qCritical() << "AGeoMarkerPropDatabase::applyProperties: not implemented type"; break;
    }

    gm->SetMarkerStyle(props.Style);
    gm->SetMarkerSize (props.Size * SizeMultiplier);
    gm->SetMarkerColor(props.Color);
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

QString AGeoMarkerPropDatabase::getInfo(QString type)
{
    if (type == "Undefined")        return "Custom markers and most of marker types created in scripts";
    if (type == "PrimarySource")    return "Markers of the primary sources";
    if (type == "PointOfOrigin")    return "Markers of the generated positions of, e.g., primary particles";
    if (type == "PosTrue")          return "Markers of the generated photon bombs and 'true' positions plotted from script";
    if (type == "PosReconstructed") return "Markers of the 'reconstructed' positions plotted from scripts";

    return "Info is not provided for this type";
}
