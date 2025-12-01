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
    const std::vector<int>    & dof    = RecMP->rec_dof;
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
    {
        if (status[i] == 0) res.emplaceBack(QVariantList{status[i], chi2[i] / dof[i], cov_xx[i], cov_yy[i], cov_xy[i]});
        else                res.emplaceBack(QVariantList{status[i], 0,                0,         0,         0});
    }
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
        abort("Valid options for Mercury.plot are: Status, Energy, Chi2 and All");
        return;
    }

    switch (opt)
    {
    case EnergyOption:
        plotEnergyHist(bins, from, to);
        break;
    case Chi2Option:
        plotChi2Hist(bins, from, to);
        break;
    case StatusOption:
        plotStatusHist();
        break;
    case EachValidOption:
        plotStatusHist();
        emit AScriptHub::getInstance().requestAddToBasket("Status");
        plotChi2Hist(bins, from, to);
        emit AScriptHub::getInstance().requestAddToBasket("Chi2");
        plotEnergyHist(bins, from, to);
        emit AScriptHub::getInstance().requestAddToBasket("Energy");
        break;
    default:
        abort("Not valid 'what' option for mercury.plot\nUse one of the following: Energy, Chi2, Status and All");
        break;
    }
}

void AMercury_si::plotEnergyHist(int bins, double from, double to)
{
    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & energy = RecMP->rec_e;

    TH1D * h = new TH1D("", "energy", bins, from, to);
    h->GetXaxis()->SetTitle("Energy");
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        h->Fill(energy[i], 1);
    }
    emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::plotChi2Hist(int bins, double from, double to)
{
    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;
    const std::vector<int>    & dof    = RecMP->rec_dof;

    TH1D * h = new TH1D("", "chi2", bins, from, to);
    h->GetXaxis()->SetTitle("Chi2");
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        h->Fill(chi2[i] / dof[i], 1);
    }
    emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::plotStatusHist()
{
    const std::vector<int> & status = RecMP->rec_status;

    TH1D * h = new TH1D("", "status", 2, 0, 2);
    for (size_t i = 0; i < status.size(); i++)
        h->Fill( (status[i] == 0 ? 0 : 1), 1);

    TAxis * ax = h->GetXaxis();
    ax->SetNdivisions(4, false);
    ax->ChangeLabelByValue(0,   -1, -1, -1, -1, -1, " ");
    ax->ChangeLabelByValue(0.5, -1, -1, -1, -1, -1, "Good");
    ax->ChangeLabelByValue(1.0, -1, -1, -1, -1, -1, " ");
    ax->ChangeLabelByValue(1.5, -1, -1, -1, -1, -1, "Fail");
    ax->ChangeLabelByValue(2.0, -1, -1, -1, -1, -1, " ");
    emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::configure_plotXY_binning(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo)
{
    XBins = xBins;
    YBins = yBins;

    XFrom = xFrom;
    XTo   = xTo;
    YFrom = yFrom;
    YTo   = yTo;
}

void AMercury_si::plot_vsRecXY(QString what)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    EPlotOption opt = whatFromString(what);
    if (opt == ErrorOption)
    {
        abort("Valid options for Mercury.plot_vsRecXY are: Status, Energy, Chi2, Density and All");
        return;
    }

    const std::vector<double> & x = RecMP->rec_x;
    const std::vector<double> & y = RecMP->rec_y;
    doPlot_vsXY(false, opt, x, y);
}

void AMercury_si::configure_plotXY_truePositions(QVariantList truePositions)
{
    const size_t num = truePositions.size();
    if (num == 0)
    {
        abort("TruePositions array is empty");
        return;
    }

    XTruePositions.resize(num);
    YTruePositions.resize(num);
    for (size_t i = 0; i < num; i++)
    {
        QVariantList el = truePositions[i].toList();
        if (el.size() < 2)
        {
            abort("TruePositions array should contain arrays of at least size two (X and Y positions)");
            return;
        }
        bool ok1, ok2;
        XTruePositions[i] = el[0].toDouble(&ok1);
        YTruePositions[i] = el[1].toDouble(&ok2);
        if (ok1 && ok2) continue;
        abort("Bad x or y value format in TruePositions array");
        return;
    }
}

