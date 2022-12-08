#include "aparticlesourcerecord.h"

#ifdef JSON11
// already in the header
#else
#include "ajsontools.h"
#include "afiletools.h"
#endif

bool AGunParticle::buildEnergyHistogram()
{
    if (EnergySpectrum.empty()) return false;
    return _EnergySampler.configure(EnergySpectrum, RangeBasedEnergies);
}

double AGunParticle::generateEnergy() const
{
    if (UseFixedEnergy) return Energy;
    return _EnergySampler.getRandom();
}

#ifndef JSON11
void AGunParticle::writeToJson(QJsonObject & json) const
{
    json["Particle"]   = QString(Particle.data());
    json["StatWeight"] = StatWeight;
    json["Individual"] = Individual;
    json["LinkedTo"] = LinkedTo;
    json["LinkingProbability"] = LinkedProb;
    json["LinkingOppositeDir"] = LinkedOpposite;
    json["Energy"] = Energy;
    json["UseFixedEnergy"] = UseFixedEnergy;
    json["RangeBasedEnergies"] = RangeBasedEnergies;

    QJsonArray ar;
        jstools::writeDPairVectorToArray(EnergySpectrum, ar);
    json["EnergySpectrum"] = ar;

    json["PreferredUnits"] = QString(PreferredUnits.data());
}
#endif

#ifdef JSON11
bool AGunParticle::readFromJson(const json11::Json::object & json)
#else
bool AGunParticle::readFromJson(const QJsonObject & json)
#endif
{
    jstools::parseJson(json, "Particle",           Particle);
    jstools::parseJson(json, "StatWeight",         StatWeight );
    jstools::parseJson(json, "Individual",         Individual );
    jstools::parseJson(json, "LinkedTo",           LinkedTo ); //linked always to previously already defined particles!
    jstools::parseJson(json, "LinkingProbability", LinkedProb );
    jstools::parseJson(json, "LinkingOppositeDir", LinkedOpposite );

    jstools::parseJson(json, "Energy",             Energy );
    jstools::parseJson(json, "PreferredUnits",     PreferredUnits);

    jstools::parseJson(json, "UseFixedEnergy",     UseFixedEnergy );
    jstools::parseJson(json, "RangeBasedEnergies", RangeBasedEnergies );

#ifdef JSON11
    json11::Json::array ar;
#else
    QJsonArray ar;
#endif
    jstools::parseJson(json, "EnergySpectrum", ar);
    jstools::readDPairVectorFromArray(ar, EnergySpectrum);

    bool ok = buildEnergyHistogram(); // !!!*** dublicated here -> it is checked by the check() method of the particle source
    if (!ok && !UseFixedEnergy) return false;//("Failed to build energy histogram for particle " + particle.Particle + " of source " + source.Name);

    return true;
}

// ---------------------- AParticleSourceRecord ----------------------

void AParticleSourceRecord::clear()
{
    Name  = "No_name";
    Shape = Point;

    X0    = 0;
    Y0    = 0;
    Z0    = 0;

    Phi   = 0;
    Theta = 0;
    Psi   = 0;

    Size1 = 10.0;
    Size2 = 10.0;
    Size3 = 10.0;

    CollPhi   = 0;
    CollTheta = 0;
    Spread    = 45.0;

    MaterialLimited = false;
    LimtedToMatName.clear();

    Activity = 1.0;

    TimeAverageMode   = 0;
    TimeAverage       = 0;
    TimeAverageStart  = 0;
    TimeAveragePeriod = 10.0;
    TimeSpreadMode    = 0;
    TimeSpreadSigma   = 50.0;
    TimeSpreadWidth   = 100.0;

    Particles.clear();
}

#ifndef JSON11
void AParticleSourceRecord::writeToJson(QJsonObject & json) const
{
    json["Name"] = QString(Name.data());

    QString str;
    switch (Shape)
    {
    case Point     : str = "point";     break;
    case Line      : str = "line";      break;
    case Rectangle : str = "rectangle"; break;
    case Round     : str = "round";     break;
    case Box       : str = "box";       break;
    case Cylinder  : str = "cylinder";  break;
    }
    json["Shape"] = str;

    json["Activity"] = Activity;

    json["X"] = X0;
    json["Y"] = Y0;
    json["Z"] = Z0;

    json["Size1"] = Size1;
    json["Size2"] = Size2;
    json["Size3"] = Size3;

    json["Phi"]   = Phi;
    json["Theta"] = Theta;
    json["Psi"]   = Psi;

    json["CollPhi"]   = CollPhi;
    json["CollTheta"] = CollTheta;
    json["Spread"]    = Spread;

    json["MaterialLimited"] = MaterialLimited;
    json["LimitedToMaterial"] = QString(LimtedToMatName.data());

    json["TimeAverageMode"]   = TimeAverageMode;
    json["TimeAverage"]       = TimeAverage;
    json["TimeAverageStart"]  = TimeAverageStart;
    json["TimeAveragePeriod"] = TimeAveragePeriod;
    json["TimeSpreadMode"]    = TimeSpreadMode;
    json["TimeSpreadSigma"]   = TimeSpreadSigma;
    json["TimeSpreadWidth"]   = TimeSpreadWidth;

    //particles
    //int GunParticleSize = Particles.size();
    //json["Particles"] = GunParticleSize;
    QJsonArray jParticleEntries;
    for (const AGunParticle & gp : Particles)
    {
        QJsonObject js;
            gp.writeToJson(js);
        jParticleEntries.append(js);
    }
    json["Particles"] = jParticleEntries;
}
#endif

