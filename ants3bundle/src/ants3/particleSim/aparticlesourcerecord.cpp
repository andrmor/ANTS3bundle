#include "aparticlesourcerecord.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "acommonfunctions.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

#include "TH1D.h"

GunParticleStruct * GunParticleStruct::clone() const
{
    //shallow copy
    GunParticleStruct* gp = new GunParticleStruct(*this);

    //clear dynamic
    gp->spectrum = 0;

    //deep copy for dynamic properties
    if (spectrum)
        gp->spectrum = new TH1D(*spectrum);

    return gp;
}

double GunParticleStruct::generateEnergy() const
{
    if (bUseFixedEnergy || !spectrum) return energy;
    return GetRandomFromHist(spectrum);
}

bool GunParticleStruct::loadSpectrum(const QString &fileName)
{
    QVector<double> x, y;
    QVector<QVector<double> *> V = {&x, &y};
    const QString res = ftools::loadDoubleVectorsFromFile(fileName, V);
    if (!res.isEmpty()) return false; // !!!*** return string

    delete spectrum; spectrum = 0;
    int size = x.size();
    spectrum = new TH1D("","Energy spectrum", size-1, x.data());
    for (int j = 1; j < size+1; j++)
        spectrum->SetBinContent(j, y[j-1]);
    return true;
}

void GunParticleStruct::writeToJson(QJsonObject & json) const
{
    json["Particle"] = Particle;
    json["StatWeight"] = StatWeight;
    json["Individual"] = Individual;
    json["LinkedTo"] = LinkedTo;
    json["LinkingProbability"] = LinkingProbability;
    json["LinkingOppositeDir"] = LinkingOppositeDir;
    json["Energy"] = energy;
    json["UseFixedEnergy"] = bUseFixedEnergy;
//    if ( spectrum )
//    {
//        QJsonArray ja;
//            writeTH1DtoJsonArr(spectrum, ja);
//        json["EnergySpectrum"] = ja;
//    }
    json["PreferredUnits"] = PreferredUnits;
}

bool GunParticleStruct::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Particle",  Particle );
    jstools::parseJson(json, "StatWeight",  StatWeight );
    jstools::parseJson(json, "Individual",  Individual );
    jstools::parseJson(json, "LinkedTo",  LinkedTo ); //linked always to previously already defined particles!
    jstools::parseJson(json, "LinkingProbability",  LinkingProbability );
    jstools::parseJson(json, "LinkingOppositeDir",  LinkingOppositeDir );

    jstools::parseJson(json, "Energy",  energy );
    jstools::parseJson(json, "PreferredUnits",  PreferredUnits );
    jstools::parseJson(json, "UseFixedEnergy",  bUseFixedEnergy );

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
        spectrum = new TH1D("", "", size-1, xx);
        for (int j = 1; j<size+1; j++) spectrum->SetBinContent(j, yy[j-1]);
        delete[] xx;
        delete[] yy;
    }
    return true;
}

GunParticleStruct::~GunParticleStruct()
{
    delete spectrum; spectrum = nullptr;
}

// ---------------------- AParticleSourceRecord ----------------------

AParticleSourceRecord::~AParticleSourceRecord()
{
    clearGunParticles();
}

void AParticleSourceRecord::clearGunParticles()
{
    for (GunParticleStruct * g : GunParticles) delete g;
    GunParticles.clear();
}

AParticleSourceRecord * AParticleSourceRecord::clone() const
{
    //shallow copy
    AParticleSourceRecord * newRec = new AParticleSourceRecord(*this);

    //clear dynamic
    newRec->GunParticles.clear();

    //deep copy of dynamic resources
    for (GunParticleStruct* g : GunParticles)
        newRec->GunParticles.push_back(g->clone());

    return newRec;
}

void AParticleSourceRecord::writeToJson(QJsonObject & json) const
{
    json["Name"] = name;
    json["Type"] = shape;
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
    json["LimitedToMaterial"] = LimtedToMatName;

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
    for (const GunParticleStruct* gp : GunParticles)
    {
        QJsonObject js;
        gp->writeToJson(js);
        jParticleEntries.append(js);
    }
    json["GunParticles"] = jParticleEntries;
}

