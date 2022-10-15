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

AMetalInterfaceRule::AMetalInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Polished;
}

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

void AMetalInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    json["RealN"]  = RealN;
    json["ImaginaryN"]  = ImaginaryN;
}

bool AMetalInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    if ( !jstools::parseJson(json, "RealN", RealN) ) return false;
    if ( !jstools::parseJson(json, "ImaginaryN", ImaginaryN) ) return false;
    return true;
}

QString AMetalInterfaceRule::doCheckOverrideData()
{
    return "";
}

AInterfaceRule::OpticalOverrideResultEnum AMetalInterfaceRule::calculate(APhoton * photon, const double * globalNormal)
{
tryAgainLabel:
    double cosTheta = 0;
    if (SurfaceSettings.isPolished())
        for (int i = 0; i < 3; i++) cosTheta += photon->v[i] * globalNormal[i];
    else
    {
        calculateLocalNormal(globalNormal, photon->v);
        for (int i = 0; i < 3; i++) cosTheta += photon->v[i] * LocalNormal[i];
    }
    const double reflCoeff = calculateReflectivity(cosTheta, RealN, ImaginaryN, photon->waveIndex);

    if (RandomHub.uniform() > reflCoeff)
    {
        //Absorption
        //qDebug() << "Override: Loss on metal";
        Status = Absorption;
        return Absorbed;
    }

    //else specular reflection --> rotating the vector: K = K - 2*(NK)*N
    double NK = 0;
    if (SurfaceSettings.isPolished())
    {
        for (int i = 0; i < 3; i++) NK += photon->v[i] * globalNormal[i];
        for (int i = 0; i < 3; i++) photon->v[i] -= 2.0 * NK * globalNormal[i];
        Status = SpikeReflection;
    }
    else
    {
        for (int i = 0; i < 3; i++) NK += photon->v[i] * LocalNormal[i];
        for (int i = 0; i < 3; i++) photon->v[i] -= 2.0 * NK * LocalNormal[i];

        double GNK = 0;
        for (int i = 0; i < 3; i++) GNK += photon->v[i] * globalNormal[i];
        if (GNK > 0) //back only in respect to the local normal but actually forward considering global one
        {
            //qDebug() << "Rule result is 'Back', but direction is actually 'Forward' --> re-running the rule";
            goto tryAgainLabel;
        }

        Status = LobeReflection;
    }

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
