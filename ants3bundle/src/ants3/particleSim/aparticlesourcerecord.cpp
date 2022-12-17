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

    TimeAverageMode   = Single;
    TimeAverage       = 0;
    TimeAverageStart  = 0;
    TimeAveragePeriod = 10.0;
    TimeSpreadMode    = NoSpread;
    TimeSpreadSigma   = 50.0;
    TimeSpreadWidth   = 100.0;

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

    // Time properties
    {
        QJsonObject js;
            QString str;
                switch (TimeAverageMode)
                {
                case Single : str = "Single"; break;
                case Train  : str = "Train";  break;
                default : qCritical() << "Not implemented TimeAverageMode in AParticleSourceRecord::writeToJson"; exit(111);
                }
            js["AverageMode"]   = str;
            js["Average"]       = TimeAverage;
            js["AverageStart"]  = TimeAverageStart;
            js["AveragePeriod"] = TimeAveragePeriod;
                switch (TimeSpreadMode)
                {
                case NoSpread      : str = "NoSpread";      break;
                case GaussSpread   : str = "GaussSpread";   break;
                case UniformSpread : str = "UniformSpread"; break;
                default : qCritical() << "Not implemented TimeSpreadMode in AParticleSourceRecord::writeToJson"; exit(111);
                }
            js["SpreadMode"]    = str;
            js["SpreadSigma"]   = TimeSpreadSigma;
            js["SpreadWidth"]   = TimeSpreadWidth;
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

    // Time properties
    {
        JsonObject js;
        jstools::parseJson(json, "Time", js);

            std::string str;
            jstools::parseJson(js, "AverageMode", str);
            if      (str == "Single") TimeAverageMode = Single;
            else if (str == "Train")  TimeAverageMode = Train;
            else ; // !!!*** error reporting

            jstools::parseJson(js, "Average",       TimeAverage);
            jstools::parseJson(js, "AverageStart",  TimeAverageStart);
            jstools::parseJson(js, "AveragePeriod", TimeAveragePeriod);

            jstools::parseJson(js, "SpreadMode", str);
            if      (str == "NoSpread")      TimeSpreadMode = NoSpread;
            else if (str == "GaussSpread")   TimeSpreadMode = GaussSpread;
            else if (str == "UniformSpread") TimeSpreadMode = UniformSpread;
            else ; // !!!*** error reporting

            jstools::parseJson(js, "SpreadSigma",   TimeSpreadSigma);
            jstools::parseJson(js, "SpreadWidth",   TimeSpreadWidth);
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
