#include "aisotope.h"
#include "ajsontools.h"

#include "TString.h"

const TString AIsotope::getTName() const
{
    TString name(Symbol.toLatin1().data());
    name += Mass;
    return name;
}

void AIsotope::writeToJson(QJsonObject &json) const
{
    json["Symbol"]    = Symbol;
    json["Mass"]      = Mass;
    json["Abundancy"] = Abundancy;
}

void AIsotope::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "Symbol",    Symbol);
    jstools::parseJson(json, "Mass",      Mass);
    jstools::parseJson(json, "Abundancy", Abundancy);
}
