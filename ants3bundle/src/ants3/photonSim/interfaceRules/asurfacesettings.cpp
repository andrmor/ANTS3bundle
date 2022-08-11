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
    case Polished : str = "Polished"; break;
    case GaussSimplistic : str = "GaussSimplistic"; break;
    }
    json["Model"] = str;
}

void ASurfaceSettings::readFromJson(const QJsonObject & json)
{
    QString str;
    jstools::parseJson(json, "Model", str);
    if      (str == "Polished")        Model = Polished;
    else if (str == "GaussSimplistic") Model = GaussSimplistic;
    else
    {
        qWarning() << "Unknown rough surface model!";
        AErrorHub::addQError("Unknown rough surface model!");
        Model = Polished;
        return;
    }
}
