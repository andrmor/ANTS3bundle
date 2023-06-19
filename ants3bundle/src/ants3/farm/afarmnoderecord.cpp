#include "afarmnoderecord.h"
#include "ajsontools.h"

void AFarmNodeRecord::writeToJson(QJsonObject & json) const
{
    json["Name"]        = Name;
    json["Address"]     = Address;
    json["Port"]        = Port;
    json["Processes"]   = Processes;
    json["SpeedFactor"] = SpeedFactor;
    json["Enabled"]     = Enabled;
}

void AFarmNodeRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Name", Name);
    jstools::parseJson(json, "Address", Address);
    jstools::parseJson(json, "Port", Port);
    jstools::parseJson(json, "Processes", Processes);
    jstools::parseJson(json, "SpeedFactor", SpeedFactor);
    jstools::parseJson(json, "Enabled", Enabled);

    Status      = Unknown;
    Checked     = false;
}
