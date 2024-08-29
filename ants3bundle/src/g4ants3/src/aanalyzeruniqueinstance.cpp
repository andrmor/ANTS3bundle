#include "aanalyzeruniqueinstance.h"
#include "ahistogram.h"

#include "G4SystemOfUnits.hh"
#include "G4Step.hh"

AAnalyzerUniqueInstance::AAnalyzerUniqueInstance(const AParticleAnalyzerRecord & properties, int globalIndexIfNoMerge) :
    Properties(properties), GlobalIndexIfNoMerge(globalIndexIfNoMerge)
{
    if      (properties.EnergyUnits == "MeV") EnergyFactor = 1 / MeV;
    else if (properties.EnergyUnits == "keV") EnergyFactor = 1 / keV;
    else if (properties.EnergyUnits == "eV")  EnergyFactor = 1 / eV;
}

bool AAnalyzerUniqueInstance::processParticle(G4Step * step)
{
    G4StepPoint * preStepPoint = step->GetPreStepPoint();

    const double time = preStepPoint->GetGlobalTime() / ns;
    if ( !Properties.UseTimeWindow ||
        (time >= Properties.TimeWindowFrom && time <= Properties.TimeWindowTo) )
    {
        const double energy = preStepPoint->GetKineticEnergy() * EnergyFactor;

        std::string particleName = step->GetTrack()->GetParticleDefinition()->GetParticleName(); // usually the names are short, string optimization will kick in
        const std::size_t squareBracketPosition = particleName.find('[');
        if (squareBracketPosition == std::string::npos)
            ; // most common case it is not an excited ion
        else particleName = particleName.substr(0, squareBracketPosition);

        const auto it = ParticleMap.find(particleName);
        if (it != ParticleMap.end())
        {
            it->second.Number++;
            it->second.Energy->fill(energy);
        }
        else
        {
            AnalyzerParticleEntry rec;
            rec.Energy = new AHistogram1D(Properties.EnergyBins, Properties.EnergyFrom, Properties.EnergyTo);
            rec.Number = 1;
            rec.Energy->fill(energy);
            ParticleMap[particleName] = rec; // !!!*** optimize to avoid second lookup
        }
    }

    return Properties.StopTracking;
}

void AAnalyzerUniqueInstance::writeToJson(json11::Json::object & json) const
{
    //json["UniqueIndex"] = Properties.UniqueIndex;
    json["VolumeBaseName"] = (Properties.VolumeBaseName.empty() ? Properties.VolumeNames.front() : Properties.VolumeBaseName ); // it will be the base name for non-unique objects

    json11::Json::array arAllParticles;
    for (const auto & pair : ParticleMap)
    {
        json11::Json::object js;
        js["Particle"] = pair.first;

        AHistogram1D * Hist = pair.second.Energy;
        const std::vector<double> & data = Hist->getContent();
        double from, to;
        Hist->getLimits(from, to);
        js["EnergyFrom"] = from;
        js["EnergyTo"] = to;
        double delta = (to - from) / Properties.EnergyBins;
        json11::Json::array ar;
        for (size_t i = 0; i < data.size(); i++)  // 0=underflow and last=overflow
        {
            json11::Json::array el;
            el.push_back(from + (i - 0.5)*delta); // i - 1 + 0.5
            el.push_back(data[i]);
            ar.push_back(el);
        }
        js["EnergyData"] = ar;

        const std::vector<double> vec = Hist->getStat();
        json11::Json::array sjs;
        for (const double & d : vec)
            sjs.push_back(d);
        js["EnergyStats"] = sjs;

        arAllParticles.push_back(js);
    }
    json["ParticleData"] = arAllParticles;

    json["GlobalIndexIfNoMerge"] = GlobalIndexIfNoMerge;
}
