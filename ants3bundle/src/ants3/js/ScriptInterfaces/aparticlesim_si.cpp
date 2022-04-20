#include "aparticlesim_si.h"
#include "aparticlesimmanager.h"
#include "aerrorhub.h"
#include "acalorimeterhub.h"
#include "acalorimeter.h"
#include "ath.h"

#include <QDebug>
#include <QVariant>

#include "TH1D.h"

AParticleSim_SI::AParticleSim_SI() :
    SimMan(AParticleSimManager::getInstance()) {}

void AParticleSim_SI::simulate(bool updateGui)
{
    AErrorHub::clear();

    SimMan.simulate();

    QString err = AErrorHub::getQError();
    if (err.isEmpty())
    {
        if (updateGui) SimMan.requestUpdateResultsGUI();;
    }
    else
    {
        abort(err);
    }
}

int AParticleSim_SI::countCalorimeters()
{
    return ACalorimeterHub::getConstInstance().countCalorimeters();
}

QVariantList AParticleSim_SI::getCalorimeterData(int calorimeterIndex, QString mode)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();

    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }

    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }

    ATH3D * data = cal->Deposition;
    if (!data)
    {
        abort("Calorimeter has no data!");
        return res;
    }

    mode = mode.toLower();
    if (mode == "x" || mode == "y" || mode == "z")
    {
        TH1D * h = (TH1D*)data->Project3D(mode.toLatin1().data());
        const int num = h->GetXaxis()->GetNbins();
        for (int i = 0; i < num; i++)
            res.push_back( QVariantList{h->GetBinCenter(i+1), h->GetBinContent(i+1)} );
    }
    else if (mode == "xy" || mode == "xz" || mode == "yx" || mode == "yz" || mode == "zx" || mode == "zy")
    {
        TH2D * h = (TH2D*)data->Project3D(mode.toLatin1().data());
        const int numX = h->GetXaxis()->GetNbins();
        const int numY = h->GetYaxis()->GetNbins();
        for (int ix = 0; ix < numX; ix++)
        {
            for (int iy = 0; iy < numY; iy++)
                res.push_back( QVariantList{h->GetXaxis()->GetBinCenter(ix+1), h->GetYaxis()->GetBinCenter(iy+1), h->GetBinContent(ix+1, iy+1)} );
        }
    }
    else abort("Undefined option in getCalorimeterData()");

    return res;
}

QVariantList AParticleSim_SI::getCalorimeterProperties(int calorimeterIndex)
{
    QVariantList res;
    const ACalorimeterHub & CalHub = ACalorimeterHub::getConstInstance();

    const int numCal = CalHub.countCalorimeters();
    if (calorimeterIndex < 0 || calorimeterIndex >= numCal)
    {
        abort("Invalid calorimeter index");
        return res;
    }

    ACalorimeter * cal = CalHub.Calorimeters[calorimeterIndex].Calorimeter;
    if (!cal)
    {
        abort("Calorimeter is nullptr!");
        return res;
    }

    const ACalorimeterProperties & prop = cal->Properties;

    QVariantList vlB;
    QVariantList vlO;
    QVariantList vlS;
    for (int i=0; i<3; i++)
    {
        vlB << prop.Bins[i];
        vlO << prop.Origin[i];
        vlS << prop.Step[i];
    }
    res.push_back(vlB);
    res.push_back(vlO);
    res.push_back(vlS);
    return res;
}
