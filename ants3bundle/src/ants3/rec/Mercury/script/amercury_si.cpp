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

#include "TAxis.h"
void AMercury_si::plot(QString what, int bins, double from, double to)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    EPlotOption opt = whatFromString(what);
    if (opt == ErrorOption)
    {
        abort("Mercury unit offers the following plot options:\n" + PlotOptions);
        return;
    }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<double> & energy = RecMP->rec_e;

    TH1D * h = nullptr;

    switch (opt)
    {
    case EnergyOption:
        h = new TH1D("", "energy", bins, from, to);
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            h->Fill(energy[i], 1);
        }
        break;
    case Chi2Option:
        h = new TH1D("", "chi2", bins, from, to);
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;

            h->Fill(chi2[i], 1);
        }
        break;
    case StatusOption:
        h = new TH1D("", "status", 2, 0, 2);
        for (size_t i = 0; i < status.size(); i++)
            h->Fill( (status[i] == 0 ? 0 : 1), 1);
        {
            TAxis * ax = h->GetXaxis();
            ax->SetNdivisions(4, false);
            ax->ChangeLabelByValue(0, -1, -1, -1, -1, -1, " ");
            ax->ChangeLabelByValue(0.5, -1, -1, -1, -1, -1, "Good");
            ax->ChangeLabelByValue(1.0, -1, -1, -1, -1, -1, " ");
            ax->ChangeLabelByValue(1.5, -1, -1, -1, -1, -1, "Fail");
            ax->ChangeLabelByValue(2.0, -1, -1, -1, -1, -1, " ");
        }
        break;
    case DensityOption:
        abort("Density option is only applicable for 2D plots");
        break;
    default:
        break;
    }

    if (h) emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::plot_vsRecXY(QString what, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    EPlotOption opt = whatFromString(what);
    if (opt == ErrorOption)
    {
        abort("Mercury unit offers the following plot options:\n" + PlotOptions);
        return;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    doPlot_vsXY(false, opt, xBins, xFrom, xTo, yBins, yFrom, yTo, x, y);
}

void AMercury_si::plot_vsTrueXY(QString what, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo, QVariantList truePositions)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    EPlotOption opt = whatFromString(what);
    if (opt == ErrorOption)
    {
        abort("Mercury unit offers the following plot options:\n" + PlotOptions);
        return;
    }

    size_t num = truePositions.size();
    if (num == 0)
    {
        abort("TruePositions array is empty");
        return;
    }
    if (num != RecMP->rec_x.size())
    {
        abort("TruePositions array size does not match reconstruction data size");
        return;
    }
    std::vector<double> x(num), y(num);

    for (size_t i = 0; i < num; i++)
    {
        QVariantList el = truePositions[i].toList();
        if (el.size() < 2)
        {
            abort("TruePositions array should contain arrays of at least size two (X and Y positions)");
            return;
        }
        x[i] = el[0].toDouble();
        y[i] = el[1].toDouble();
    }

    doPlot_vsXY(true, opt, xBins, xFrom, xTo, yBins, yFrom, yTo, x, y);
}

void AMercury_si::doPlot_vsXY(bool vsTrue, EPlotOption opt, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo,
                              const std::vector<double> & x, const std::vector<double> & y)
{
    if (!vsTrue)
        if (opt == BiasXOption || opt == BiasYOption || opt == SigmaXOption || opt == SigmaYOption)
        {
            abort("Bias and Sigma options are available only for 2D plots vs true positions");
            return;
        }

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<double> & energy = RecMP->rec_e;
    const std::vector<double> & recX   = RecMP->rec_x;
    const std::vector<double> & recY   = RecMP->rec_y;

    TH2D * h = new TH2D("", "", xBins, xFrom, xTo, yBins, yFrom, yTo);
    QString title;

    // !!!*** TODO normalization (TH2D * hNorm = ...)

    switch (opt)
    {
    case EnergyOption:
        title = "energy_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;

            h->Fill(x[i], y[i], energy[i]);
        }
        break;
    case Chi2Option:
        title = "chi2_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;

            h->Fill(x[i], y[i], chi2[i]);
        }
        break;
    case StatusOption:
        title = "status_";
        for (size_t i = 0; i < status.size(); i++)
            h->Fill(x[i], y[i], (status[i] == 0 ? 0 : 1));
        break;
    case DensityOption:
        title = "density_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            h->Fill(x[i], y[i], 1);
        }
        break;
    case BiasXOption:
        title = "biasX_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            h->Fill(x[i], y[i], recX[i] - x[i]);
        }
        break;
    case BiasYOption:
        title = "biasY_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            h->Fill(x[i], y[i], recY[i] - y[i]);
        }
        break;
    case SigmaXOption:
        title = "sigmaX_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            const double delta = recX[i] - x[i];
            h->Fill(x[i], y[i], delta*delta);
        }
        break;
    case SigmaYOption:
        title = "sigmaY_";
        for (size_t i = 0; i < status.size(); i++)
        {
            if (status[i] != 0) continue;
            const double delta = recY[i] - y[i];
            h->Fill(x[i], y[i], delta*delta);
        }
        break;
    default:
        break;
    }

    if (h)
    {
        title += (vsTrue ? "trueXY" : "recXY");
        h->SetTitle(title.toLatin1().data());
        emit AScriptHub::getInstance().requestDraw(h, "colz", true);
    }
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

AMercury_si::EPlotOption AMercury_si::whatFromString(QString what)
{
    what = what.toUpper();

    if (what == "ENERGY")  return EnergyOption;
    if (what == "CHI2")    return Chi2Option;
    if (what == "STATUS")  return StatusOption;
    if (what == "DENSITY") return DensityOption;
    if (what == "BIASX")   return BiasXOption;
    if (what == "BIASY")   return BiasYOption;
    if (what == "SIGMAX")  return SigmaXOption;
    if (what == "SIGMAY")  return SigmaYOption;

    return ErrorOption;
}
