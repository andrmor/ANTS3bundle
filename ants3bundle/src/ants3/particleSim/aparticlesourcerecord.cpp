#include "aparticlesourcerecord.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "acommonfunctions.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

#include "TH1D.h"

double GunParticleStruct::generateEnergy() const
{
    if (UseFixedEnergy) return Energy;
    return EnergyDistr.getRandom();
}

bool GunParticleStruct::loadSpectrum(const std::string &fileName)
{
/*
    QVector<double> x, y;
    QVector<QVector<double> *> V = {&x, &y};
    const QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return false; // !!!*** return string

    delete EnergyDistr; EnergyDistr = 0;
    int size = x.size();
    EnergyDistr = new TH1D("","Energy spectrum", size-1, x.data());
    for (int j = 1; j < size+1; j++)
        EnergyDistr->SetBinContent(j, y[j-1]);
*/
    return true;
}

void GunParticleStruct::writeToJson(QJsonObject & json) const
{
    json["Particle"]   = QString(Particle.data());
    json["StatWeight"] = StatWeight;
    json["Individual"] = Individual;
    json["LinkedTo"] = LinkedTo;
    json["LinkingProbability"] = LinkedProb;
    json["LinkingOppositeDir"] = LinkedOpposite;
    json["Energy"] = Energy;
    json["UseFixedEnergy"] = UseFixedEnergy;
//    if ( spectrum )
//    {
//        QJsonArray ja;
//            writeTH1DtoJsonArr(spectrum, ja);
//        json["EnergySpectrum"] = ja;
//    }
    json["PreferredUnits"] = QString(PreferredUnits.data());
}

bool GunParticleStruct::readFromJson(const QJsonObject & json)
{
    QString tmp;
    jstools::parseJson(json, "Particle", tmp); Particle = tmp.toLatin1().data();

    jstools::parseJson(json, "StatWeight",  StatWeight );
    jstools::parseJson(json, "Individual",  Individual );
    jstools::parseJson(json, "LinkedTo",  LinkedTo ); //linked always to previously already defined particles!
    jstools::parseJson(json, "LinkingProbability",  LinkedProb );
    jstools::parseJson(json, "LinkingOppositeDir",  LinkedOpposite );

    jstools::parseJson(json, "Energy",  Energy );
    jstools::parseJson(json, "PreferredUnits",  tmp); PreferredUnits = tmp.toLatin1().data();
    jstools::parseJson(json, "UseFixedEnergy",  UseFixedEnergy );

/*
    QJsonArray ar = json["EnergySpectrum"].toArray();
    if (!ar.isEmpty())
    {
        int size = ar.size();
        double* xx = new double [size];
        double* yy = new double [size];
        for (int i=0; i<size; i++)
        {
            xx[i] = ar[i].toArray()[0].toDouble();
            yy[i] = ar[i].toArray()[1].toDouble();
        }
        EnergyDistr = new TH1D("", "", size-1, xx);
        for (int j = 1; j<size+1; j++) EnergyDistr->SetBinContent(j, yy[j-1]);
        delete[] xx;
        delete[] yy;
    }
*/
    return true;
}

// ---------------------- AParticleSourceRecord ----------------------

void AParticleSourceRecord::clear()
{
    name  = "No_name";
    shape = Point;

    X0    = 0;
    Y0    = 0;
    Z0    = 0;

    Phi   = 0;
    Theta = 0;
    Psi   = 0;

    size1 = 10.0;
    size2 = 10.0;
    size3 = 10.0;

    CollPhi   = 0;
    CollTheta = 0;
    Spread    = 45.0;

    DoMaterialLimited = false;
    LimtedToMatName.clear();

    Activity = 1.0;

    TimeAverageMode = 0;
    TimeAverage = 0;
    TimeAverageStart = 0;
    TimeAveragePeriod = 10.0;
    TimeSpreadMode = 0;
    TimeSpreadSigma = 50.0;
    TimeSpreadWidth = 100.0;

    GunParticles.clear();
}