void AMercury_si::plot_vsTrueXY(QString what)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    EPlotOption opt = whatFromString(what);
    if (opt == ErrorOption)
    {
        abort("Valid options for Mercury.plot_vsTrueXY are: Status, Energy, Chi2, Density, BiasX, BiasY, ErrorX, ErrorY and All");
        return;
    }

    size_t num = XTruePositions.size();
    if (num == 0)
    {
        abort("TruePositions array is empty");
        return;
    }
    if (num != RecMP->rec_x.size())
    {
        abort("TruePositions array size does not match reconstruction data size.\nDid you call mercury.configure_plotXY_truePositions() first?");
        return;
    }

    doPlot_vsXY(true, opt, XTruePositions, YTruePositions);
}

void AMercury_si::doPlot_vsXY(bool vsTrue, EPlotOption opt, const std::vector<double> & x, const std::vector<double> & y)
{
    if (!vsTrue)
        if (opt == BiasXOption || opt == BiasYOption || opt == ErrorXOption || opt == ErrorYOption)
        {
            abort("Bias and Error options are available only for 2D plots vs true positions");
            return;
        }

    QString titleSuffix = (vsTrue ? "_trueXY" : "_recXY");
    switch (opt)
    {
    case EnergyOption:
        plotEnergyXYHist(x, y, titleSuffix);
        break;
    case Chi2Option:
        plotChi2XYHist(x, y, titleSuffix);
        break;
    case StatusOption:
        plotStatusXYHist(x, y, titleSuffix);
        break;
    case DensityOption:
        plotDensityXYHist(x, y, titleSuffix);
        break;
    case BiasXOption:
        plotBiasXYHist(x, y, titleSuffix, true);
        break;
    case BiasYOption:
        plotBiasXYHist(x, y, titleSuffix, false);
        break;
    case ErrorXOption:
        plotSigmaXYHist(x, y, titleSuffix, true);
        break;
    case ErrorYOption:
        plotSigmaXYHist(x, y, titleSuffix, false);
        break;
    case EachValidOption:
        plotStatusXYHist(x, y, titleSuffix);  emit AScriptHub::getInstance().requestAddToBasket("Status" + titleSuffix);
        plotChi2XYHist(x, y, titleSuffix);    emit AScriptHub::getInstance().requestAddToBasket("Chi2" + titleSuffix);
        plotDensityXYHist(x, y, titleSuffix); emit AScriptHub::getInstance().requestAddToBasket("Density" + titleSuffix);
        plotEnergyXYHist(x, y, titleSuffix);  emit AScriptHub::getInstance().requestAddToBasket("Energy" + titleSuffix);
        if (vsTrue)
        {
            plotBiasXYHist(x, y, titleSuffix, true);   emit AScriptHub::getInstance().requestAddToBasket("BiasX" + titleSuffix);
            plotBiasXYHist(x, y, titleSuffix, false);  emit AScriptHub::getInstance().requestAddToBasket("BiasY" + titleSuffix);
            plotSigmaXYHist(x, y, titleSuffix, true);  emit AScriptHub::getInstance().requestAddToBasket("ErrorX" + titleSuffix);
            plotSigmaXYHist(x, y, titleSuffix, false); emit AScriptHub::getInstance().requestAddToBasket("ErrorY" + titleSuffix);
        }
        break;
    default:
        break;
    }
}

TH2D * AMercury_si::create2Dhist()
{
    TH2D * hist = new TH2D("", "", XBins, XFrom, XTo, YBins, YFrom, YTo);  // used for normalization
    hist->GetXaxis()->SetTitle("X, mm");
    hist->GetYaxis()->SetTitle("Y, mm");
    return hist;
}

void AMercury_si::plotEnergyXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & energy = RecMP->rec_e;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        hist->    Fill(x[i], y[i], energy[i]);
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    hist->GetZaxis()->SetTitle("Energy");
    QString title = "Energy" + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histNorm;
}

