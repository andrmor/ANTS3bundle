#include "asurfaceinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "aphotonstatistics.h"
#include "ajsontools.h"
#include "arandomhub.h"

#include <QJsonObject>

#include "TMath.h"
#include "TRandom2.h"

AInterfaceRule::OpticalOverrideResultEnum ASurfaceInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    return Absorbed;
}

QString ASurfaceInterfaceRule::getReportLine() const
{
    //return QString("Albedo %1").arg(Albedo);
    return "Test";
}

QString ASurfaceInterfaceRule::getLongReportLine() const
{
    QString s = "--> Surface <--\n";
    //s += QString("Albedo: %1").arg(Albedo);
    return s;
}

void ASurfaceInterfaceRule::writeToJson(QJsonObject & json) const
{
    AInterfaceRule::writeToJson(json);

    //json["Albedo"] = Albedo;
}

bool ASurfaceInterfaceRule::readFromJson(const QJsonObject & json)
{
    return true; //jstools::parseJson(json, "Albedo", Albedo);
}

QString ASurfaceInterfaceRule::checkOverrideData()
{
    return "";
}
