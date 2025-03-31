#include "amultidrawrecord.h"
#include "ajsontools.h"

void AMultidrawRecord::init()
{
    const int size = BasketItems.size();

    if      (size == 2) { NumX = 2; NumY = 1;}
    else if (size == 3) { NumX = 3; NumY = 1;}
    else if (size == 4) { NumX = 2; NumY = 2;}
    else if (size == 5) { NumX = 2; NumY = 3;}
    else if (size == 6) { NumX = 2; NumY = 3;}
    else                { NumX = 3; NumY = 3;}
}

void AMultidrawRecord::writeToJson(QJsonObject & json) const
{
    QJsonArray ar;
    for (int i : BasketItems) ar.push_back(i);
    json["BasketItems"] = ar;

    json["NumX"] = NumX;
    json["NumY"] = NumY;

    json["EnforceMargins"] = EnforceMargins;
    json["MarginLeft"]     = MarginLeft;
    json["MarginRight"]    = MarginRight;
    json["MarginTop"]      = MarginTop;
    json["MarginBottom"]   = MarginBottom;

    json["ScaleLabels"]    = ScaleLabels;

    json["ScaleOffsetTitleX"] = ScaleOffsetTitleX;
    json["ScaleOffsetTitleY"] = ScaleOffsetTitleY;
    json["ScaleOffsetTitleZ"] = ScaleOffsetTitleZ;

    json["ScaleOffsetAxisX"] = ScaleOffsetAxisX;
    json["ScaleOffsetAxisY"] = ScaleOffsetAxisY;
    json["ScaleOffsetAxisZ"] = ScaleOffsetAxisZ;
}

void AMultidrawRecord::readFronJson(const QJsonObject & json)
{
    QJsonArray ar;
    jstools::parseJson(json, "BasketItems", ar);
    BasketItems.clear();
    for (int i = 0; i < ar.size(); i++)
        BasketItems.push_back(ar[i].toInt());

    jstools::parseJson(json, "NumX", NumX);
    jstools::parseJson(json, "NumY", NumY);

    jstools::parseJson(json, "EnforceMargins", EnforceMargins);
    jstools::parseJson(json, "MarginLeft",     MarginLeft);
    jstools::parseJson(json, "MarginRight",    MarginRight);
    jstools::parseJson(json, "MarginTop",      MarginTop);
    jstools::parseJson(json, "MarginBottom",   MarginBottom);

    jstools::parseJson(json, "ScaleLabels",    ScaleLabels);

    jstools::parseJson(json, "ScaleOffsetTitleX", ScaleOffsetTitleX);
    jstools::parseJson(json, "ScaleOffsetTitleY", ScaleOffsetTitleY);
    jstools::parseJson(json, "ScaleOffsetTitleZ", ScaleOffsetTitleZ);

    jstools::parseJson(json, "ScaleOffsetAxisX", ScaleOffsetAxisX);
    jstools::parseJson(json, "ScaleOffsetAxisY", ScaleOffsetAxisY);
    jstools::parseJson(json, "ScaleOffsetAxisZ", ScaleOffsetAxisZ);
}
