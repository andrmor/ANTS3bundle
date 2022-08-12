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
            do
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
            while (scal < 0);

            break;
        }
//    case ASurfaceSettings::Model3 :
//        {

//            break;
//        }
    default:;
    }

    qDebug() << "localNorm:"  << LocalNormal[0] << ' ' << LocalNormal[1] << ' ' << LocalNormal[2];
}