void AParticleSourceRecord::writeToJson(QJsonObject & json) const
{
    json["Name"] = QString(name.data());

    QString str;
    switch (shape)
    {
    case Point     : str = "point";
    case Line      : str = "line";
    case Rectangle : str = "rectangle";
    case Round     : str = "round";
    case Box       : str = "box";
    case Cylinder  : str = "cylinder";
    }
    json["Shape"] = str;

    json["Activity"] = Activity;
    json["X"] = X0;
    json["Y"] = Y0;
    json["Z"] = Z0;
    json["Size1"] = size1;
    json["Size2"] = size2;
    json["Size3"] = size3;
    json["Phi"] = Phi;
    json["Theta"] = Theta;
    json["Psi"] = Psi;
    json["CollPhi"] = CollPhi;
    json["CollTheta"] = CollTheta;
    json["Spread"] = Spread;

    json["DoMaterialLimited"] = DoMaterialLimited;
    json["LimitedToMaterial"] = QString(LimtedToMatName.data());

    json["TimeAverageMode"] = TimeAverageMode;
    json["TimeAverage"] = TimeAverage;
    json["TimeAverageStart"] = TimeAverageStart;
    json["TimeAveragePeriod"] = TimeAveragePeriod;
    json["TimeSpreadMode"] = TimeSpreadMode;
    json["TimeSpreadSigma"] = TimeSpreadSigma;
    json["TimeSpreadWidth"] = TimeSpreadWidth;

    //particles
    int GunParticleSize = GunParticles.size();
    json["Particles"] = GunParticleSize;
    QJsonArray jParticleEntries;
    for (const GunParticleStruct & gp : GunParticles)
    {
        QJsonObject js;
        gp.writeToJson(js);
        jParticleEntries.append(js);
    }
    json["GunParticles"] = jParticleEntries;
}

bool AParticleSourceRecord::readFromJson(const QJsonObject & json)
{
    clear();

    QString tmp;
    jstools::parseJson(json, "Name", tmp); name = tmp.toLatin1().data();

    jstools::parseJson(json, "Type", tmp);
    if      (tmp == "point")     shape = Point;
    else if (tmp == "line")      shape = Line;
    else if (tmp == "rectangle") shape = Rectangle;
    else if (tmp == "round")     shape = Round;
    else if (tmp == "box")       shape = Box;
    else if (tmp == "cylinder")  shape = Cylinder;

    jstools::parseJson(json, "Activity", Activity);

    jstools::parseJson(json, "X", X0);
    jstools::parseJson(json, "Y", Y0);
    jstools::parseJson(json, "Z", Z0);

    jstools::parseJson(json, "Size1", size1);
    jstools::parseJson(json, "Size2", size2);
    jstools::parseJson(json, "Size3", size3);

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

    jstools::parseJson(json, "DoMaterialLimited", DoMaterialLimited);
    jstools::parseJson(json, "LimitedToMaterial", tmp); LimtedToMatName = tmp.toLatin1().data();

    QJsonArray jGunPartArr = json["GunParticles"].toArray();
    const int numGP = jGunPartArr.size();
    for (int ip = 0; ip < numGP; ip++)
    {
        QJsonObject jThisGunPart = jGunPartArr[ip].toObject();

        GunParticleStruct gp;
        bool bOK = gp.readFromJson(jThisGunPart);
        if (!bOK) return false;
        GunParticles.push_back(gp);
    }
    return true;
}

std::string AParticleSourceRecord::getShapeString() const
{
    switch (shape)
    {
    case Point     : return "Point";
    case Line      : return "Line";
    case Rectangle : return "Rectangle";
    case Round     : return "Round";
    case Box       : return "Box";
    case Cylinder  : return "Cylinder";
    }
}

std::string AParticleSourceRecord::check() const
{
    const int numParts = GunParticles.size();
    if (numParts == 0) return "No particles defined";

    if (Spread < 0)    return "negative spread angle";
    if (Activity < 0)  return "negative activity";

    int    numIndParts   = 0;
    double TotPartWeight = 0;
    for (int ip = 0; ip < numParts; ip++)
    {
        const GunParticleStruct & gp = GunParticles.at(ip);
        if (gp.Individual)
        {
            numIndParts++;
            if (GunParticles.at(ip).StatWeight < 0) return "Negative statistical weight for particle #" + std::to_string(ip);
            TotPartWeight += GunParticles.at(ip).StatWeight;
        }
        else
        {
            if (ip == gp.LinkedTo) return "Particle #" + std::to_string(ip) + " is linked to itself";
            if (ip <  gp.LinkedTo) return "Invalid linking for particle #" + std::to_string(ip);
        }

        if (gp.Energy <= 0) return "Energy <= 0 for particle #" + std::to_string(ip);
    }

    if (numIndParts   == 0) return "No individual particles defined";
    if (TotPartWeight == 0) return "Total statistical weight of individual particles is zero";

    return "";
}

#include "amaterialhub.h"
void AParticleSourceRecord::updateLimitedToMat()
{
    bool bFound = false;
    int iMat = 0;
    const QStringList mats = AMaterialHub::getConstInstance().getListOfMaterialNames();
    const QString LimitTo = LimtedToMatName.data();
    for (; iMat < mats.size(); iMat++)
        if (LimitTo == mats[iMat])
        {
            bFound = true;
            break;
        }

    if (bFound)
    {
        bLimitToMat = true;
        LimitedToMat = iMat;
    }
    else bLimitToMat = false;
}
