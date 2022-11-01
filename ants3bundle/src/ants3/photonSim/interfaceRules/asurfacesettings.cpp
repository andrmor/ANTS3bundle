#include "asurfacesettings.h"
#include "ajsontools.h"
#include "aerrorhub.h"

#include <QDebug>

void ASurfaceSettings::writeToJson(QJsonObject & json) const
{
    QString str;
    switch (Model)
    {
    default:
    case Polished :
        str = "Polished";
        break;
    case Glisur :
        str = "Glisur";
        json["Polish"] = Polish;
        break;
    case Unified :
        str = "Unified";
        json["SigmaAlpha"] = SigmaAlpha;
        break;
    }
    json["Model"] = str;
}

void ASurfaceSettings::readFromJson(const QJsonObject & json)
{
    QString str;
    jstools::parseJson(json, "Model", str);
    if      (str == "Polished")        Model = Polished;
    else if (str == "Glisur")
    {
        Model = Glisur;
        jstools::parseJson(json, "Polish", Polish);
    }
    else if (str == "Unified")
    {
        Model = Unified;
        jstools::parseJson(json, "SigmaAlpha", SigmaAlpha);
    }
    else
    {
        qWarning() << "Unknown optical surface model!";
        AErrorHub::addQError("Unknown optical surface model!");
        Model = Polished;
        return;
    }
}
