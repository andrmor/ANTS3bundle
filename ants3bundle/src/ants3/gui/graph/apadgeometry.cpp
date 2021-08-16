#include "apadgeometry.h"
#include "ajsontools.h"

void APadGeometry::writeToJson(QJsonObject & json) const
{
    json["xLow"]  = xLow;
    json["yLow"]  = yLow;
    json["xHigh"] = xHigh;
    json["yHigh"] = yHigh;
}

void APadGeometry::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "xLow",  xLow);
    jstools::parseJson(json, "yLow",  yLow);
    jstools::parseJson(json, "xHigh", xHigh);
    jstools::parseJson(json, "yHigh", yHigh);
}
