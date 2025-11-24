#include "amercury_si.h"
#include "alightresponsehub.h"
#include "reconstructor_mp.h"
#include "ascripthub.h"

#include <QVariant>

#include "TH1D.h"
#include "TH2D.h"

AMercury_si::AMercury_si() :
    LRHub(ALightResponseHub::getInstance()) {}

void AMercury_si::newReconstructor(QString type, int numThreads)
{
    if (!LRHub.Model)
    {
        abort("LRF model was not created yet");
        return;
    }

    if (numThreads < 1)
    {
        abort("The numThread argument of newReconstructor method should be at least 1");
        return;
    }

    resetReconstructor();

    if      (type == "COG") RecMP = new ReconstructorMP(LRHub.Model, numThreads);
    else if (type == "ML")  RecMP = new RecML_MP(LRHub.Model, numThreads);
    else if (type == "LS")  RecMP = new RecLS_MP(LRHub.Model, numThreads);
    else
    {
        abort("The method newReconstructor should have a type of 'COG', 'ML' or 'LS'");
        return;
    }
}

void AMercury_si::reconstructEvents(QVariantList sensorSignalsOverAllEvents)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const size_t numEvents = sensorSignalsOverAllEvents.size();
    if (numEvents == 0)
    {
        abort("The array with events for reconstructEvents is empty");
        return;
    }

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

    RecMP->ProcessEvents(A);
}

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

QVariantList AMercury_si::getRecStats()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<double> & cov_xx = RecMP->cov_xx;
    const std::vector<double> & cov_yy = RecMP->cov_yy;
    const std::vector<double> & cov_xy = RecMP->cov_xy;

    const size_t size = status.size();
    if (size != chi2.size() || size != cov_xx.size() || size != cov_yy.size() || size != cov_xy.size())
    {
        abort("Mismatch in status array sizes");
        return res;
    }

    for (size_t i = 0; i < size; i++)
        res.emplaceBack(QVariantList{status[i], chi2[i], cov_xx[i], cov_yy[i], cov_xy[i]});
    return res;
}

void AMercury_si::plotChi2(int bins, double from, double to)
{
    if (!LRHub.Model)
    {
        abort("Light response model is not defined");
        return;
    }
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;

    TH1D * h = new TH1D("", "chi2", bins, from, to);
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;

        h->Fill(chi2[i], 1);
    }

    emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::plotChi2_XY(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo)
{
    if (!LRHub.Model)
    {
        abort("Light response model is not defined");
        return;
    }
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<double> & x      = RecMP->rec_x;
    const std::vector<double> & y      = RecMP->rec_y;

    TH2D * h = new TH2D("", "chi2_xy", xBins, xFrom, xTo, yBins, yFrom, yTo);
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;

        h->Fill(x[i], y[i], chi2[i]);
    }

    emit AScriptHub::getInstance().requestDraw(h, "colz", true);
}


void AMercury_si::setCOG_AbsCutoff(double val)
{
    if (RecMP) RecMP->setCogAbsCutoff(val);
    else abort("Reconstructor was not created yet");
}

void AMercury_si::setCOG_RelCutoff(double val)
{
    if (RecMP) RecMP->setCogRelCutoff(val);
    else abort("Reconstructor was not created yet");
}

void AMercury_si::setCutoffRadius(double val)
{
    if (RecMP) RecMP->setRecCutoffRadius(val);
    else abort("Reconstructor was not created yet");
}

void AMercury_si::setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls)
{
    RecMinuitMP * rec = dynamic_cast<RecMinuitMP*>(RecMP);
    if (!rec)
    {
        abort("Reconstructor not created or it is not 'ML' or 'LS' type");
        return;
    }
    rec->setRMtolerance(RMtolerance);
    rec->setRMmaxIterations(RMmaxIterations);
    rec->setRMmaxFuncCalls(RMmaxFuncCalls);
}

// --- Private methods ---

void AMercury_si::resetReconstructor()
{
    delete RecMP; RecMP = nullptr;
}

