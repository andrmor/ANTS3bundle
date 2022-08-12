#include "asurfaceinterfacerule.h"
#include "aphoton.h"
#include "amaterial.h"
#include "amaterialhub.h"
#include "aphotonstatistics.h"
#include "ajsontools.h"
#include "arandomhub.h"
#include "asurfacesettings.h" // !!!*** tmp

#include <QJsonObject>
#include <QDebug>

#include "TMath.h"
#include "TRandom2.h"

ASurfaceInterfaceRule::ASurfaceInterfaceRule(int MatFrom, int MatTo)
    : AInterfaceRule(MatFrom, MatTo)
{
    SurfaceSettings.Model = ASurfaceSettings::GaussSimplistic; // !!!***
}

AInterfaceRule::OpticalOverrideResultEnum ASurfaceInterfaceRule::calculate(APhoton * Photon, const double * NormalVector)
{
    qDebug() << "Surface rule triggered";

    calculateLocalNormal(NormalVector, Photon->v);

    return DelegateLocalNormal;
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

void ASurfaceInterfaceRule::doWriteToJson(QJsonObject & json) const
{
    //json["Albedo"] = Albedo;
}

bool ASurfaceInterfaceRule::doReadFromJson(const QJsonObject & json)
{
    return true; //jstools::parseJson(json, "Albedo", Albedo);
}

QString ASurfaceInterfaceRule::doCheckOverrideData()
{
    if (isPolishedSurface()) return "Polished surface cannot be used with this interface type!";
    return "";
}
