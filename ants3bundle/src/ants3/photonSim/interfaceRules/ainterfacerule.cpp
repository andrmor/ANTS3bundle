#include "ainterfacerule.h"
#include "aphotonsimhub.h"
#include "amaterialhub.h"
#include "arandomhub.h"
#include "ajsontools.h"

#include "abasicinterfacerule.h"
#include "aspectralbasicinterfacerule.h"
#include "fsnpinterfacerule.h"
#include "awaveshifterinterfacerule.h"
#include "ametalinterfacerule.h"
#include "asurfaceinterfacerule.h"
#include "aunifiedrule.h"

#include <QDebug>
#include <QJsonObject>

#include "TH1.h"

AInterfaceRule * AInterfaceRule::interfaceRuleFactory(const QString & Model, int MatFrom, int MatTo)
{
    if (Model == "Simplistic")
        return new ABasicInterfaceRule(MatFrom, MatTo);
    if (Model == "SimplisticSpectral")
        return new ASpectralBasicInterfaceRule(MatFrom, MatTo);
    if (Model == "DielectricToMetal")
        return new AMetalInterfaceRule(MatFrom, MatTo);
    if (Model == "FSNP")
        return new FsnpInterfaceRule(MatFrom, MatTo);
    if (Model == "SurfaceWLS")
        return new AWaveshifterInterfaceRule(MatFrom, MatTo);
    if (Model == "RoughSurface")
        return new ASurfaceInterfaceRule(MatFrom, MatTo);
    if (Model == "Unified")
        return new AUnifiedRule(MatFrom, MatTo);

    return nullptr; //undefined override type!
}

QStringList AInterfaceRule::getAllInterfaceRuleTypes()
{
    QStringList l;

    l << "Simplistic"
      << "SimplisticSpectral"
      << "FSNP"
      << "DielectricToMetal"
      << "SurfaceWLS"
      << "RoughSurface"
      << "Unified";

    return l;
}

AInterfaceRule::AInterfaceRule(int MatFrom, int MatTo) :
    RandomHub(ARandomHub::getInstance()),
    MatFrom(MatFrom), MatTo(MatTo) {}

AInterfaceRule::~AInterfaceRule(){}

QString AInterfaceRule::getLongReportLine() const
{
    return getReportLine();
}

void AInterfaceRule::writeToJson(QJsonObject & json) const
{
    json["Model"]   = getType();
    json["MatFrom"] = MatFrom;
    json["MatTo"]   = MatTo;

    QJsonObject jsurf;
    SurfaceSettings.writeToJson(jsurf);
    json["SurfaceProperties"] = jsurf;

    doWriteToJson(json);
}

bool AInterfaceRule::readFromJson(const QJsonObject & json)
{
    QJsonObject jsurf;
    jstools::parseJson(json, "SurfaceProperties", jsurf);
    SurfaceSettings.readFromJson(jsurf);

    return doReadFromJson(json);
}

QString AInterfaceRule::checkOverrideData()
{
    if (isNotPolishedSurface())
    {
        if (!canHaveRoughSurface()) return "This interface rule type cannot have rough optical surface";
        QString err = SurfaceSettings.checkRuntimeData();
        if (!err.isEmpty()) return err;
    }

    return doCheckOverrideData();
}

