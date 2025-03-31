#include "adrawobject.h"
#include "ajsontools.h"

#include "TNamed.h"

ADrawObject::ADrawObject(TObject *pointer, const char *options)
{
    Pointer = pointer;
    Options = options;
    extractName();
}

ADrawObject::ADrawObject(TObject *pointer, const QString &options, bool enabled)
{
    Pointer = pointer;
    Options = options;
    bEnabled = enabled;
    extractName();
}

ADrawObject::ADrawObject(TObject *pointer, const QString &options, bool enabled, bool bLogScaleX, bool bLogScaleY) :
    Pointer(pointer), Options(options), bEnabled(enabled), bLogScaleX(bLogScaleX), bLogScaleY(bLogScaleY)
{
    extractName();
}

void ADrawObject::extractName()
{
    if (!Pointer) return;

    TNamed * tn = dynamic_cast<TNamed*>(Pointer);
    if (tn) Name = tn->GetTitle();
}

void ADrawObject::writeToJson(QJsonObject & json) const
{
    json["Name"]      = Name;
    json["Options"]   = Options;
    json["Enabled"]   = bEnabled;
    json["LogX"]      = bLogScaleX;
    json["LogY"]      = bLogScaleY;

    if (Multidraw)
    {
        QJsonObject js;
        MultidrawSettings.writeToJson(js);
        json["Multidraw"] = js;
    }
}

void ADrawObject::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Name",      Name);
    jstools::parseJson(json, "Options",   Options);
    jstools::parseJson(json, "Enabled",   bEnabled);
    jstools::parseJson(json, "LogX",      bLogScaleX);
    jstools::parseJson(json, "LogY",      bLogScaleY);

    QJsonObject js;
    Multidraw = jstools::parseJson(json, "Multidraw", js);
    if (Multidraw) MultidrawSettings.readFronJson(js);
}
