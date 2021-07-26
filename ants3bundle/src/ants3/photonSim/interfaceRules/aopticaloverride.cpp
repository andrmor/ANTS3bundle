#include "aopticaloverride.h"
#include "aphotsimsettings.h"

#include <QDebug>
#include <QJsonObject>

#ifdef GUI
#include <QFrame>
#endif

AOpticalOverride::AOpticalOverride(A3MatHub *MatCollection, int MatFrom, int MatTo) :
    SimSet(APhotSimSettings::getConstInstance()), WaveSet(SimSet.WaveSet),
    MatCollection(MatCollection), MatFrom(MatFrom), MatTo(MatTo) {}

void AOpticalOverride::writeToJson(QJsonObject &json) const
{
    json["Model"] = getType();
    json["MatFrom"] = MatFrom;
    json["MatTo"] = MatTo;
}

bool AOpticalOverride::readFromJson(const QJsonObject &)
{
    return true;
}

#ifdef GUI
QWidget *AOpticalOverride::getEditWidget(QWidget *, GraphWindowClass *)
{
    QFrame* f = new QFrame();
    f->setFrameStyle(QFrame::Box);
    f->setMinimumHeight(100);
    return f;
}
#endif

#include "abasicopticaloverride.h"
#include "spectralbasicopticaloverride.h"
#include "fsnpopticaloverride.h"
#include "awaveshifteroverride.h"
#include "scatteronmetal.h"
#include "ascriptopticaloverride.h"

AOpticalOverride *OpticalOverrideFactory(QString model, A3MatHub *MatCollection, int MatFrom, int MatTo)
{
    if (model == "Simplistic" || model == "Simplistic_model")
        return new ABasicOpticalOverride(MatCollection, MatFrom, MatTo);
    if (model == "SimplisticSpectral" || model == "SimplisticSpectral_model")
        return new SpectralBasicOpticalOverride(MatCollection, MatFrom, MatTo);
    else if (model == "DielectricToMetal")
        return new ScatterOnMetal(MatCollection, MatFrom, MatTo);
    else if (model == "FSNP" || model == "FS_NP" || model=="Neves_model")
        return new FSNPOpticalOverride(MatCollection, MatFrom, MatTo);
    else if (model == "SurfaceWLS")
        return new AWaveshifterOverride(MatCollection, MatFrom, MatTo);
    else if (model == "CustomScript")
        return new AScriptOpticalOverride(MatCollection, MatFrom, MatTo);

    return nullptr; //undefined override type!
}

const QStringList ListOvAllOpticalOverrideTypes()
{
    QStringList l;

    l << "Simplistic"
      << "SimplisticSpectral"
      << "FSNP"
      << "DielectricToMetal"
      << "ClaudioModel"
      << "SurfaceWLS"
      << "CustomScript";

    return l;
}