#include "TVector3.h"
void AInterfaceRule::calculateLocalNormal(const double * globalNormal, const double * photonDirection)
{
    //qDebug() << (int)SurfaceSettings.Model;
    //qDebug() << "globNorm:" << globalNormal[0] << ' ' << globalNormal[1] << ' ' << globalNormal[2];
    //qDebug() << "photDir:"  << photonDirection[0] << ' ' << photonDirection[1] << ' ' << photonDirection[2];

    switch (SurfaceSettings.Model)
    {
    case ASurfaceSettings::Polished :
        // for Polished, it should not be called at all!
        for (int i = 0; i < 3; i++) LocalNormal[i] = globalNormal[i];
        break;
    case ASurfaceSettings::Glisur :
        {
            // see G4ThreeVector G4OpBoundaryProcess::GetFacetNormal(const G4ThreeVector& Momentum, const G4ThreeVector& Normal ) const
            // https://apc.u-paris.fr/~franco/g4doxy4.10/html/_g4_op_boundary_process_8cc_source.html
            // line 665

            if (SurfaceSettings.Polish < 1.0)
            {
                double nk;
                do
                {
                    TVector3 smear;
                    do
                    {
                        smear.SetX(2.0 * RandomHub.uniform() - 1.0);
                        smear.SetY(2.0 * RandomHub.uniform() - 1.0);
                        smear.SetZ(2.0 * RandomHub.uniform() - 1.0);
                    }
                    while (smear.Mag() > 1.0);
                    smear = (1.0 - SurfaceSettings.Polish) * smear;
                    //FacetNormal = Normal + smear;
                    nk = 0;
                    for (int i = 0; i < 3; i++)
                    {
                        LocalNormal[i] = globalNormal[i] + smear[i];
                        nk += photonDirection[i] * LocalNormal[i];
                    }
                }
                //while (Momentum * FacetNormal >= 0.0);
                while (nk <= 0.0);

                //FacetNormal = FacetNormal.unit();
                double mag = 0;
                for (int i = 0; i < 3; i++) mag += LocalNormal[i] * LocalNormal[i];
                mag = sqrt(mag);
                if (abs(mag) > 1e-30)
                    for (int i = 0; i < 3; i++) LocalNormal[i] /= mag;
            }
            else
            {
                //FacetNormal = Normal;
                for (int i = 0; i < 3; i++) LocalNormal[i] = globalNormal[i];
            }

            break;
        }
    case ASurfaceSettings::Unified :
        {
            // see G4ThreeVector G4OpBoundaryProcess::GetFacetNormal(const G4ThreeVector& Momentum, const G4ThreeVector& Normal ) const
            // https://apc.u-paris.fr/~franco/g4doxy4.10/html/_g4_op_boundary_process_8cc_source.html
            // line 620

            /* This function code alpha to a random value taken from the
               distribution p(alpha) = g(alpha; 0, sigma_alpha)*std::sin(alpha),
               for alpha > 0 and alpha < 90, where g(alpha; 0, sigma_alpha)
               is a gaussian distribution with mean 0 and standard deviation
               sigma_alpha.  */

            if (SurfaceSettings.SigmaAlpha == 0.0)
            {
                //FacetNormal = Normal;
                for (int i = 0; i < 3; i++) LocalNormal[i] = globalNormal[i];
                break;
            }

            //G4ThreeVector tmpNormal = Normal;
            const TVector3 tmpNormal(globalNormal);
            TVector3 FacetNormal;
            const double f_max = std::min(1.0, 4.0 * SurfaceSettings.SigmaAlpha);
            double alpha, nk;
            do
            {
                //do alpha = G4RandGauss::shoot(0.0, SurfaceSettings.SigmaAlpha);
                //while (G4UniformRand() * f_max > std::sin(alpha) || alpha >= halfpi );
                do alpha = RandomHub.gauss(0.0, SurfaceSettings.SigmaAlpha);
                while (RandomHub.uniform() * f_max > sin(alpha) || alpha >= 0.5*3.1415926535 );

                //double phi = G4UniformRand() * twopi;
                const double phi = RandomHub.uniform() * 2.0*3.1415926535;

                const double SinAlpha = sin(alpha);
                const double CosAlpha = cos(alpha);
                const double SinPhi   = sin(phi);
                const double CosPhi   = cos(phi);

                const double unit_x = SinAlpha * CosPhi;
                const double unit_y = SinAlpha * SinPhi;
                const double unit_z = CosAlpha;

                FacetNormal.SetX(unit_x);
                FacetNormal.SetY(unit_y);
                FacetNormal.SetZ(unit_z);

                //G4ThreeVector tmpNormal = Normal;

                FacetNormal.RotateUz(tmpNormal);

                nk = 0;
                for (int i = 0; i < 3; i++) nk += photonDirection[i] * FacetNormal[i];
            }
            //while (Momentum * FacetNormal >= 0.0);
            while (nk <= 0.0);

            for (int i = 0; i < 3; i++) LocalNormal[i] = FacetNormal[i];

            break;
        }
    case ASurfaceSettings::CustomNormal :
    {
        if (!SurfaceSettings.NormalDistributionHist)
        {
            for (int i = 0; i < 3; i++) LocalNormal[i] = globalNormal[i];
            break;
        }

        const TVector3 tmpNormal(globalNormal);
        TVector3 FacetNormal;
        double nk;
        do
        {
            double alpha = SurfaceSettings.NormalDistributionHist->GetRandom();

            const double phi = RandomHub.uniform() * 2.0*3.1415926535;

            const double SinAlpha = sin(alpha);
            const double CosAlpha = cos(alpha);
            const double SinPhi   = sin(phi);
            const double CosPhi   = cos(phi);

            const double unit_x = SinAlpha * CosPhi;
            const double unit_y = SinAlpha * SinPhi;
            const double unit_z = CosAlpha;

            FacetNormal.SetX(unit_x);
            FacetNormal.SetY(unit_y);
            FacetNormal.SetZ(unit_z);

            FacetNormal.RotateUz(tmpNormal);

            nk = 0;
            for (int i = 0; i < 3; i++) nk += photonDirection[i] * FacetNormal[i];
        }
        while (nk <= 0.0);

        for (int i = 0; i < 3; i++) LocalNormal[i] = FacetNormal[i];

        break;
    }
    default:;
    }

    //qDebug() << "localNorm:"  << LocalNormal[0] << ' ' << LocalNormal[1] << ' ' << LocalNormal[2];
}
