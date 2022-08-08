#include "ametalinterfacerule.h"
#include "amaterialhub.h"
#include "aphoton.h"
#include "aphotonstatistics.h"
#include "arandomhub.h"
#include "ajsontools.h"

#include <QDebug>
#include <QJsonObject>

#include "TComplex.h"
#include "TRandom2.h"

QString AMetalInterfaceRule::getReportLine() const
{
    QString s;
    s += "n = " + QString::number(RealN) + "  ";
    s += "k = " + QString::number(ImaginaryN);
    return s;
}

QString AMetalInterfaceRule::getLongReportLine() const
{
    QString s = "--> Dielectric to metal <--\n";
    s += "Refractive index of metal:\n";
    s += QString("  real: %1\n").arg(RealN);
    s += QString("  imaginary: %1").arg(ImaginaryN);
    return s;
}

void AMetalInterfaceRule::writeToJson(QJsonObject &json) const
{
    AInterfaceRule::writeToJson(json);

    json["RealN"]  = RealN;
    json["ImaginaryN"]  = ImaginaryN;
}

bool AMetalInterfaceRule::readFromJson(const QJsonObject &json)
{
    if ( !jstools::parseJson(json, "RealN", RealN) ) return false;
    if ( !jstools::parseJson(json, "ImaginaryN", ImaginaryN) ) return false;
    return true;
}

QString AMetalInterfaceRule::checkOverrideData()
{
    return "";
}

AInterfaceRule::OpticalOverrideResultEnum AMetalInterfaceRule::calculate(APhoton *Photon, const double *NormalVector)
{
    double CosTheta = Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2];

    //  double Rindex1, Rindex2;
    //  if (iWave == -1)
    //    {
    //      Rindex1 = (*MatCollection)[MatFrom]->n;
    //      //Rindex2 = (*MatCollection)[MatTo]->n;
    //      Rindex2 = RealN;
    //    }
    //  else
    //    {
    //      Rindex1 = (*MatCollection)[MatFrom]->nWaveBinned.at(iWave);
    //      //Rindex2 = (*MatCollection)[MatTo]->nWaveBinned.at(iWave);
    //      Rindex2 = RealN;
    //    }
    //  double Refl = calculateReflectivity(CosTheta, RealN/Rindex1, ImaginaryN/Rindex1);

    double Refl = calculateReflectivity(CosTheta, RealN, ImaginaryN, Photon->waveIndex);
    //qDebug() << "Dielectric-metal override: Cos theta="<<CosTheta<<" Reflectivity:"<<Refl;

    if (RandomHub.uniform() > Refl)
    {
        //Absorption
        //qDebug() << "Override: Loss on metal";
        Status = Absorption;
        return Absorbed;
    }

    //else specular reflection
    //qDebug()<<"Override: Specular reflection from metal";
    //rotating the vector: K = K - 2*(NK)*N
    double NK = NormalVector[0]*Photon->v[0]; NK += NormalVector[1]*Photon->v[1];  NK += NormalVector[2]*Photon->v[2];
    Photon->v[0] -= 2.0*NK*NormalVector[0]; Photon->v[1] -= 2.0*NK*NormalVector[1]; Photon->v[2] -= 2.0*NK*NormalVector[2];
    Status = SpikeReflection;
    return Back;
}

double AMetalInterfaceRule::calculateReflectivity(double CosTheta, double RealN, double ImaginaryN, int waveIndex)
{
    //qDebug() << "cosTheta, n, k: "<< CosTheta << RealN << ImaginaryN;

    TComplex N(RealN, ImaginaryN);
    TComplex U(1,0);

    const double SinTheta = (CosTheta < 0.9999999) ? sqrt(1.0 - CosTheta*CosTheta) : 0;
    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    const double nFrom = MatHub[MatFrom]->getRefractiveIndex(waveIndex);
    //qDebug() << "nFrom" << nFrom;

    const TComplex SinPhi = SinTheta / N * nFrom;
    //qDebug() << "SinPhi" << SinPhi.Re() << SinPhi.Im();

    //TComplex CosPhi = TMath::Sqrt( U - SinTheta*SinTheta/ (N*N/nFrom/nFrom) );
    const TComplex CosPhi = TMath::Sqrt( U - SinPhi*SinPhi );
    //qDebug() << "CosPhi:" << CosPhi.Re() << CosPhi.Im();

    const TComplex rs = (nFrom*CosTheta - N*CosPhi) / (nFrom*CosTheta + N*CosPhi);
    const TComplex rp = ( -N*CosTheta + nFrom*CosPhi) / (N*CosTheta + nFrom*CosPhi);

    const double RS = rs.Rho2();
    const double RP = rp.Rho2();
    //qDebug() << "rs" << rs.Re() << rs.Im() << RS;

    const double R = 0.5 * (RS + RP);
    //qDebug() << "Refl coeff = "<< R;

    return R;
}

