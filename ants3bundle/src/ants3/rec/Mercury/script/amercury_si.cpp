#include "amercury_si.h"
#include "reconstructor.h"
#include "reconstructor_mp.h"
#include "lrmodel.h"

#include <QVariant>

AMercury_si::AMercury_si() {}

/*
void AMercury_si::createReconstructor_CoG()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecCoG(Model);
}

void AMercury_si::createReconstructor_LS()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecLS(Model);
}

void AMercury_si::createReconstructor_ML()
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    Rec = new RecML(Model);
}
*/

void AMercury_si::createReconstructor_COG_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new ReconstructorMP(Model, numThreads);
}

void AMercury_si::createReconstructor_LS_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new RecLS_MP(Model, numThreads);
}

void AMercury_si::createReconstructor_ML_multi(int numThreads)
{
    if (!Model)
    {
        abort("LRF model was not created yet!");
        return;
    }

    resetReconstructors();
    RecMP = new RecML_MP(Model, numThreads);
}

/*
void AMercury_si::reconstructEvent(QVariantList sensSignals)
{
    const size_t numEl = sensSignals.size();
    std::vector<double> a(numEl);
    std::vector<bool>   sat(numEl, false);

    for (size_t i = 0; i < numEl; i++)
        a[i] = sensSignals[i].toDouble();

    if (Rec) Rec->ProcessEvent(a, sat);
    else     abort("Reconstructor was not yet created");
}
*/

void AMercury_si::reconstructEvents(QVariantList sensorSignalsOverAllEvents)
{
    const size_t numEvents = sensorSignalsOverAllEvents.size();

    std::vector<std::vector<double>> A(numEvents);

    size_t numEl = 0;

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList sensSignals = sensorSignalsOverAllEvents[iEv].toList();
        numEl = sensSignals.size();

        A[iEv].resize(numEl);
        for (size_t i = 0; i < numEl; i++)
            A[iEv][i] = sensSignals[i].toDouble();
    }

    if (RecMP) RecMP->ProcessEvents(A);
    else     abort("Reconstructor was not yet created");
}

/*
double AMercury_si::getPositionX()
{
    if (Rec) return Rec->getRecX();

    abort("Reconstructor was not yet created");
    return 0;
}

double AMercury_si::getPositionY()
{
    if (Rec) return Rec->getRecY();

    abort("Reconstructor was not yet created");
    return 0;
}
*/

QVariantList AMercury_si::getRecXYZ()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    const std::vector<double> & z = RecMP->rec_z;

    const size_t size = x.size();
    if (size != y.size() || size != z.size())
    {
        abort("Mismatch in xyz array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{x[i], y[i], z[i]});
    return res;
}

QVariantList AMercury_si::getRecXYZE()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    const std::vector<double> & z = RecMP->rec_z;
    const std::vector<double> & e = RecMP->rec_e;

    const size_t size = x.size();
    if (size != y.size() || size != z.size() || size != e.size())
    {
        abort("Mismatch in xyze array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{x[i], y[i], z[i], e[i]});
    return res;
}

void AMercury_si::setCOG_AbsCutoff(double val)
{
    if (Rec) Rec->setCogAbsCutoff(val);
    else abort("Reconstructor was not yet created");
}

void AMercury_si::setCOG_RelCutoff(double val)
{
    if (Rec) Rec->setCogRelCutoff(val);
    else abort("Reconstructor was not yet created");
}

// --- LRFs ---

void AMercury_si::newLightResponseModel(int numSensors)
{
    delete Model;
    Model = new LRModel(numSensors);
}

void AMercury_si::addSensor(int iSensor, double x, double y)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    if (Model->SensorExists(iSensor)) Model->AddSensor(iSensor, x, y);
    else abort("Sensor index is not valid");
}

void AMercury_si::setLRF(int iSensor, QString jsonString)
{
    if (!Model)
    {
        abort("Model was not created yet!");
        return;
    }

    Model->SetJsonLRF(iSensor, jsonString.toLatin1().data());
}

QString AMercury_si::exportLightResponseModel()
{
    QString res;
    if (Model) res = Model->GetJsonString().data();
    return res;
}

void AMercury_si::importLightResponseModel(QString jsonStr)
{
    delete Model; Model = nullptr;
    Model = new LRModel(jsonStr.toLatin1().data());
}

void AMercury_si::clearAllFitData()
{
    if (Model) Model->ClearAllFitData();
}

void AMercury_si::addFitData(int iSensor, QVariantList xyza)
{
    if (!Model) return;

    const size_t size = xyza.size();
    std::vector<std::array <double, 4>> data(size);
    for (size_t iEv = 0; iEv < size; iEv++)
    {
        QVariantList event = xyza[iEv].toList();
        for (size_t i = 0; i < 4; i++)
            data[iEv][i] = event[i].toDouble();
    }

    Model->AddFitData(iSensor, data);
}

void AMercury_si::fitSensor(int iSensor)
{
    if (Model) Model->FitSensor(iSensor);
}

#include "alrfdrawer.h"
void AMercury_si::plotLRF_radial(int iSensor, bool showNodes)
{
    ALrfDrawer dr(Model);
    QString err = dr.drawRadial(iSensor, showNodes);
    if (!err.isEmpty()) abort(err);
}

#include "ascripthub.h"
void AMercury_si::showLightResponseExplorer()
{
    if (!Model) abort("Light response model is not defined");
    else emit AScriptHub::getInstance().requestShowLightResponseExplorer(Model);
}

double AMercury_si::eval(int iSensor, double x, double y, double z)
{
    if (Model) return Model->Eval(iSensor, x, y, z);
    else return 0;
}

double AMercury_si::eval(int iSensor, QVariantList xyz)
{
    if (xyz.length() != 3) return 0;

    double pos[3];
    for (size_t i = 0; i < 3; i++) pos[i] = xyz[i].toDouble();

    if (Model) return Model->Eval(iSensor, pos);
    else return 0;
}

void AMercury_si::resetReconstructors()
{
    delete Rec;   Rec   = nullptr;
    delete RecMP; RecMP = nullptr;
}

