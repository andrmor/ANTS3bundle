#include "aunifiedrule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "arandomhub.h"
//#include "asimulationstatistics.h"
#include "ajsontools.h"
#include "aphoton.h"

#include <QJsonObject>

#include "TMath.h"

AUnifiedRule::AUnifiedRule(int MatFrom, int MatTo) : AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Unified;
}

AInterfaceRule::OpticalOverrideResultEnum AUnifiedRule::calculate(APhoton * Photon, const double * NormalVector)
{
    const double Refl = computeRefractionProbability(Photon, NormalVector);

    double rnd = RandomHub.uniform();

    rnd -= Refl * Cspec;
    if (rnd < 0)
    {
        // qDebug() << "Specular spike";
        //rotating the vector: K = K - 2*(NK)*N
        double NK = NormalVector[0]*Photon->v[0]; NK += NormalVector[1]*Photon->v[1];  NK += NormalVector[2]*Photon->v[2];
        Photon->v[0] -= 2.0*NK*NormalVector[0]; Photon->v[1] -= 2.0*NK*NormalVector[1]; Photon->v[2] -= 2.0*NK*NormalVector[2];

        Status = SpikeReflection;
        return Back;
    }

    rnd -= Refl * Cback;
    if (rnd < 0)
    {
        // qDebug() << "Backscatter spike";
        for (int i = 0; i < 3; i++) Photon->v[i] = -Photon->v[i];

        Status = BackscatterSpikeReflection;
        return Back;
    }

    rnd -= Refl * Cdiflobe;
    if (rnd < 0)
    {
        // qDebug() << "Lambertian";
        double norm2;
        do
        {
            Photon->generateRandomDir();
            Photon->v[0] -= NormalVector[0]; Photon->v[1] -= NormalVector[1]; Photon->v[2] -= NormalVector[2];
            norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
        }
        while (norm2 < 0.000001);

        double normInverted = 1.0/TMath::Sqrt(norm2);
        Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;

        Status = LambertianReflection;
        return Back;
    }

    calculateLocalNormal(NormalVector, Photon->v);
    rnd -= Refl * Cspeclobe;
    if (rnd < 0)
    {
        qDebug() << "Spectral lobe (microfacet reflection)";
        //rotating the vector: K = K - 2*(NK)*N
        double NK = LocalNormal[0]*Photon->v[0]; NK += LocalNormal[1]*Photon->v[1];  NK += LocalNormal[2]*Photon->v[2];
        Photon->v[0] -= 2.0*NK*LocalNormal[0]; Photon->v[1] -= 2.0*NK*LocalNormal[1]; Photon->v[2] -= 2.0*NK*LocalNormal[2];

        Status = LobeReflection;
        return Back;
    }

    // What is left: transmission (microfacet!)

    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    if (MatHub[MatTo]->Dielectric)
    {
        // must be synchronized with performRefraction() method of APhotontracer class
        // must be synchronized with AInterfaceRuleTester::on_pbTracePhotons_clicked()

        const double RefrIndexFrom = MatHub[MatFrom]->getRefractiveIndex(Photon->waveIndex);
        const double RefrIndexTo   = MatHub[MatTo]->getRefractiveIndex(Photon->waveIndex);

        const double nn = RefrIndexFrom / RefrIndexTo;
        double NK = 0;
        for (int i = 0; i < 3; i++) NK += Photon->v[i] * LocalNormal[i];

        const double UnderRoot = 1.0 - nn*nn*(1.0 - NK*NK);
        if (UnderRoot < 0)
        {
            //should not happen --> reflection coefficient takes it into account
            Status = Error;
            return Absorbed;
        }
        const double tmp = nn * NK - sqrt(UnderRoot);
        for (int i = 0; i < 3; i++) Photon->v[i] = nn * Photon->v[i] - tmp * LocalNormal[i];

        Status = Transmission;
        return Forward;
    }
    // else interface on Metal -> assuming absorption
    Status = Absorption;
    return Absorbed;
}

double AUnifiedRule::computeRefractionProbability(const APhoton * Photon, const double * NormalVector) const
{
    // has to be synchronized (algorithm) with the method calculateReflectionProbability() of the APhotonTracer class of lsim module!
    // has to be synchronized (algorithm) with the AInterfaceRuleTester calculateReflectionProbability(const APhoton & Photon)

    double NK = 0;
    for (int i = 0; i < 3; i++) NK += Photon->v[i] * NormalVector[i];
    const double cos1 = fabs(NK); // cos of the angle of incidence
    const double sin1 = (cos1 < 0.9999999) ? sqrt(1.0 - cos1*cos1) : 0;

    const AMaterialHub & MatHub = AMaterialHub::getConstInstance();
    if (MatHub[MatTo]->Dielectric)
    {
        const double RefrIndexFrom = MatHub[MatFrom]->getRefractiveIndex(Photon->waveIndex);
        const double RefrIndexTo   = MatHub[MatTo]->getRefractiveIndex(Photon->waveIndex);

        const double sin2 = RefrIndexFrom / RefrIndexTo * sin1;
        if (fabs(sin2) > 1.0)
        {
            // qDebug()<<"Total internal reflection, RefCoeff = 1.0";
            return 1.0;
        }
        else
        {
            const double cos2 = sqrt(1.0 - sin2*sin2);
            double Rs = (RefrIndexFrom*cos1 - RefrIndexTo*cos2) / (RefrIndexFrom*cos1 + RefrIndexTo*cos2);
            Rs *= Rs;
            double Rp = (RefrIndexFrom*cos2 - RefrIndexTo*cos1) / (RefrIndexFrom*cos2 + RefrIndexTo*cos1);
            Rp *= Rp;
            return 0.5 * (Rs + Rp);
        }
    }
    else
    {
        const double nFrom = MatHub[MatFrom]->getRefractiveIndex(Photon->waveIndex);
        const std::complex<double> & NTo = MatHub[MatTo]->getComplexRefractiveIndex(Photon->waveIndex);

        const std::complex<double> sin2 = sin1 / NTo * nFrom;
        const std::complex<double> cos2 = sqrt( 1.0 - sin2*sin2 );

        const std::complex<double> rs = (nFrom*cos1 -   NTo*cos2) / (nFrom*cos1 +   NTo*cos2);
        const std::complex<double> rp = ( -NTo*cos1 + nFrom*cos2) / (  NTo*cos1 + nFrom*cos2);

        const double RS = std::norm(rs);
        const double RP = std::norm(rp);

        return 0.5 * (RS + RP);
    }
}

QString AUnifiedRule::getReportLine() const
{
    return "Uni";
}

QString AUnifiedRule::getLongReportLine() const
{
    return "UniLong";
}

void AUnifiedRule::doWriteToJson(QJsonObject &json) const
{
    json["Cspec"]     = Cspec;
    json["Cspeclobe"] = Cspeclobe;
    json["Cdiflobe"]  = Cdiflobe;
    json["Cback"]     = Cback;
}

bool AUnifiedRule::doReadFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "Cspec",     Cspec);
    jstools::parseJson(json, "Cspeclobe", Cspeclobe);
    jstools::parseJson(json, "Cdiflobe",  Cdiflobe);
    jstools::parseJson(json, "Cback",     Cback);

    return true;
}

QString AUnifiedRule::doCheckOverrideData()
{
    if (Cspec + Cspeclobe + Cdiflobe + Cback != 1.0) return "Sum of all coefficients should be unity!";
    return "";
}