void AMercury_si::plotChi2XYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<int>    & dof    = RecMP->rec_dof;
    const std::vector<double> & chi2   = RecMP->rec_chi2min;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        hist-> Fill(x[i], y[i], chi2[i] / dof[i]);
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    hist->GetZaxis()->SetTitle("Chi2");
    QString title = "Chi2" + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histNorm;
}

void AMercury_si::plotStatusXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;

    for (size_t i = 0; i < status.size(); i++)
    {
        hist->Fill (x[i], y[i], (status[i] == 0 ? 0 : 1));
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    hist->GetZaxis()->SetTitle("Status");
    QString title = "Status" + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histNorm;
}

void AMercury_si::plotDensityXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix)
{
    TH2D * hist     = create2Dhist();
    //TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        hist-> Fill(x[i], y[i], 1);
        // no filling as there is no averaging!
    }
    //hist->Divide(histNorm); // no division!

    hist->GetZaxis()->SetTitle("Event density");
    QString title = "Density" + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
}

void AMercury_si::plotBiasXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & recX   = RecMP->rec_x;
    const std::vector<double> & recY   = RecMP->rec_y;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        hist-> Fill(x[i], y[i], (vsX ? recX[i] - x[i] : recY[i] - y[i]));
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    hist->GetZaxis()->SetTitle(vsX ? "Bias in X" : "Bias in Y");
    QString title = QString(vsX ? "BiasX" : "BiasY") + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histNorm;
}

void AMercury_si::plotSigmaXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & recX   = RecMP->rec_x;
    const std::vector<double> & recY   = RecMP->rec_y;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        const double delta = (vsX ? recX[i] - x[i] : recY[i] - y[i]);
        hist->Fill(x[i], y[i], delta * delta);
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    for (int bin = 0; bin <= hist->GetNcells(); bin++)
        hist->SetBinContent(bin, sqrt(hist->GetBinContent(bin)));

    hist->GetZaxis()->SetTitle(vsX ? "RMS error in X" : "RMS error in Y");
    QString title = QString(vsX ? "ErrorX" : "ErrorY") + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histNorm;
}

void AMercury_si::configure_COG(double signalAbsoluteCutoff, double signalRelativeCutoff)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    RecMP->setCogAbsCutoff(signalAbsoluteCutoff);
    RecMP->setCogRelCutoff(signalRelativeCutoff);
}

void AMercury_si::configure_statistical(bool reconstructEnergy, bool reconstructZ, double fixedZ)
{
    RecMinuitMP * rec = dynamic_cast<RecMinuitMP*>(RecMP);
    if (!rec)
    {
        abort("Reconstructor not created or it is not 'ML' or 'LS' type");
        return;
    }

    rec->setAutoE(reconstructEnergy);
    if (reconstructZ) rec->setFreeZ();
    else rec->setFixedZ(fixedZ);
}

void AMercury_si::setCutoffRadius(double val)
{
    if (RecMP) RecMP->setRecCutoffRadius(val);
    else abort("Reconstructor was not created yet");
}

void AMercury_si::configure_statistical_Minuit(double tolerance, int maxIterations, int maxFuncCalls)
{
    RecMinuitMP * rec = dynamic_cast<RecMinuitMP*>(RecMP);
    if (!rec)
    {
        abort("Reconstructor not created or it is not 'ML' or 'LS' type");
        return;
    }
    rec->setRMtolerance(tolerance);
    rec->setRMmaxIterations(maxIterations);
    rec->setRMmaxFuncCalls(maxFuncCalls);
}

void AMercury_si::configure_statistical_step(double initialStepX, double initialStepY, double initialStepZ, double initialStepEnergy)
{
    RecMinuitMP * rec = dynamic_cast<RecMinuitMP*>(RecMP);
    if (!rec)
    {
        abort("Reconstructor not created or it is not 'ML' or 'LS' type");
        return;
    }

    rec->setRMstepX(initialStepX);
    rec->setRMstepY(initialStepY);
    rec->setRMstepZ(initialStepZ);
    rec->setRMstepEnergy(initialStepEnergy);
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
    if (what == "ERRORX")  return ErrorXOption;
    if (what == "ERRORY")  return ErrorYOption;
    if (what == "ALL")     return EachValidOption;

    return ErrorOption;
}
