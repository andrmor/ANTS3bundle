#include "asourcegeneratorsettings.h"
#include "aerrorhub.h"

#ifndef JSON11
#include "ajsontools.h"
#endif

void ASourceGeneratorSettings::clear()
{
    for (AParticleSourceRecord * rec : SourceData) delete rec;
    SourceData.clear();

    MultiEnabled   = false;
    MultiNumber    = 1;
    MultiMode      = Constant;
}

double ASourceGeneratorSettings::calculateTotalActivity() const
{
    double TotalActivity = 0;
    for (const AParticleSourceRecord * r : SourceData)
        TotalActivity += r->Activity;
    return TotalActivity;
}

bool ASourceGeneratorSettings::check() const
{
    std::string Error;
    for (const AParticleSourceRecord * ps : SourceData)
    {
        const std::string err = ps->check();
        if (!err.empty()) Error += "Source " + ps->Name + ": " + err + "\n";
    }

    if (Error.empty()) return true;

    AErrorHub::addError(Error.data());
    return false;
}

bool ASourceGeneratorSettings::clone(int iSource)
{
#ifdef JSON11
    return false;
#else
    if (iSource < 0 || iSource >= (int)SourceData.size()) return false;

    AParticleSourceRecord * r = SourceData[iSource];
    r->Name += "_c";

    QJsonObject json;
    r->writeToJson(json);

    AParticleSourceRecord * clone = new AParticleSourceRecord();
    clone->readFromJson(json);

    SourceData.insert(SourceData.begin() + iSource + 1, clone);
    return true;
#endif
}

bool ASourceGeneratorSettings::replace(int iSource, AParticleSourceRecord * source)
{
    if (iSource < 0 || iSource >= (int)SourceData.size()) return false;

    delete SourceData[iSource];
    SourceData[iSource] = source;
    return true;
}

void ASourceGeneratorSettings::remove(int iSource)
{
    if (SourceData.empty()) return;
    if (iSource < 0 || iSource >= (int)SourceData.size()) return;

    delete SourceData[iSource];
    SourceData.erase(SourceData.begin() + iSource);
}

#ifndef JSON11
void ASourceGeneratorSettings::writeToJson(QJsonObject &json) const
{
    QJsonArray ja;
    for (const AParticleSourceRecord * ps : SourceData)
    {
        QJsonObject js;
            ps->writeToJson(js);
        ja.append(js);
    }
    json["ParticleSources"] = ja;

    QJsonObject js;
    js["Enabled"] = MultiEnabled;
    js["Mode"]    = ( MultiMode == Constant ? "Constant" : "Poisson" );
    js["Number"]  = MultiNumber;
    json["MultiplePerEvent"] = js;
}
#endif

#ifdef JSON11
bool ASourceGeneratorSettings::readFromJson(const json11::Json::object &json)
#else
bool ASourceGeneratorSettings::readFromJson(const QJsonObject & json)
#endif
{
    clear();

#ifdef JSON11
    json11::Json::array ar;
#else
    QJsonArray ar;
#endif
    jstools::parseJson(json, "ParticleSources", ar);

    for (int iSource = 0; iSource < (int)ar.size(); iSource++)
    {
#ifdef JSON11
        json11::Json::object js = ar[iSource].object_items();
#else
        QJsonObject js = ar[iSource].toObject();
#endif
        AParticleSourceRecord * ps = new AParticleSourceRecord(); // !!!*** other types
        bool ok = ps->readFromJson(js);
        if (ok)
        {
            SourceData.push_back(ps);
        }
        else
        {
            // Load particle source # << iSource << from json failed!
            return false;
        }
    }

#ifdef JSON11
    json11::Json::object js;
#else
    QJsonObject js;
#endif
    jstools::parseJson(json, "MultiplePerEvent", js);
    {
        jstools::parseJson(js, "Enabled", MultiEnabled);
        jstools::parseJson(js, "Number",  MultiNumber);

        std::string strMulti;
        jstools::parseJson(js, "Mode",    strMulti);
        if (strMulti == "Poisson")
            MultiMode = Poisson;
        else
            MultiMode = Constant;
    }

    return true;
}
