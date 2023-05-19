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

std::string AGunParticle::configureEnergySampler()
{
    _EnergySampler.clear();
    if (UseFixedEnergy) return "";
    return _EnergySampler.configure(EnergySpectrum, false);
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
            QString str;
            switch (PreferredUnits)
            {
            case meV : str = "meV"; break;
            case eV  : str = "eV";  break;
            case keV : str = "keV"; break;
            case MeV : str = "MeV"; break;
            default: qCritical() << "Not implemented PreferredUnits"; exit(111);
            }
            js["PreferredUnits"] = str;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(EnergySpectrum, ar);
            js["EnergySpectrum"] = ar;

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

            jstools::parseJson(js, "FixedEnergy", FixedEnergy);
            std::string str;
            jstools::parseJson(js, "PreferredUnits", str);
            if      (str == "meV") PreferredUnits = meV;
            else if (str == "eV")  PreferredUnits = eV;
            else if (str == "keV") PreferredUnits = keV;
            else if (str == "MeV") PreferredUnits = MeV;
            else ;// !!!*** error handling
            jstools::parseJson(js, "UseFixedEnergy",     UseFixedEnergy);

            JsonArray ar;
                jstools::parseJson(js, "EnergySpectrum", ar);
            jstools::readDPairVectorFromArray(ar, EnergySpectrum);

            configureEnergySampler(); // !!!*** error handling
    }

    return true;
}

// ---------------------- AParticleSourceRecord ----------------------

void AParticleSourceRecord::clear()
{
    Name     = "No_name";
    Activity = 1.0;
    Shape    = Point;

    X0    = 0;
    Y0    = 0;
    Z0    = 0;

    Phi   = 0;
    Theta = 0;
    Psi   = 0;

    Size1 = 10.0;
    Size2 = 10.0;
    Size3 = 10.0;

    MaterialLimited = false;
    LimtedToMatName = "";

    AngularMode     = Isotropic;
    DirectionPhi    = 0;
    DirectionTheta  = 0;
    UseCutOff       = false;
    CutOff          = 45.0;
    DispersionSigma = 5.0;
    AngularDistribution.clear();

    TimeOffsetMode    = FixedOffset;
    TimeFixedOffset       = 0;
    TimeByEventStart  = 0;
    TimeByEventPeriod = 10.0;
    TimeSpreadMode    = NoSpread;
    TimeSpreadSigma   = 50.0;
    TimeSpreadWidth   = 100.0;
    TimeDistribution.clear();

    Particles.clear();
}

