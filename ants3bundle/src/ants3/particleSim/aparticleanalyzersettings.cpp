#include "aparticleanalyzersettings.h"

#ifdef JSON11
#else
#include "ajsontools.h"
#include "aerrorhub.h"
#endif

AParticleAnalyzerSettings::AParticleAnalyzerSettings() {}

bool AParticleAnalyzerRecord::isAllowedEnergyUnit(const std::string & str)
{
    if (str == "MeV") return true;
    if (str == "keV") return true;
    if (str == "eV")  return true;
    return false;
}

#ifndef JSON11
void AParticleAnalyzerRecord::writeToJson(QJsonObject & json) const
{
    json["EergyBins"]   = EnergyBins;
    json["EnergyFrom"]  = EnergyFrom;
    json["EnergyTo"]    = EnergyTo;
    json["EnergyUnits"] = QString(EnergyUnits.data());

    json["UseTimeWindow"]  = UseTimeWindow;
    json["TimeWindowFrom"] = TimeWindowFrom;
    json["TimeWindowTo"]   = TimeWindowTo;

    json["StopTracking"] = StopTracking;

    json["SingleInstanceForAllCopies"] = SingleInstanceForAllCopies;
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
}
