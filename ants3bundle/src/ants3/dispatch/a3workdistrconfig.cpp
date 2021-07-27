#include "a3workdistrconfig.h"
#include "ajsontools.h"

void A3NodeWorkerConfig::writeToJson(QJsonObject &json) const
{
    json["ConfigFile"]  = ConfigFile;
    json["InputFiles"]  = jstools::vectorQStringsToJsonArray(InputFiles);
    json["OutputFiles"] = jstools::vectorQStringsToJsonArray(OutputFiles);
}

void A3NodeWorkerConfig::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "ConfigFile",  ConfigFile);
    jstools::parseJson(json, "InputFiles",  InputFiles);
    jstools::parseJson(json, "OutputFiles", OutputFiles);
}

// ----

bool A3WorkNodeConfig::isLocalNode() const
{
    return Address.isEmpty();
}

void A3WorkNodeConfig::writeToJson(QJsonObject & json) const
{
    json["Address"] = Address;
    json["Port"]    = Port;

    QJsonArray wA;
    for (const A3NodeWorkerConfig & w : Workers)
    {
        QJsonObject js;
            w.writeToJson(js);
        wA.append(js);
    }
    json["Workers"] = wA;
}

void A3WorkNodeConfig::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "Address", Address);
    jstools::parseJson(json, "Port",    Port);

    Workers.clear();
    QJsonArray wA;
    jstools::parseJson(json, "Workers", wA);
    Workers.resize(wA.size());
    for (int i=0; i<wA.size(); i++)
        Workers[i].readFromJson(wA[i].toObject());
}

// -----

void A3WorkDistrConfig::writeToJson(QJsonObject & json) const
{
    json["Command"]     = Command;
    json["ExchangeDir"] = ExchangeDir;
    json["CommonFiles"] = jstools::vectorQStringsToJsonArray(CommonFiles);

    QJsonArray nodesA;
    for (const A3WorkNodeConfig & wn : Nodes)
    {
        QJsonObject js;
            wn.writeToJson(js);
        nodesA.append(js);
    }
    json["Nodes"] = nodesA;
}

void A3WorkDistrConfig::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "Command", Command);
    jstools::parseJson(json, "ExchangeDir", ExchangeDir);
    jstools::parseJson(json, "CommonFiles", CommonFiles);

    Nodes.clear();
    QJsonArray nodesA;
    jstools::parseJson(json, "Nodes", nodesA);
    Nodes.resize(nodesA.size());
    for (int i=0; i<nodesA.size(); i++)
        Nodes[i].readFromJson(nodesA[i].toObject());
}
