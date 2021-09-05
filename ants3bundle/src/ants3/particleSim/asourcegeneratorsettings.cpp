#include "asourcegeneratorsettings.h"

#include "ajsontools.h"

void ASourceGeneratorSettings::clear()
{
    SourceData.clear();

    MultiEnabled   = false;
    MultiNumber    = 1;
    MultiMode      = Constant;
}

double ASourceGeneratorSettings::calculateTotalActivity() const
{
    double TotalActivity = 0;
    for (const AParticleSourceRecord & r : SourceData)
        TotalActivity += r.Activity;
    return TotalActivity;
}

std::string ASourceGeneratorSettings::check() const
{
    std::string Error;
    for (const AParticleSourceRecord & ps : SourceData)
    {
        const std::string err = ps.check();
        if (!err.empty()) Error += "Source " + ps.Name + ": " + err + "\n";
    }
    return Error;
}

bool ASourceGeneratorSettings::clone(int iSource)
{
    if (iSource < 0 || iSource >= (int)SourceData.size()) return false;

    AParticleSourceRecord r = SourceData[iSource];
    r.Name += "_c";
    SourceData.insert(SourceData.begin() + iSource + 1, r);
    return true;
}

bool ASourceGeneratorSettings::replace(int iSource, AParticleSourceRecord & source)
{
    if (iSource < 0 || iSource >= (int)SourceData.size()) return false;

    SourceData[iSource] = source;
    return true;
}

void ASourceGeneratorSettings::remove(int iSource)
{
    if (SourceData.empty()) return;
    if (iSource < 0 || iSource >= (int)SourceData.size()) return;

    SourceData.erase(SourceData.begin() + iSource);
}

void ASourceGeneratorSettings::writeToJson(QJsonObject &json) const
{
    QJsonArray ja;
    for (const AParticleSourceRecord & ps : SourceData)
    {
        QJsonObject js;
        ps.writeToJson(js);
        ja.append(js);
    }
    json["ParticleSources"] = ja;

    QJsonObject js;
    js["Enabled"] = MultiEnabled;
    js["Mode"]    = ( MultiMode == Constant ? "Constant" : "Poisson" );
    js["Number"]  = MultiNumber;
    json["MultiplePerEvent"] = js;
}

bool ASourceGeneratorSettings::readFromJson(const QJsonObject & json)
{
    clear();

    if (!json.contains("ParticleSources"))
    {
        // error?
        return false;
    }

    QJsonArray ar = json["ParticleSources"].toArray();
    for (int iSource = 0; iSource < ar.size(); iSource++)
    {
        QJsonObject js = ar.at(iSource).toObject();
        AParticleSourceRecord ps;
        bool ok = ps.readFromJson(js);
        if (ok) SourceData.push_back(ps);
        else
        {
            // Load particle source # << iSource << from json failed!
            return false;
        }
    }

    QJsonObject js;
    jstools::parseJson(json, "MultiplePerEvent", js);
    {
        jstools::parseJson(js, "Enabled", MultiEnabled);
        jstools::parseJson(js, "Number",  MultiNumber);

        QString strMulti;
        jstools::parseJson(js, "Mode",    strMulti);
        if (strMulti == "Poisson")
            MultiMode = Poisson;
        else
            MultiMode = Constant;
    }

    return true;
}