#ifndef JSON11
void AParticleSourceRecord::writeToJson(QJsonObject & json) const
{
    json["Name"]     = QString(Name.data());
    json["Activity"] = Activity;

    // Spatial properties
    {
        QJsonObject js;
            QString str;
                switch (Shape)
                {
                case Point     : str = "Point";     break;
                case Line      : str = "Line";      break;
                case Rectangle : str = "Rectangle"; break;
                case Round     : str = "Round";     break;
                case Box       : str = "Box";       break;
                case Cylinder  : str = "Cylinder";  break;
                }
            js["Shape"] = str;
            js["Position"] = QJsonArray{X0, Y0, Z0};
            js["Size1"] = Size1;
            js["Size2"] = Size2;
            js["Size3"] = Size3;
            js["Orientation"] = QJsonArray{Phi, Theta, Psi};
            QJsonObject mjs;
                mjs["Enabled"] = MaterialLimited;
                mjs["Material"] = QString(LimtedToMatName.data());
            js["MaterialLimited"] = mjs;
        json["Spatial"] = js;
    }

    // Angular properties
    {
        QJsonObject js;
            QString str;
            switch (AngularMode)
            {
            case Isotropic       : str = "Isotropic"; break;
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

    // Time properties
    {
        QJsonObject js;
            QString str;
                switch (TimeOffsetMode)
                {
                case FixedOffset              : str = "Fixed";        break;
                case ByEventIndexOffset       : str = "ByEventIndex"; break;
                case CustomDistributionOffset : str = "Custom";       break;
                default : qCritical() << "Not implemented TimeOffsetMode in AParticleSourceRecord::writeToJson"; exit(111);
                }
            js["OffsetMode"]    = str;
            js["FixedOffset"]   = TimeFixedOffset;
            js["ByEventStart"]  = TimeByEventStart;
            js["ByEventPeriod"] = TimeByEventPeriod;
                switch (TimeSpreadMode)
                {
                case NoSpread          : str = "None";        break;
                case GaussianSpread    : str = "Gaussian";    break;
                case UniformSpread     : str = "Uniform";     break;
                case ExponentialSpread : str = "Exponential"; break;
                default : qCritical() << "Not implemented TimeSpreadMode in AParticleSourceRecord::writeToJson"; exit(111);
                }
            js["SpreadMode"]     = str;
            js["SpreadSigma"]    = TimeSpreadSigma;
            js["SpreadWidth"]    = TimeSpreadWidth;
            js["SpreadHalfLife"] = TimeSpreadHalfLife;
                switch (TimeHalfLifePrefUnit)
                {
                case ns  : str = "ns";  break;
                case us  : str = "us";  break;
                case ms  : str = "ms";  break;
                case s   : str = "s";   break;
                case min : str = "min"; break;
                case h   : str = "h";   break;
                }
            js["HalfLifePreferUnit"] = str;
            QJsonArray ar;
                jstools::writeDPairVectorToArray(TimeDistribution, ar);
            js["CustomDistribution"] = ar;
        json["Time"] = js;
    }

    // Particles
    {
        QJsonArray ar;
            for (const AGunParticle & gp : Particles)
            {
                QJsonObject js;
                    gp.writeToJson(js);
                ar.append(js);
            }
        json["Particles"] = ar;
    }
}
#endif

bool AParticleSourceRecord::readFromJson(const JsonObject & json)
{
    clear();

    jstools::parseJson(json, "Name",     Name);
    jstools::parseJson(json, "Activity", Activity);

    // Spatial properties
    {
        JsonObject js;
        jstools::parseJson(json, "Spatial", js);

            std::string str;
            jstools::parseJson(js, "Shape", str);
            if      (str == "Point")     Shape = Point;
            else if (str == "Line")      Shape = Line;
            else if (str == "Rectangle") Shape = Rectangle;
            else if (str == "Round")     Shape = Round;
            else if (str == "Box")       Shape = Box;
            else if (str == "Cylinder")  Shape = Cylinder;
            else ; // !!!*** error

            JsonArray pjs;
            jstools::parseJson(js, "Position", pjs);
                if (pjs.size() == 3)
                {
#ifdef JSON11
                    X0 = pjs[0].number_value();
                    Y0 = pjs[1].number_value();
                    Z0 = pjs[2].number_value();
#else
                    X0 = pjs[0].toDouble();
                    Y0 = pjs[1].toDouble();
                    Z0 = pjs[2].toDouble();
#endif
                }
                else ; // !!!*** error

            jstools::parseJson(js, "Size1", Size1);
            jstools::parseJson(js, "Size2", Size2);
            jstools::parseJson(js, "Size3", Size3);

            JsonArray ojs;
            jstools::parseJson(js, "Orientation", ojs);
                if (ojs.size() == 3)
                {
#ifdef JSON11
                    Phi   = ojs[0].number_value();
                    Theta = ojs[1].number_value();
                    Psi   = ojs[2].number_value();
#else
                    Phi   = ojs[0].toDouble();
                    Theta = ojs[1].toDouble();
                    Psi   = ojs[2].toDouble();
#endif
                }
                else ; // !!!*** error

            JsonObject mjs;
            jstools::parseJson(js, "MaterialLimited", mjs);
                jstools::parseJson(mjs, "Enabled", MaterialLimited);
                jstools::parseJson(mjs, "Material", LimtedToMatName);
    }

    // Angular properties
    {
        JsonObject js;
        jstools::parseJson(json, "Angular", js);

            std::string str;
            jstools::parseJson(js, "Mode", str);
            if      (str == "Isotropic") AngularMode = Isotropic;
            else if (str == "Fixed")     AngularMode = FixedDirection;
            else if (str == "Gauss")     AngularMode = GaussDispersion;
            else if (str == "Custom")    AngularMode = CustomAngular;
            // !!!*** error if not found

            jstools::parseJson(js, "Phi",       DirectionPhi);
            jstools::parseJson(js, "Theta",     DirectionTheta);
            jstools::parseJson(js, "UseCutOff", UseCutOff);
            jstools::parseJson(js, "CutOff",    CutOff);
            jstools::parseJson(js, "Sigma",     DispersionSigma);
            JsonArray ar;
                jstools::parseJson(js, "CustomDistribution", ar);
            jstools::readDPairVectorFromArray(ar, AngularDistribution);
            configureAngularSampler(); // !!!*** error handling
    }

    // Time properties
    {
        JsonObject js;
        jstools::parseJson(json, "Time", js);

            std::string str;
            jstools::parseJson(js, "OffsetMode", str);
            if      (str == "Fixed")        TimeOffsetMode = FixedOffset;
            else if (str == "ByEventIndex") TimeOffsetMode = ByEventIndexOffset;
            else if (str == "Custom")       TimeOffsetMode = CustomDistributionOffset;
            else ; // !!!*** error reporting

            jstools::parseJson(js, "FixedOffset",   TimeFixedOffset);
            jstools::parseJson(js, "ByEventStart",  TimeByEventStart);
            jstools::parseJson(js, "ByEventPeriod", TimeByEventPeriod);

            jstools::parseJson(js, "SpreadMode", str);
            if      (str == "None")        TimeSpreadMode = NoSpread;
            else if (str == "Gaussian")    TimeSpreadMode = GaussianSpread;
            else if (str == "Uniform")     TimeSpreadMode = UniformSpread;
            else if (str == "Exponential") TimeSpreadMode = ExponentialSpread;
            else ; // !!!*** error reporting

            jstools::parseJson(js, "SpreadSigma",    TimeSpreadSigma);
            jstools::parseJson(js, "SpreadWidth",    TimeSpreadWidth);
            jstools::parseJson(js, "SpreadHalfLife", TimeSpreadHalfLife);
            jstools::parseJson(js, "HalfLifePreferUnit", str);
            if      (str == "ns")  TimeHalfLifePrefUnit = ns;
            else if (str == "us")  TimeHalfLifePrefUnit = us;
            else if (str == "ms")  TimeHalfLifePrefUnit = ms;
            else if (str == "s")   TimeHalfLifePrefUnit = s;
            else if (str == "min") TimeHalfLifePrefUnit = min;
            else if (str == "h")   TimeHalfLifePrefUnit = h;
            else ; // !!!*** error
            JsonArray ar;
                jstools::parseJson(js, "CustomDistribution", ar);
            jstools::readDPairVectorFromArray(ar, TimeDistribution);
            configureTimeSampler(); // !!!*** error handling
    }

    // Particles
    {
        JsonArray ar;
        jstools::parseJson(json, "Particles", ar);

            const int numGP = ar.size();
            for (int ip = 0; ip < numGP; ip++)
            {
                JsonObject js;
                jstools::arrayElementToObject(ar, ip, js);

                AGunParticle gp;
                bool bOK = gp.readFromJson(js);
                if (!bOK) return false;
                Particles.push_back(gp);
            }
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

bool AParticleSourceRecord::isDirectional() const
{
    switch (AngularMode)
    {
    case Isotropic       : return UseCutOff;
    case FixedDirection  : return true;
    case GaussDispersion : return true;
    case CustomAngular   : return true;
    default              : return false;
    }
}

std::string AParticleSourceRecord::check() const
{
    if (Activity < 0)  return "Negative activity";

    switch (Shape)
    {
    case Point     :
        break;
    case Line      :
        if (Size1 <= 0) return "Source length should be positive";
        break;
    case Rectangle :
        if (Size1 <= 0 || Size2 <= 0) return "Both sizes should be positive";
        break;
    case Round     :
        if (Size1 <= 0) return "Diameter should be positive";
        break;
    case Box       :
        if (Size1 <= 0 || Size2 <= 0 || Size3 <= 0) return "All three sizes should be positive";
        break;
    case Cylinder  :
        if (Size1 <= 0 || Size2 <= 0) return "Diameter and height should be positive";
        break;
    }

    switch (AngularMode)
    {
    case Isotropic       : break;
    case FixedDirection  : break;
    case GaussDispersion :
        if (DispersionSigma < 0) return "Dispersion sigma cannot be negative";
        break;
    case CustomAngular   :
        if (!_AngularSampler.isReady()) return "Angular sampler is not ready: Check angular distribution";
        break;
    }
    if (UseCutOff && CutOff < 0) return "Negative cut-off angle";

    if (TimeOffsetMode == AParticleSourceRecord::CustomDistributionOffset)
    {
        if (!_TimeSampler.isReady()) return "Time sampler is not ready: Check custom time offset distribution";
    }
    switch (TimeSpreadMode)
    {
    case GaussianSpread :
        if (TimeSpreadSigma < 0) return "Time spread sigma cannot be negative";
        break;
    case ExponentialSpread :
        if (TimeSpreadHalfLife <= 0) return "Half-life should be positive";
        break;
    default : break;
    }

    const int numParts = Particles.size();
    if (numParts == 0) return "No particles defined";
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
            if (Particles[gp.LinkedTo].Particle == "-") return "Particle (#" + std::to_string(ip) + "): direct deposition (\"-\" particle) cannot be set to \"BtBPair\"";
        }

        if (gp.UseFixedEnergy)
        {
            if (gp.FixedEnergy < 0) return "Negative energy is set for particle #" + std::to_string(ip);
        }
        else
        {
            if (!gp._EnergySampler.isReady()) return "Energy sampler is not ready: Check energy spectrum";
        }
    }

    if (numIndParts   == 0) return "No individual particles defined";
    if (totPartWeight == 0) return "Total statistical weight of individual particles is zero";

    return "";
}

std::string AParticleSourceRecord::configureAngularSampler()
{
    _AngularSampler.clear();
    if (AngularMode != CustomAngular) return "";
    return _AngularSampler.configure(AngularDistribution, false);
}

std::string AParticleSourceRecord::configureTimeSampler()
{
    _TimeSampler.clear();
    if (TimeOffsetMode != CustomDistributionOffset) return "";
    return _TimeSampler.configure(TimeDistribution, false);
}
