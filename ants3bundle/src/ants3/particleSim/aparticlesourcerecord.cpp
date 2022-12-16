#include "aparticlesourcerecord.h"

#ifdef JSON11
// already in the header
#else
#include "ajsontools.h"
#include "afiletools.h"
#endif

using JsonArray =
#ifdef JSON11
    json11::Json::array;
#else
    QJsonArray;
#endif

using JsonObject =
#ifdef JSON11
    json11::Json::object;
#else
    QJsonObject;
#endif

bool AGunParticle::configureEnergySampler()
{
    if (EnergySpectrum.empty())
    {
        _EnergySampler.clear();
        return false;
    }
    return _EnergySampler.configure(EnergySpectrum, RangeBasedEnergies);
}

double AGunParticle::generateEnergy() const
{
    if (UseFixedEnergy) return FixedEnergy;
    return _EnergySampler.getRandom();
}

bool AGunParticle::isDirectDeposition() const
{
#ifdef GEANT4
    return (particleDefinition == nullptr);
#else
    return (Particle == "-");
#endif
}

#ifndef JSON11
void AGunParticle::writeToJson(QJsonObject & json) const
{
    json["Particle"] = QString(Particle.data());

    // Generation properties
    {
        JsonObject js;

            QString str;
                switch (GenerationType)
                {
                case Independent           : str = "Independent";           break;
                case Linked_IfGenerated    : str = "Linked_IfGenerated";    break;
                case Linked_IfNotGenerated : str = "Linked_IfNotGenerated"; break;
                default: qCritical() << "Not implemented AGunParticle type in writeToJson!"; exit(111);
                }
            js["Type"] = str;
            js["BtBPair"]            = BtBPair;
            js["StatWeight"]         = StatWeight;
            js["LinkedTo"]           = LinkedTo;
            js["LinkingProbability"] = LinkedProb;

        json["Generation"] = js;
    }

    // Energy properties
    {
        JsonObject js;

            js["UseFixedEnergy"]     = UseFixedEnergy;
            js["FixedEnergy"]        = FixedEnergy;
            js["PreferredUnits"] = QString(PreferredUnits.data());
            QJsonArray ar;
                jstools::writeDPairVectorToArray(EnergySpectrum, ar);
            js["EnergySpectrum"] = ar;
            js["RangeBasedEnergies"] = RangeBasedEnergies;

        json["Energy"] = js;
    }
}
#endif

