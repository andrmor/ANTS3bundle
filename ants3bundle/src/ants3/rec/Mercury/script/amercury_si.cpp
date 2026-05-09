#include "amercury_si.h"
#include "alightresponsehub.h"
#include "reconstructor_mp.h"
#include "ascripthub.h"

#include <QVariant>

#include "TH1D.h"
#include "TH2D.h"

AMercury_si::AMercury_si() :
    LRHub(ALightResponseHub::getInstance())
{
    Description = "A module for position reconstrucion of events based on 'Mercury' library of Vladimir Solovov.\n"
                  "Documentation can be found here:\n"
                  "https://mercurydocs.readthedocs.io/en/latest/index.html";

    Help["newReconstructor"] = "Create a reconstructor of a type defined by the 'type' argument:\n"
                               "'COG' (center of gravity), 'ML' (statistical, with maximum likelihood optimization) or 'LS' (statistical, with least squares optmization).\n"
                               "The second argument defines the number of threads used in reconstruction.\n"
                               "Requires the response model to be defined in 'response' script unit";

    Help["reconstructEvents"] = {{1, "Reconstruct event positions from the provided array of sensor amplitudes.\n"
                                     "The first dimension of the array is events, the second is sensor amplitudes"},
                                 {2, "Reconstruct event positions from the provided array of sensor amplitudes.\n"
                                     "The first dimension of that array is events, the second is sensor amplitudes.\n"
                                     "The second argument gives per-event information on which sesors to ignore:\n"
                                     "(e.g., due to saturation) and thus is array of arrays of bools: true signifies a sensor to ignore"}};

    Help["getRecXYZE"] = "Get array with reconstruction data for all events. Each event is an array of [X Y Z Energy].\n"
                         "Energy is 0 if reconstruction failed for that event";

    Help["getRecStats"] = "Get array with reconstruction stats for statistical reconstruction. For each event the following data are reported:\n"
                          "[Status Chi2 CovarianceXX CovarianceYY Covariance XY], where Status = 0 signifies successful reconstruction";

    Help["plot"] = "Plot 1-dimenional reconstruction-related data. The available 'what' options are:\n"
        "Energy, Chi2, Status and All. The 'All' option results in generation of plots with all available 1D options.\n"
        "Note that setting 'from' and 'to' arguments to zero results in autamtic axis ranges";

    Help["configure_plotXY_binning"] = "Configures XY binning for plot_vsRecXY and plot_vsTrueXY methods";

    Help["plot_vsRecXY"] = "Plot reconstruction-related data vs reconstructed XY position. The available 'what' options are:\n"
                           "Energy, Chi2, Status, Density and All. The 'All' option results in generation of plots with all available 2D options";

    Help["configure_plotXY_truePositions"] = "Configures true position data (array of [X Y Z] sub-arrays for all events) to be used in plot_vsTrueXY method";

    Help["plot_vsTrueXY"] = "Plot reconstruction-related data vs true XY position. The available 'what' options are:\n"
                            "Energy, Chi2, Status, Density, BiasX, BiasY, ResX, ResY and All. The 'All' option results in generation of plots with all available 2D options.\n"
                            "Bias is mean difference between the true and reconstructed;\n"
                            "Res is a quick-and-dirty estimate of resolution, obtained as 2.35 * RMS of (RecPos - Bias_this_bin - TruePos)";

    Help["configure_COG"] = "Configure COG reconstructor: 'signalAbsoluteCutoff' and 'signalRelativeCutoff' define the limits on sensor amplitudes:\n"
                            "For a given event, when a sensor signal is below that limit, this sensor is not considered in the reconstruction.\n"
                            "The limit can be given as an absolute value (the first argument) or as a fraction of the amplitude of the sensor with the strongest amplitude in the event (the second argument)";

    Help["configure_statistical"] = "Configure statistical reconstruction: whether ot not to reconstruct energy and Z. If Z is not to be reconstructed, 'fixedZ' argument sets the assumed Z position";
    Help["setCutoffRadius"] = "Define the cut-off radius from the center of the sensor with the maximum amplitude in an event.\n"
                              "All sensors, situated further (center-to-center distance), are not considered in the reconstruction of the event";
    Help["configure_statistical_step"] = "Fine-tuning of statistical reconstruction by providing the initial steps in the optimization process.\n"
                                         "The default values are 1 1 1 0";
    Help["configure_statistical_Minuit"] = "Fine-tuning of statistical reconstruction by providing the Minuit optimizer parameters: the tolerance, maximum number of iterations and maximumum number of function calls.\n"
                                           "The default values are 0.001 1000 and 500";
}

