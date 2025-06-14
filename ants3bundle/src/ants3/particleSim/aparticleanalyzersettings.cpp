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
    json["AnalyzeCreated"] = AnalyzeCreated;

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
        //json["TypeIndex"] = TypeIndex;
        //json["UniqueIndex"] = UniqueIndex;
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
    AnalyzeCreated = false;
    jstools::parseJson(json, "AnalyzeCreated", AnalyzeCreated);

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
    //jstools::parseJson(json, "UniqueIndex", UniqueIndex);
    jstools::parseJson(json, "VolumeBaseName", VolumeBaseName);
    json11::Json::array ar;
    jstools::parseJson(json, "VolumeNames", ar);
    VolumeNames.resize(ar.size());
    for (size_t i = 0; i < ar.size(); i++)
        VolumeNames[i] = ar[i].string_value();
#endif
}

void AParticleAnalyzerRecord::addVolumeNameIfNew(const std::string & name)
{
    for (const auto & n : VolumeNames)
        if (n == name) return;

    VolumeNames.push_back(name);
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
        for (const AParticleAnalyzerRecord & ana : AnalyzerTypes)
        {
            QJsonObject js;
            ana.writeToJson(js, includeG4ants3Set);
            ar.append(js);
        }
        json["AnalyzerTypes"] = ar;

        // UniqueToTypeLUT
        {
            QJsonArray arLUT;
            for (size_t index : UniqueToTypeLUT)
                arLUT.append( (int)index );
            json["UniqueToTypeLUT"] = arLUT;
        }

        // GlobalToUniqueLUT
        {
            QJsonArray arLUT;
            for (size_t index : GlobalToUniqueLUT)
                arLUT.append( (int)index );
            json["GlobalToUniqueLUT"] = arLUT;
        }

        json["NumberOfUniqueAnalyzers"] = (int)NumberOfUniqueAnalyzers;
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

    AnalyzerTypes.clear();
    GlobalToUniqueLUT.clear();

#ifdef JSON11
    json11::Json::array anaArray;
    jstools::parseJson(json, "AnalyzerTypes", anaArray);
    for (size_t i = 0; i < anaArray.size(); i++)
    {
        json11::Json::object mjs = anaArray[i].object_items();

        AParticleAnalyzerRecord r;
        r.readFromJson(mjs);
        AnalyzerTypes.push_back(r);
    }

    // UniqueToTypeLUT
    {
        json11::Json::array lutArray;
        jstools::parseJson(json, "UniqueToTypeLUT", lutArray);
        for (size_t i = 0; i < lutArray.size(); i++)
        {
            int index = lutArray[i].int_value();
            UniqueToTypeLUT.push_back(index);
        }
    }

    // GlobalToUniqueLUT
    {
        json11::Json::array lutArray;
        jstools::parseJson(json, "GlobalToUniqueLUT", lutArray);
        for (size_t i = 0; i < lutArray.size(); i++)
        {
            int index = lutArray[i].int_value();
            GlobalToUniqueLUT.push_back(index);
        }
    }

    int num;
    jstools::parseJson(json, "NumberOfUniqueAnalyzers", num);
    if (num < 0) num = 0; // !!!*** error reporting
    NumberOfUniqueAnalyzers = num;

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

    AnalyzerTypes.clear();
    GlobalToUniqueLUT.clear();
    UniqueToTypeLUT.clear();

    NumberOfUniqueAnalyzers = 0;
}