bool AGunParticle::readFromJson(const JsonObject & json)
{
    jstools::parseJson(json, "Particle", Particle);

    // Generation properties
    {
        JsonObject js;
        jstools::parseJson(json, "Generation", js);

            std::string str;
            jstools::parseJson(js, "Type", str);
            if      (str == "Independent")           GenerationType = EType::Independent;
            else if (str == "Linked_IfGenerated")    GenerationType = EType::Linked_IfGenerated;
            else if (str == "Linked_IfNotGenerated") GenerationType = EType::Linked_IfNotGenerated;
            else
            {
                // !!!*** generate error
                // "Unknown AGunParticle type in readFromJson, setting to EType::Independent";
                GenerationType = EType::Independent;
            }
            jstools::parseJson(js, "BtBPair",            BtBPair);
            jstools::parseJson(js, "StatWeight",         StatWeight);
            jstools::parseJson(js, "LinkedTo",           LinkedTo);
            jstools::parseJson(js, "LinkingProbability", LinkedProb);
    }

    // Energy properties
    {
        JsonObject js;
        jstools::parseJson(json, "Energy", js);

            jstools::parseJson(js, "FixedEnergy",        FixedEnergy );
            jstools::parseJson(js, "PreferredUnits",     PreferredUnits);
            jstools::parseJson(js, "UseFixedEnergy",     UseFixedEnergy );
            jstools::parseJson(js, "RangeBasedEnergies", RangeBasedEnergies );

            JsonArray ar;
                jstools::parseJson(js, "EnergySpectrum", ar);
            jstools::readDPairVectorFromArray(ar, EnergySpectrum);

            configureEnergySampler();
    }

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

    AngularMode = UniformAngular;
    DirectionPhi   = 0;
    DirectionTheta = 0;
    UseCutOff = false;
    CutOff    = 45.0;
    DispersionSigma = 5.0;
    AngularDistribution.clear();

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

    // Angular properties
    {
        QJsonObject js;
            QString str;
            switch (AngularMode)
            {
            case UniformAngular  : str = "Uniform"; break;
            case FixedDirection  : str = "Fixed";   break;
            case GaussDispersion : str = "Gauss";   break;
            case CustomAngular   : str = "Custom";  break;
            }
            js["Mode"]  = str;
            js["Phi"]   = DirectionPhi;
            js["Theta"] = DirectionTheta;
            js["UseCutOff"] = UseCutOff;
            js["CutOff"] = CutOff;
            js["Sigma"] = DispersionSigma;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(AngularDistribution, ar);
            js["CustomDistribution"] = ar;
        json["Angular"] = js;
    }

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

bool AParticleSourceRecord::readFromJson(const JsonObject & json)
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
    // !!!*** error if not found

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

    // Angular properties
    {
        JsonObject js;
        jstools::parseJson(json, "Angular", js);

            std::string str;
            jstools::parseJson(js, "Mode", str);
            if      (str == "Uniform") AngularMode = UniformAngular;
            else if (str == "Fixed")   AngularMode = FixedDirection;
            else if (str == "Gauss")   AngularMode = GaussDispersion;
            else if (str == "Custom")  AngularMode = CustomAngular;
            // !!!*** error if not found

            jstools::parseJson(js, "Phi",       DirectionPhi);
            jstools::parseJson(js, "Theta",     DirectionTheta);
            jstools::parseJson(js, "UseCutOff", UseCutOff);
            jstools::parseJson(js, "CutOff",    CutOff);
            jstools::parseJson(js, "Sigma",     DispersionSigma);
            JsonArray ar;
                jstools::parseJson(js, "CustomDistribution", ar);
            jstools::readDPairVectorFromArray(ar, AngularDistribution);
            configureAngularSampler();
    }

    jstools::parseJson(json, "TimeAverageMode",   TimeAverageMode);
    jstools::parseJson(json, "TimeAverage",       TimeAverage);
    jstools::parseJson(json, "TimeAverageStart",  TimeAverageStart);
    jstools::parseJson(json, "TimeAveragePeriod", TimeAveragePeriod);
    jstools::parseJson(json, "TimeSpreadMode",    TimeSpreadMode);
    jstools::parseJson(json, "TimeSpreadSigma",   TimeSpreadSigma);
    jstools::parseJson(json, "TimeSpreadWidth",   TimeSpreadWidth);

    jstools::parseJson(json, "MaterialLimited", MaterialLimited);
    jstools::parseJson(json, "LimitedToMaterial", LimtedToMatName);

    JsonArray jGunPartArr;
        jstools::parseJson(json, "Particles", jGunPartArr);
    const int numGP = jGunPartArr.size();
    for (int ip = 0; ip < numGP; ip++)
    {
/*
#ifdef JSON11
        json11::Json::object jThisGunPart = jGunPartArr[ip].object_items();
#else
        QJsonObject jThisGunPart = jGunPartArr[ip].toObject();
#endif
*/
        JsonObject js;
        jstools::arrayElementToObject(jGunPartArr, ip, js);

        AGunParticle gp;
        bool bOK = gp.readFromJson(js);
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

    if (Activity < 0)  return "Negative activity";

    switch (AngularMode)
    {
    case UniformAngular : break;
    case FixedDirection : break;
    case GaussDispersion :
        if (DispersionSigma < 0) return "Dispersion sigma cannot be negative";
        break;
    case CustomAngular :
        if (AngularDistribution.size() < 2) return "Custom angular distribution data should contain at least two data points";
        // !!!*** other checks
        if (!_AngularSampler.isReady()) return "Angular sampler is not ready: Check angular distribution";
        break;
    }

    if (UseCutOff && CutOff < 0) return "Negative cutt-off angle";

    int    numIndParts   = 0;
    double totPartWeight = 0;
    for (int ip = 0; ip < numParts; ip++)
    {
        const AGunParticle & gp = Particles.at(ip);
        if (gp.GenerationType == AGunParticle::Independent)
        {
            numIndParts++;
            if (Particles.at(ip).StatWeight < 0) return "Negative statistical weight for particle #" + std::to_string(ip);
            totPartWeight += Particles.at(ip).StatWeight;
        }
        else
        {
            if (ip == gp.LinkedTo) return "Particle #" + std::to_string(ip) + " is linked to itself";
            if (ip <  gp.LinkedTo) return "Invalid linking for particle #" + std::to_string(ip);
        }

        if (gp.BtBPair)
        {
            // !!!*** tweak error message
            if (Particles[gp.LinkedTo].Particle == "-") return "Particle (#" + std::to_string(ip) + ") cannot be set \"LinkedBtBPair\" to one representing direct deposition (\"-\")";
        }

        if (gp.FixedEnergy <= 0) return "Energy <= 0 for particle #" + std::to_string(ip);

        if (!gp.UseFixedEnergy)
        {
            if (gp.EnergySpectrum.size() < 2) return "Energy spectrum should have at least 2 points";
            // !!!*** make full error check
            if (!gp._EnergySampler.isReady()) return "Energy sampler is not ready: Check energy spectrum";
        }
    }

    if (numIndParts   == 0) return "No individual particles defined";
    if (totPartWeight == 0) return "Total statistical weight of individual particles is zero";

    return "";
}

bool AParticleSourceRecord::configureAngularSampler()
{
    if (AngularDistribution.empty())
    {
        _AngularSampler.clear();
        return false;
    }
    return _AngularSampler.configure(AngularDistribution, false);
}
