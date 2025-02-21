#include "abasicinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
//#include "amaterialhub.h"
#include "arandomhub.h"
//#include "asimulationstatistics.h"
#include "ajsontools.h"

#include <QJsonObject>

#include "TMath.h"

ABasicInterfaceRule::ABasicInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Polished;
}

AInterfaceRule::EInterfaceRuleResult ABasicInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    double rnd = RandomHub.uniform();

    // surface loss?
    rnd -= Abs;
    if (rnd < 0)
    {
        // qDebug()<<"Override: surface loss!";
        Status = Absorption;
        return Absorbed;
    }

    // specular reflection?
    rnd -= Spec;
    if (rnd < 0)
    {
        // qDebug()<<"Override: specular reflection!";
        //rotating the vector: K = K - 2*(NK)*N
        if (SurfaceSettings.isPolished())
        {
            double NK = NormalVector[0]*Photon->v[0]; NK += NormalVector[1]*Photon->v[1];  NK += NormalVector[2]*Photon->v[2];
            Photon->v[0] -= 2.0*NK*NormalVector[0]; Photon->v[1] -= 2.0*NK*NormalVector[1]; Photon->v[2] -= 2.0*NK*NormalVector[2];
            Status = SpikeReflection;
        }
        else
        {
            calculateLocalNormal(NormalVector, Photon->v);
            double NK = LocalNormal[0]*Photon->v[0]; NK += LocalNormal[1]*Photon->v[1];  NK += LocalNormal[2]*Photon->v[2];
            Photon->v[0] -= 2.0*NK*LocalNormal[0]; Photon->v[1] -= 2.0*NK*LocalNormal[1]; Photon->v[2] -= 2.0*NK*LocalNormal[2];
            Status = LobeReflection;
        }
        return Back;
    }

    // scattering?
    rnd -= Scat;
    if (rnd<0)
    {
        // qDebug()<<"scattering triggered";

        switch (ScatterModel)
        {
        case 0: //4Pi scattering
            // qDebug()<<"4Pi scatter";
             Photon->generateRandomDir();
            // qDebug()<<"New direction:"<<K[0]<<K[1]<<K[2];

            //enering new volume or backscattering?
            //normal is in the positive direction in respect to the original direction!
            if (Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2] < 0)
            {
                // qDebug()<<"   scattering back";
                Status = LambertianReflection;
                return Back;
            }
            // qDebug()<<"   continuing to the next volume";
            Status = Transmission;
            return Forward;

        case 1: //2Pi lambertian, remaining in the same volume (back scattering)
        {
            // qDebug()<<"2Pi lambertian scattering backward";
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
        case 2: //2Pi lambertian, scattering to the next volume
        {
            // qDebug()<<"2Pi lambertian scattering forward";
            double norm2;
            do
            {
                Photon->generateRandomDir();
                Photon->v[0] += NormalVector[0]; Photon->v[1] += NormalVector[1]; Photon->v[2] += NormalVector[2];
                norm2 = Photon->v[0]*Photon->v[0] + Photon->v[1]*Photon->v[1] + Photon->v[2]*Photon->v[2];
            }
            while (norm2 < 0.000001);

            double normInverted = 1.0/TMath::Sqrt(norm2);
            Photon->v[0] *= normInverted; Photon->v[1] *= normInverted; Photon->v[2] *= normInverted;
            Status = Transmission;
            return Forward;
        }
        }
    }

    // overrides NOT triggered
    if (isPolishedSurface())
    {
        // what is left is covered by Fresnel in the tracker code
        // qDebug()<<"Overrides did not trigger, using fresnel";
        Status = Transmission;
        return NotTriggered;
    }

    // rough surface, calculating facet normal and delegating the rest to the tracer
    calculateLocalNormal(NormalVector, Photon->v);
    Status = LocalNormalDelegated;
    return DelegateLocalNormal;
}

QString ABasicInterfaceRule::getReportLine() const
{
    double probFresnel = 1.0 - (Spec + Scat + Abs);
    QString s;
    if (Abs > 0) s += QString("Abs %1 +").arg(Abs);
    if (Spec > 0)  s += QString("Spec %1 +").arg(Spec);
    if (Scat > 0)
    {
        switch( ScatterModel )
        {
        case 0:
            s += "Iso ";
            break;
        case 1:
            s += "Lamb_B ";
            break;
        case 2:
            s += "Lamb_F ";
            break;
        }
        s += QString::number(Scat);
        s += " +";
    }
    if (probFresnel > 1e-10) s += QString("Fres %1 +").arg(probFresnel);
    s.chop(2);
    return s;
}

QString ABasicInterfaceRule::getLongReportLine() const
{
    QString s = "--> Simplistic <--\n";
    if (Abs > 0) s += QString("Absorption: %1%\n").arg(100.0 * Abs);
    if (Spec > 0)  s += QString("Specular reflection: %1%\n").arg(100.0 * Spec);
    if (Scat)
    {
        s += QString("Scattering: %1%").arg(100.0 * Scat);
        switch (ScatterModel)
        {
        case 0: s += " (isotropic)\n"; break;
        case 1: s += " (Lambertian, back)\n"; break;
        case 2: s += " (Lambertian, forward)\n"; break;
        }
    }
    double fres = 1.0 - Abs - Spec - Scat;
    if (fres > 0) s += QString("Fresnel: %1%").arg(100.0 * fres);
    return s;
}

QString ABasicInterfaceRule::getDescription() const
{
    QString txt = "This interface defines three wavelength-independent parameters:\n"
                  "1) Absorption - the probability the photon is killed\n"
                  "2) Specuar reflection - the probability of specular reflection (flat or rough surface: see below)\n"
                  "3) Scattering\n"
                  "    3a) Lambertian back in 2Pi\n"
                  "    3b) Lambertian forward in 2Pi\n"
                  "    3c) Isotropic scattering in 4Pi\n"
                  "All values must be in the range from 0 to 1\n"
                  "If the sum does not ammounts to 1, the remaining fraction is the \"normal\" Freshnel/Snell physics\n"
                  "\n"
                  "The rough surface settings only affect specular reflection!";
    return txt;
}

void ABasicInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    json["Abs"]      = Abs;
    json["Spec"]     = Spec;
    json["Scat"]     = Scat;
    json["ScatMode"] = ScatterModel;
}

bool ABasicInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    if ( !jstools::parseJson(json, "Abs",      Abs) )          return false;
    if ( !jstools::parseJson(json, "Spec",     Spec) )         return false;
    if ( !jstools::parseJson(json, "Scat",     Scat) )         return false;
    if ( !jstools::parseJson(json, "ScatMode", ScatterModel) ) return false;
    return true;
}

QString ABasicInterfaceRule::doCheckOverrideData()
{
    if (ScatterModel < 0 || ScatterModel > 2) return "Invalid scatter model";

    if (Abs  < 0 || Abs  > 1.0) return "Absorption probability should be within [0, 1.0]";
    if (Spec < 0 || Spec > 1.0) return "Reflection probability should be within [0, 1.0]";
    if (Scat < 0 || Scat > 1.0) return "Scattering probability should be within [0, 1.0]";

    if (Abs + Spec + Scat > 1.0) return "Sum of all process probabilities cannot exceed 1.0";

    return "";
}
