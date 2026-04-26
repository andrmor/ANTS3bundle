#include "ageomarkerproperties.h"

#include <QDebug>

AGeoMarkerPropDatabase::AMarkConvMap AGeoMarkerPropDatabase::cMap = {{2,50}, {3,51}, {4,53}, {5,52,}, {24,53}, {25,54}, {26,55}, {27,56}, {28,57}, {30,58},
                                                                     {31,51}, {32,59}, {35,60}, {36,61}, {37,62}, {38,63}, {40,64}, {42,65}, {44,66}, {46,67}};

void AGeoMarkerPropDatabase::fillDefault()
{
    Data.clear();

    //enum class EGeoMarkerType {Undefined, PrimarySource, PointOfOrigin, PosTrue, PosReconstructed};

    Data["Undefined"]        = {1,  2.0, 1};
    Data["PrimarySource"]    = {3,  3.0, 51};
    Data["PointOfOrigin"]    = {2,  1.5, 860};
    Data["PosTrue"]          = {5,  2.0, 860};
    Data["PosReconstructed"] = {2,  2.0, 2};

    SizeMultiplier = 1.0;
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
    int style = props.Style;
    if (props.LineWidth > 1)
    {
        if (AGeoMarkerPropDatabase::cMap.count(style) > 0)
            style = AGeoMarkerPropDatabase::cMap[style] + 18 * (props.LineWidth - 2);
    }
    qDebug() << "aaaaaaaaaaaaaa" << props.Style << "with line width of" << props.LineWidth << "-->" << style;

    gm->SetMarkerStyle(style);
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

#include "ajsontools.h"
void AGeoMarkerPropDatabase::writeToJson(QJsonObject & json) const
{
    json["SizeMultiplier"] = SizeMultiplier;

    QJsonArray ar;
    for (const auto & [type, props] : Data)
    {
        QJsonObject js;
            js["Type"] = type;
            QJsonArray el;
            el << props.Style << props.Size << props.Color << props.LineWidth;
            js["Properties"] = el;
        ar.push_back(js);
    }
    json["PropertiesByType"] = ar;
}

void AGeoMarkerPropDatabase::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "SizeMultiplier", SizeMultiplier);

    QJsonArray ar;
    jstools::parseJson(json, "PropertiesByType", ar);
    for (int i = 0; i < ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
            QString type = "Undefined"; // safe to overrite with anything
            jstools::parseJson(js, "Type", type);
            QJsonArray el;
            jstools::parseJson(js, "Properties", el);
            if (el.size() > 3)
            {
                Data[type] = {el[0].toInt(), (float)el[1].toDouble(), el[2].toInt(), el[3].toInt()}; // style, size, color
            }
    }
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
