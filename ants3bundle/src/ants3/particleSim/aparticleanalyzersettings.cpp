#include "aparticleanalyzersettings.h"

#ifdef JSON11
#else
#include "ajsontools.h"
#include "aerrorhub.h"
#endif

AParticleAnalyzerSettings::AParticleAnalyzerSettings()
{
    clear();
}

bool AParticleAnalyzerRecord::isAllowedEnergyUnit(const std::string & str)
{
    if (str == "MeV") return true;
    if (str == "keV") return true;
    if (str == "eV")  return true;
    return false;
}

#ifndef JSON11
void AParticleAnalyzerRecord::writeToJson(QJsonObject & json, bool includeGeant4Features) const
{
    json["EnergyBins"]  = EnergyBins;
    json["EnergyFrom"]  = EnergyFrom;
    json["EnergyTo"]    = EnergyTo;
    json["EnergyUnits"] = QString(EnergyUnits.data());

    json["UseTimeWindow"]  = UseTimeWindow;
    json["TimeWindowFrom"] = TimeWindowFrom;
    json["TimeWindowTo"]   = TimeWindowTo;

    json["StopTracking"] = StopTracking;

    json["SingleInstanceForAllCopies"] = SingleInstanceForAllCopies;

    if (includeGeant4Features)
    {
        json["UniqueIndex"] = UniqueIndex;
        json["VolumeBaseName"] = QString(VolumeBaseName.data());

        QJsonArray ar;
        for (const auto & n : VolumeNames) ar.push_back( QString(n.data()) );
        json["VolumeNames"] = ar;
    }
}
#endif

#ifdef JSON11
void AParticleAnalyzerRecord::readFromJson(const json11::Json::object & json)
#else
void AParticleAnalyzerRecord::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "EnergyBins", EnergyBins);
    jstools::parseJson(json, "EnergyFrom", EnergyFrom);
    jstools::parseJson(json, "EnergyTo", EnergyTo);
    jstools::parseJson(json, "EnergyUnits", EnergyUnits);
    if (!isAllowedEnergyUnit(EnergyUnits))
    {
#ifndef JSON11
        AErrorHub::addError("Unknown energy unit in AGeoParticleAnalyzer::readFromJson (" + EnergyUnits + "), setting to keV");
#endif
        EnergyUnits = "keV";
    }

    jstools::parseJson(json, "UseTimeWindow", UseTimeWindow);
    jstools::parseJson(json, "TimeWindowFrom", TimeWindowFrom);
    jstools::parseJson(json, "TimeWindowTo", TimeWindowTo);

    jstools::parseJson(json, "StopTracking", StopTracking);

    jstools::parseJson(json, "SingleInstanceForAllCopies", SingleInstanceForAllCopies);

#ifdef JSON11
    jstools::parseJson(json, "UniqueIndex", UniqueIndex);
    jstools::parseJson(json, "VolumeBaseName", VolumeBaseName);
    json11::Json::array ar;
    jstools::parseJson(json, "VolumeNames", ar);
    VolumeNames.resize(ar.size());
    for (size_t i = 0; i < ar.size(); i++)
        VolumeNames[i] = ar[i].string_value();
#endif
}

// -------------

#ifndef JSON11
void AParticleAnalyzerSettings::writeToJson(QJsonObject & json, bool includeG4ants3Set) const
{
    json["Enabled"] = Enabled;
    json["FileName"] = FileName.data();

    if (includeG4ants3Set)
    {
        QJsonArray ar;
        for (const AParticleAnalyzerRecord & ana : Analyzers)
        {
            QJsonObject js;
            ana.writeToJson(js, includeG4ants3Set);
            ar.append(js);
        }
        json["Analyzers"] = ar;
    }
}
#endif

#ifdef JSON11
void AParticleAnalyzerSettings::readFromJson(const json11::Json::object & json)
#else
void AParticleAnalyzerSettings::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "Enabled",  Enabled);
    jstools::parseJson(json, "FileName", FileName);

    Analyzers.clear();
#ifdef JSON11
    json11::Json::array anaArray;
    jstools::parseJson(json, "Analyzers", anaArray);
    for (size_t i = 0; i < anaArray.size(); i++)
    {
        json11::Json::object mjs = anaArray[i].object_items();

        AParticleAnalyzerRecord r;
        r.readFromJson(mjs);
        Analyzers.push_back(r);
    }
#endif
    // no need to read configs on ANTS3 side
}

#ifndef JSON11
#include "ageometryhub.h"
void AParticleAnalyzerSettings::initFromHub()
{
    AGeometryHub::getConstInstance().fillParticleAnalyzerRecords(this);
}
#endif

void AParticleAnalyzerSettings::clear()
{
    Enabled  = false;
    FileName = "ParticleAnalyzers.json";

    Analyzers.clear();
}
