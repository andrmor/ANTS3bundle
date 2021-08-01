#include "ainterfacerule.h"
#include "aphotonsimhub.h"
#include "amaterialhub.h"

#include <QDebug>
#include <QJsonObject>

#ifdef GUI
#include <QFrame>
#endif

AInterfaceRule::AInterfaceRule(int MatFrom, int MatTo) :
    MatFrom(MatFrom), MatTo(MatTo) {}

QString AInterfaceRule::getLongReportLine() const
{
    return getReportLine();
}

void AInterfaceRule::writeToJson(QJsonObject &json) const
{
    json["Model"]   = getType();
    json["MatFrom"] = MatFrom;
    json["MatTo"]   = MatTo;
}

bool AInterfaceRule::readFromJson(const QJsonObject &)
{
    return true;
}

#ifdef GUI
QWidget *AInterfaceRule::getEditWidget(QWidget *, GraphWindowClass *)
{
    QFrame* f = new QFrame();
    f->setFrameStyle(QFrame::Box);
    f->setMinimumHeight(100);
    return f;
}
#endif

#include "abasicinterfacerule.h"
#include "aspectralbasicinterfacerule.h"
#include "fsnpinterfacerule.h"
#include "awaveshifterinterfacerule.h"
#include "ametalinterfacerule.h"

AInterfaceRule * interfaceRuleFactory(const QString & Model, int MatFrom, int MatTo)
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

    return nullptr; //undefined override type!
}

QStringList getAllInterfaceRuleTypes()
{
    QStringList l;

    l << "Simplistic"
      << "SimplisticSpectral"
      << "FSNP"
      << "DielectricToMetal"
      << "SurfaceWLS";

    return l;
}