void AMercury_si::abortRun()
{
    if (RecMP)
    {
        qDebug() << "Abort requested --> mercury lib";
        RecMP->Abort();
    }
}

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

    std::vector<std::vector<double>> amplitudes(numEvents);

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList sensSignals = sensorSignalsOverAllEvents[iEv].toList();
        qsizetype numEl = sensSignals.size();

        amplitudes[iEv].resize(numEl);
        for (qsizetype i = 0; i < numEl; i++)
            amplitudes[iEv][i] = sensSignals[i].toDouble();
    }

    RecMP->ProcessEvents(amplitudes);

    EventsPassingFilter = std::vector<bool>(numEvents, true);
}

void AMercury_si::reconstructEvents(QVariantList sensorSignalsOverAllEvents, QVariantList ignoreSensorsByEvent)
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

    std::vector<std::vector<double>> amplitudes(numEvents);
    std::vector<std::vector<bool>>   ignoreSens(numEvents);

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList vlSensSignals = sensorSignalsOverAllEvents[iEv].toList();
        QVariantList vlIgnores     = ignoreSensorsByEvent[iEv].toList();

        qsizetype numEl = vlSensSignals.size();
        if (vlIgnores.size() != numEl)
        {
            abort("reconstructEvents: inconsistent number of elements in argument arrays");
            return;
        }

        amplitudes[iEv].resize(numEl);
        ignoreSens[iEv].resize(numEl);

        for (qsizetype i = 0; i < numEl; i++)
        {
            amplitudes[iEv][i] = vlSensSignals[i].toDouble();
            ignoreSens[iEv][i] = vlIgnores[i].toBool();
        }
    }

    RecMP->ProcessEvents(amplitudes, ignoreSens);

    EventsPassingFilter = std::vector<bool>(numEvents, true);
}

/*
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
*/

QVariantList AMercury_si::getRecXYZE()
{
    QVariantList res;
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return res;
    }

    const std::vector<int>    & good = RecMP->rec_status;
    const std::vector<double> & x    = RecMP->rec_x;
    const std::vector<double> & y    = RecMP->rec_y;
    const std::vector<double> & z    = RecMP->rec_z;
    const std::vector<double> & e    = RecMP->rec_e;

    const size_t size = x.size();
    if (size != y.size() || size != z.size() || size != e.size())
    {
        abort("Mismatch in xyze array sizes");
        return res;
    }

    bool bStatistical = (dynamic_cast<RecMinuitMP*>(RecMP));
    double thisZ = Z0;
    for (size_t i = 0; i < size; i++)
    {
        double energy;

        if (bStatistical)
        {
            energy = (good[i] == 0 ? e[i] : 0);
            thisZ = z[i];
        }
        else
        {
            energy = 1.0;
        }

        res.emplaceBack(QVariantList{x[i], y[i], thisZ, energy});
    }
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

void AMercury_si::clearEventFilter()
{
    EventFilter.clear();
}