#ifdef JSON11
bool AParticleSourceRecord::readFromJson(const json11::Json::object & json)
#else
bool AParticleSourceRecord::readFromJson(const QJsonObject & json)
#endif
{
    clear();

    jstools::parseJson(json, "Name", Name);

    std::string tmp;
    jstools::parseJson(json, "Shape", tmp);
    if      (tmp == "point")     Shape = Point;
    else if (tmp == "line")      Shape = Line;
    else if (tmp == "rectangle") Shape = Rectangle;
    else if (tmp == "round")     Shape = Round;
    else if (tmp == "box")       Shape = Box;
    else if (tmp == "cylinder")  Shape = Cylinder;

    jstools::parseJson(json, "Activity", Activity);

    jstools::parseJson(json, "X", X0);
    jstools::parseJson(json, "Y", Y0);
    jstools::parseJson(json, "Z", Z0);

    jstools::parseJson(json, "Size1", Size1);
    jstools::parseJson(json, "Size2", Size2);
    jstools::parseJson(json, "Size3", Size3);

    jstools::parseJson(json, "Phi",   Phi);
    jstools::parseJson(json, "Theta", Theta);
    jstools::parseJson(json, "Psi",   Psi);

    jstools::parseJson(json, "CollPhi",   CollPhi);
    jstools::parseJson(json, "CollTheta", CollTheta);
    jstools::parseJson(json, "Spread",    Spread);

    jstools::parseJson(json, "TimeAverageMode",   TimeAverageMode);
    jstools::parseJson(json, "TimeAverage",       TimeAverage);
    jstools::parseJson(json, "TimeAverageStart",  TimeAverageStart);
    jstools::parseJson(json, "TimeAveragePeriod", TimeAveragePeriod);
    jstools::parseJson(json, "TimeSpreadMode",    TimeSpreadMode);
    jstools::parseJson(json, "TimeSpreadSigma",   TimeSpreadSigma);
    jstools::parseJson(json, "TimeSpreadWidth",   TimeSpreadWidth);

    jstools::parseJson(json, "MaterialLimited", MaterialLimited);
    jstools::parseJson(json, "LimitedToMaterial", LimtedToMatName);

#ifdef JSON11
    json11::Json::array jGunPartArr;
#else
    QJsonArray jGunPartArr;
#endif
    jstools::parseJson(json, "Particles", jGunPartArr);
    const int numGP = jGunPartArr.size();
    for (int ip = 0; ip < numGP; ip++)
    {

#ifdef JSON11
        json11::Json::object jThisGunPart = jGunPartArr[ip].object_items();
#else
        QJsonObject jThisGunPart = jGunPartArr[ip].toObject();
#endif

        AGunParticle gp;
        bool bOK = gp.readFromJson(jThisGunPart);
        if (!bOK) return false;
        Particles.push_back(gp);
    }
    return true;
}

std::string AParticleSourceRecord::getShapeString() const
{
    switch (Shape)
    {
    case Point     : return "Point";
    case Line      : return "Line";
    case Rectangle : return "Rectangle";
    case Round     : return "Round";
    case Box       : return "Box";
    case Cylinder  : return "Cylinder";
    }
    return "UnknownShape";
}

std::string AParticleSourceRecord::check() const
{
    const int numParts = Particles.size();
    if (numParts == 0) return "No particles defined";

    if (Spread < 0)    return "negative spread angle";
    if (Activity < 0)  return "negative activity";

    int    numIndParts   = 0;
    double TotPartWeight = 0;
    for (int ip = 0; ip < numParts; ip++)
    {
        const AGunParticle & gp = Particles.at(ip);
        if (gp.Individual)
        {
            numIndParts++;
            if (Particles.at(ip).StatWeight < 0) return "Negative statistical weight for particle #" + std::to_string(ip);
            TotPartWeight += Particles.at(ip).StatWeight;
        }
        else
        {
            if (ip == gp.LinkedTo) return "Particle #" + std::to_string(ip) + " is linked to itself";
            if (ip <  gp.LinkedTo) return "Invalid linking for particle #" + std::to_string(ip);
        }

        if (gp.LinkedOpposite)
        {
            if (ip == 0) return "The first particle cannot be set as \"opposite\"!"; // not needed?
            if (Particles[gp.LinkedTo].Particle == "-") return "Particle (#" + std::to_string(ip) + ") cannot be set \"opposite\" to one representing direct deposition (\"-\")";
        }

        if (gp.Energy <= 0) return "Energy <= 0 for particle #" + std::to_string(ip);

        if (!gp.UseFixedEnergy)
        {
            if (gp.EnergySpectrum.size() < 2) return "Energy spectrum should have at least 2 points";
            if (!gp._EnergySampler.isReady()) return "Energy sampler is not ready: Check energy spectrum"; // !!!*** make full error check
        }
    }

    if (numIndParts   == 0) return "No individual particles defined";
    if (TotPartWeight == 0) return "Total statistical weight of individual particles is zero";

    return "";
}
