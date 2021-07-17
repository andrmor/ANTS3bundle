#include "a3config.h"
#include "ajsontools.h"
#include "a3geometry.h"
#include "a3mathub.h"

#include <QDebug>

A3Config & A3Config::getInstance()
{
    static A3Config instance;
    return instance;
}

A3Config::A3Config()
{
    for (int i=0; i<25; i++)
        lines += QString("%0-abcdef\n").arg(i);
}

void A3Config::writeDetectorConfig(QJsonObject & json) const
{
    QJsonObject jsonDet;
    {
        QJsonObject jsGeo;
            A3Geometry::getInstance().writeToJson(jsGeo);
        jsonDet["Geometry"] = jsGeo;

        QJsonArray jsMatAr;
            A3MatHub::getInstance().writeToJsonAr(jsMatAr);
        jsonDet["Materials"] = jsMatAr;
    }
    json["Detector"] = jsonDet;
}

void A3Config::readDetectorConfig(const QJsonObject & json)
{
    QJsonObject jsonDet;
    bool ok = jstools::parseJson(json, "Detector", jsonDet);
    if (!ok)
    {
        qWarning() << "json does not contain detector settings!";
        return;
    }

    QJsonObject jsGeo;
    ok = jstools::parseJson(jsonDet, "Geometry", jsGeo);
    if (ok) A3Geometry::getInstance().readFromJson(jsGeo);

    QJsonArray jsMatAr;
    ok = jstools::parseJson(jsonDet, "Materials", jsMatAr);
    if (ok) A3MatHub::getInstance().readFromJsonAr(jsMatAr);
}

void A3Config::writeAllConfig(QJsonObject & json) const
{
    writeDetectorConfig(json);
    // ...
}

void A3Config::readAllConfig(const QJsonObject & json)
{
    readDetectorConfig(json);
    // ...
}

