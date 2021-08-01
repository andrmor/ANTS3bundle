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

void AInterfaceRule::writeToJson(QJsonObject &json) const
{
    json["Model"] = getType();
    json["MatFrom"] = MatFrom;
    json["MatTo"] = MatTo;
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
#include "spectralbasicopticaloverride.h"
#include "fsnpopticaloverride.h"
#include "awaveshifteroverride.h"
#include "scatteronmetal.h"
#include "ascriptopticaloverride.h"

AInterfaceRule * interfaceRuleFactory(const QString &model, int MatFrom, int MatTo)
{
    if (model == "Simplistic" || model == "Simplistic_model")
        return new ABasicInterfaceRule(MatFrom, MatTo);
    if (model == "SimplisticSpectral" || model == "SimplisticSpectral_model")
        return new SpectralBasicOpticalOverride(MatFrom, MatTo);
    else if (model == "DielectricToMetal")
        return new ScatterOnMetal(MatFrom, MatTo);
    else if (model == "FSNP" || model == "FS_NP" || model=="Neves_model")
        return new FSNPOpticalOverride(MatFrom, MatTo);
    else if (model == "SurfaceWLS")
        return new AWaveshifterOverride(MatFrom, MatTo);
//    else if (model == "CustomScript")
//        return new AScriptOpticalOverride(MatFrom, MatTo);

    return nullptr; //undefined override type!
}

const QStringList getAllInterfaceRuleTypes()
{
    QStringList l;

    l << "Simplistic"
      << "SimplisticSpectral"
      << "FSNP"
      << "DielectricToMetal"
      << "SurfaceWLS";
//      << "CustomScript";

    return l;
}
