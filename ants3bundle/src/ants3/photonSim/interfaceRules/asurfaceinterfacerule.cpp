#include "asurfaceinterfacerule.h"
#include "aphoton.h"
//#include "amaterial.h"
//#include "amaterialhub.h"
#include "aphotonstatistics.h"
//#include "ajsontools.h"
//#include "arandomhub.h"
#include "asurfacesettings.h"

#include <QJsonObject>
#include <QDebug>

ASurfaceInterfaceRule::ASurfaceInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::Glisur;
}

AInterfaceRule::OpticalOverrideResultEnum ASurfaceInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    calculateLocalNormal(NormalVector, Photon->v);
    Status = LocalNormalDelegated;
    return DelegateLocalNormal;
}

QString ASurfaceInterfaceRule::getReportLine() const
{
    return "";
}

QString ASurfaceInterfaceRule::getLongReportLine() const
{
    QString s = "--> Rough surface <--\n";
    return s;
}

QString ASurfaceInterfaceRule::getDescription() const
{
    return "";
}

void ASurfaceInterfaceRule::doWriteToJson(QJsonObject & /*json*/) const {}

bool ASurfaceInterfaceRule::doReadFromJson(const QJsonObject & /*json*/)
{
    return true;
}

QString ASurfaceInterfaceRule::doCheckOverrideData()
{
    if (isPolishedSurface()) return "Polished surface cannot be used with this interface type!";
    return "";
}
