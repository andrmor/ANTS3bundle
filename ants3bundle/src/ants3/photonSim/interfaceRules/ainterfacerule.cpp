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

#include <QDebug>
#include <QJsonObject>

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
      << "RoughSurface";

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
    if (isNotPolishedSurface() && !canHaveRoughSurface()) return "This interface rule type cannot have rough optical surface";

    return doCheckOverrideData();
}

#include "TVector3.h"
void AInterfaceRule::calculateLocalNormal(const double * globalNormal, const double * photonDirection)
{
    qDebug() << (int)SurfaceSettings.Model;
    qDebug() << "globNorm:" << globalNormal[0] << ' ' << globalNormal[1] << ' ' << globalNormal[2];
    qDebug() << "photDir:"  << photonDirection[0] << ' ' << photonDirection[1] << ' ' << photonDirection[2];

    switch (SurfaceSettings.Model)
    {
    case ASurfaceSettings::Polished :
        // for Polished, it should not be called at all!
        for (int i = 0; i < 3; i++) LocalNormal[i] = globalNormal[i];
        break;
    case ASurfaceSettings::GaussSimplistic :
        {
            TVector3 gn(globalNormal);
            TVector3 ort = gn.Orthogonal();

            double scal = 0;
            //do
            {
                TVector3 vec(gn);

                double rand = RandomHub.gauss(0, 15.0);
                vec.Rotate(rand * 3.1415926/180.0, ort);
                vec.Rotate(RandomHub.uniform() * 2.0*3.1415926, gn);

                scal = 0;
                for (int i = 0; i < 3; i++)
                {
                    LocalNormal[i] = vec[i];
                    scal += LocalNormal[i] * photonDirection[i];
                }
                qDebug() << "nk" << scal;
            }
            //while (scal < 0);

            break;
        }
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
                while (nk >= 0.0);

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

/*
            G4ThreeVector FacetNormal;
            double alpha;
            double sigma_alpha = 0.0;
            if (OpticalSurface) sigma_alpha = OpticalSurface->GetSigmaAlpha();
            if (sigma_alpha == 0.0) return FacetNormal = Normal;

            double f_max = std::min(1.0, 4.0 * sigma_alpha);
            do
            {
                do alpha = G4RandGauss::shoot(0.0, sigma_alpha);
                while (G4UniformRand() * f_max > std::sin(alpha) || alpha >= halfpi );

                double phi = G4UniformRand() * twopi;

                double SinAlpha = std::sin(alpha);
                double CosAlpha = std::cos(alpha);
                double SinPhi   = std::sin(phi);
                double CosPhi   = std::cos(phi);

                double unit_x = SinAlpha * CosPhi;
                double unit_y = SinAlpha * SinPhi;
                double unit_z = CosAlpha;

                FacetNormal.setX(unit_x);
                FacetNormal.setY(unit_y);
                FacetNormal.setZ(unit_z);

                G4ThreeVector tmpNormal = Normal;

                FacetNormal.rotateUz(tmpNormal);
            }
            while (Momentum * FacetNormal >= 0.0);
*/
            break;
        }
    default:;
    }

    qDebug() << "localNorm:"  << LocalNormal[0] << ' ' << LocalNormal[1] << ' ' << LocalNormal[2];
}