bool AParticleSourceRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Name", name);
    jstools::parseJson(json, "Type", shape);
    jstools::parseJson(json, "Activity", Activity);
    jstools::parseJson(json, "X", X0);
    jstools::parseJson(json, "Y", Y0);
    jstools::parseJson(json, "Z", Z0);
    jstools::parseJson(json, "Size1", size1);
    jstools::parseJson(json, "Size2", size2);
    jstools::parseJson(json, "Size3", size3);
    jstools::parseJson(json, "Phi", Phi);
    jstools::parseJson(json, "Theta", Theta);
    jstools::parseJson(json, "Psi", Psi);
    jstools::parseJson(json, "CollPhi", CollPhi);
    jstools::parseJson(json, "CollTheta", CollTheta);
    jstools::parseJson(json, "Spread", Spread);

    TimeAverageMode = 0;
    jstools::parseJson(json, "TimeAverageMode", TimeAverageMode);
    TimeAverage = 0;
    jstools::parseJson(json, "TimeAverage", TimeAverage);
    TimeAverageStart = 0;
    jstools::parseJson(json, "TimeAverageStart", TimeAverageStart);
    TimeAveragePeriod = 10.0;
    jstools::parseJson(json, "TimeAveragePeriod", TimeAveragePeriod);
    TimeSpreadMode = 0;
    jstools::parseJson(json, "TimeSpreadMode", TimeSpreadMode);
    TimeSpreadSigma = 50.0;
    jstools::parseJson(json, "TimeSpreadSigma", TimeSpreadSigma);
    TimeSpreadWidth = 100.0;
    jstools::parseJson(json, "TimeSpreadWidth", TimeSpreadWidth);

    DoMaterialLimited = bLimitToMat = false;
    LimtedToMatName = "";
    if (json.contains("DoMaterialLimited"))
    {
        jstools::parseJson(json, "DoMaterialLimited", DoMaterialLimited);
        jstools::parseJson(json, "LimitedToMaterial", LimtedToMatName);

        if (DoMaterialLimited) updateLimitedToMat();
    }

    //GunParticles
    clearGunParticles();
    QJsonArray jGunPartArr = json["GunParticles"].toArray();
    int numGP = jGunPartArr.size();
    //qDebug()<<"Entries in gunparticles:"<<numGP;
    for (int ip = 0; ip < numGP; ip++)
    {
        QJsonObject jThisGunPart = jGunPartArr[ip].toObject();

        GunParticleStruct* gp = new GunParticleStruct();
        bool bOK = gp->readFromJson(jThisGunPart);
        if (!bOK)
        {
            delete gp;
            return false;
        }
        GunParticles.push_back(gp);
    }
    return true;
}

const QString AParticleSourceRecord::getShapeString() const
{
    switch (shape)
    {
    case 0: return "Point";
    case 1: return "Linear";
    case 2: return "Square";
    case 3: return "Round";
    case 4: return "Box";
    case 5: return "Cylinder";
    default: ;
    }
    return "-error-";
}

const QString AParticleSourceRecord::checkSource() const
{
    if (shape < 0 || shape > 5) return "unknown source shape";

    int numParts = GunParticles.size();     //11 - no particles defined
    if (numParts == 0) return "no particles defined";

    if (Spread < 0) return "negative spread angle";
    if (Activity < 0) return "negative activity";

    //checking all particles
    int numIndParts = 0;
    double TotPartWeight = 0;
    for (int ip = 0; ip<numParts; ip++)
    {
        GunParticleStruct * gp = GunParticles.at(ip);
        if (gp->Individual)
        {
            //individual
            numIndParts++;
            if (GunParticles.at(ip)->StatWeight < 0) return QString("negative statistical weight for particle #%1").arg(ip);
            TotPartWeight += GunParticles.at(ip)->StatWeight;
        }
        else
        {
            //linked
            if (ip == gp->LinkedTo) return QString("particle #%1 is linked to itself").arg(ip);
            if (ip < gp->LinkedTo) return QString("invalid linking for particle #%1").arg(ip);
        }

        if (gp->energy <= 0) return QString("invalid energy of %1 for particle #%2").arg(gp->energy).arg(ip);
    }

    if (numIndParts == 0) return "no individual particles defined";
    if (TotPartWeight == 0) return "total statistical weight of individual particles is zero";

    return "";
}

#include "amaterialhub.h"
void AParticleSourceRecord::updateLimitedToMat()
{
    bool bFound = false;
    int iMat = 0;
    const QStringList mats = AMaterialHub::getConstInstance().getListOfMaterialNames();
    for (; iMat < mats.size(); iMat++)
        if (LimtedToMatName == mats[iMat])
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
