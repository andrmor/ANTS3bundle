#include "adrawmarginsrecord.h"
#include "ajsontools.h"

void ADrawMarginsRecord::writeToJson(QJsonObject & json) const
{
    json["Top"]       = Top;
    json["Bottom"]    = Bottom;
    json["Left"]      = Left;
    json["Right"]     = Right;

    json["RightForZ"] = RightForZ;
}

void ADrawMarginsRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Top",       Top);
    jstools::parseJson(json, "Bottom",    Bottom);
    jstools::parseJson(json, "Left",      Left);
    jstools::parseJson(json, "Right",     Right);

    jstools::parseJson(json, "RightForZ", RightForZ);
}