void AMercury_si::setFilterByEnergy(double eMin, double eMax)
{
    EventFilter.
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
        plotChi2Hist(bins, 0, 0);
        emit AScriptHub::getInstance().requestAddToBasket("Chi2");
        plotEnergyHist(bins, 0, 0);
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

#include "ageomarkerclass.h"
void AMercury_si::showReconstructedPositions(QVariantList XYZE_ofEvents, QVariantList goodEvents)
{
    if (XYZE_ofEvents.isEmpty())
    {
        abort("showReconstructedPositions: XYZE_ofEvents should contain non-empty array of coordinates: [[x0,y0,z0], [x1,y1,z1], ... ]\n"
              "or arrays of xyze: [[x0,y0,z0, e0], [x1,y1,z1, e1], ... ] (events with energy=0 are not shown).");
        return;
    }

    const int numEvents = XYZE_ofEvents.size();
    bool haveGoods = false;
    if (!goodEvents.isEmpty())
    {
        if (numEvents != goodEvents.size())
        {
            abort("showReconstructedPositions: sizes of XYZE_ofEvents and goodEvents arrays are different");
            return;
        }
        haveGoods = true;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(EGeoMarkerType::PosReconstructed, 20, 1, 1); // properties are auto-updated
    for (int i = 0; i < numEvents; i++)
    {
        QVariantList el = XYZE_ofEvents[i].toList();
        if (el.size() < 3)
        {
            abort("showReconstructedPositions: bad format for coordinates in XYZE_ofEvents");
            delete markers;
            return;
        }

        if (el.size() > 3 && el[3].toDouble() == 0) continue;
        if (haveGoods && !goodEvents[i].toBool()) continue;

        markers->SetNextPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble());
    }

    AScriptHub & ScrHub = AScriptHub::getInstance();
    ScrHub.prepareToWait();
    emit ScrHub.requestAddMarkers(markers);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AMercury_si::showTruePositions(QVariantList XYZ_ofEvents, QVariantList goodEvents)
{
    if (XYZ_ofEvents.isEmpty())
    {
        abort("showTruePositions: XYZ_ofEvents should contain non-empty array of coordinates: [[x0,y0,z0], [x1,y1,z1], ... ]");
        return;
    }

    const int numEvents = XYZ_ofEvents.size();
    bool haveGoods = false;
    if (!goodEvents.isEmpty())
    {
        if (numEvents != goodEvents.size())
        {
            abort("showTruePositions: sizes of XYZ_ofEvents and goodEvents arrays are different");
            return;
        }
        haveGoods = true;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(EGeoMarkerType::PosTrue, 20, 1, 1); // properties are auto-updated
    for (int i = 0; i < numEvents; i++)
    {
        QVariantList el = XYZ_ofEvents[i].toList();
        if (el.size() < 3)
        {
            abort("showTruePositions: bad format for coordinates in XYZ_ofEvents");
            delete markers;
            return;
        }

        //if (el.size() > 3 && el[3].toDouble() == 0) continue;
        if (haveGoods && !goodEvents[i].toBool()) continue;

        markers->SetNextPoint(el[0].toDouble(), el[1].toDouble(), el[2].toDouble());
    }

    AScriptHub & ScrHub = AScriptHub::getInstance();
    ScrHub.prepareToWait();
    emit ScrHub.requestAddMarkers(markers);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AMercury_si::showEventExplorer(QVariantList sensorSignalsOverAllEvents, QVariantList truePositions)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const size_t numEvents = sensorSignalsOverAllEvents.size();
    if (numEvents == 0)
    {
        abort("The array with events for showEventExplorer is empty");
        return;
    }

    bool bHaveTrue = false;
    if (!truePositions.isEmpty())
    {
        if (numEvents != truePositions.size())
        {
            abort("The array with true positions for showEventExplorer has mismatching number of events");
            return;
        }
        bHaveTrue = true;
    }

    std::vector<std::vector<double>> * amplitudes = new std::vector<std::vector<double>>(numEvents); // will be owned by the Explorer

    std::vector<std::array<double,3>> * trues = nullptr;
    if (bHaveTrue) trues = new std::vector<std::array<double,3>>(numEvents); // will be owned by the Explorer

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList sensSignals = sensorSignalsOverAllEvents[iEv].toList();
        qsizetype numEl = sensSignals.size();

        amplitudes->at(iEv).resize(numEl);
        for (qsizetype i = 0; i < numEl; i++)
            amplitudes->at(iEv)[i] = sensSignals[i].toDouble();

        if (bHaveTrue)
        {
            QVariantList position = truePositions[iEv].toList();
            const qsizetype numEl = position.size();
            if (numEl > 1)
            {
                trues->at(iEv)[0] = position[0].toDouble();
                trues->at(iEv)[1] = position[1].toDouble();
            }
            if (numEl > 2)
                trues->at(iEv)[2] = position[2].toDouble();
        }
    }

    emit AScriptHub::getInstance().requestShowEventExplorer(RecMP->getFirstWorker(), amplitudes, trues);
}

void AMercury_si::doPlot_vsXY(bool vsTrue, EPlotOption opt, const std::vector<double> & x, const std::vector<double> & y)
{
    if (!vsTrue)
        if (opt == BiasXOption || opt == BiasYOption || opt == ResXOption || opt == ResYOption)
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
    case ResXOption:
        plotResXYHist(x, y, titleSuffix, true);
        break;
    case ResYOption:
        plotResXYHist(x, y, titleSuffix, false);
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
            plotResXYHist(x, y, titleSuffix, true);  emit AScriptHub::getInstance().requestAddToBasket("ResX" + titleSuffix);
            plotResXYHist(x, y, titleSuffix, false); emit AScriptHub::getInstance().requestAddToBasket("ResY" + titleSuffix);
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

void AMercury_si::plotResXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX)
{
    TH2D * hist     = create2Dhist();
    TH2D * histNorm = create2Dhist();

    const std::vector<int>    & status = RecMP->rec_status;
    const std::vector<double> & recX   = RecMP->rec_x;
    const std::vector<double> & recY   = RecMP->rec_y;

    //computing bias
    TH2D * histB     = create2Dhist();
    TH2D * histNormB = create2Dhist();
    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        histB-> Fill(x[i], y[i], (vsX ? recX[i] - x[i] : recY[i] - y[i]));
        histNormB->Fill(x[i], y[i], 1);
    }
    histB->Divide(histNormB);
    delete histNormB;

    for (size_t i = 0; i < status.size(); i++)
    {
        if (status[i] != 0) continue;
        double delta = (vsX ? recX[i] - x[i] : recY[i] - y[i]);
        const double bias = histB->GetBinContent(histB->FindBin(x[i], y[i])); // it is <rec - true>
        delta -= bias;
        hist->Fill(x[i], y[i], delta * delta);
        histNorm->Fill(x[i], y[i], 1);
    }
    hist->Divide(histNorm);

    for (int bin = 0; bin <= hist->GetNcells(); bin++)
        hist->SetBinContent(bin, 2.35 * sqrt(hist->GetBinContent(bin)));

    hist->GetZaxis()->SetTitle(vsX ? "Resolution estimate in X" : "Resolution estimate in Y");
    QString title = QString(vsX ? "ResX" : "ResY") + titleSuffix;
    hist->SetTitle(title.toLatin1().data());

    emit AScriptHub::getInstance().requestDraw(hist, "colz", true);
    delete histB;
    delete histNorm;
}

void AMercury_si::configure_COG(double signalAbsoluteCutoff, double signalRelativeCutoff, double z0)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    RecMP->setCogAbsCutoff(signalAbsoluteCutoff);
    RecMP->setCogRelCutoff(signalRelativeCutoff);
    Z0 = z0;
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
    if (what == "RESX")    return ResXOption;
    if (what == "RESY")    return ResYOption;
    if (what == "ALL")     return EachValidOption;

    return ErrorOption;
}

// ---

void AEventFilterRecord::clear()
{
    SuccessRec = false;

    ByEnergy = false;
    EnergyMin = 0;
    EnergyMax = 1e99;

    ByChi2 = false;
    Chi2Min = 0;
    Chi2Max = 1e99;
}
