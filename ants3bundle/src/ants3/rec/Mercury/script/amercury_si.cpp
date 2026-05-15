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

void AMercury_si::importSensorSignals(QVariantList sensorSignalsOverAllEvents)
{
    const size_t numEvents = sensorSignalsOverAllEvents.size();
    if (numEvents == 0)
    {
        abort("The array with sensor signals is empty");
        return;
    }

    SensorSignals.resize(numEvents);

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList sensSignals = sensorSignalsOverAllEvents[iEv].toList();
        qsizetype numEl = sensSignals.size();

        SensorSignals[iEv].resize(numEl);
        for (qsizetype i = 0; i < numEl; i++)
            SensorSignals[iEv][i] = sensSignals[i].toDouble();
    }
}

void AMercury_si::clearTruePositions()
{
    XTruePositions.clear();
    YTruePositions.clear();
    ZTruePositions.clear();
}

void AMercury_si::reconstructEvents()
{
    if (!RecMP)
    {
        abort("Reconstructor was not created");
        return;
    }
    if (SensorSignals.empty())
    {
        abort("Sensor signals were not imported");
        return;
    }

    RecMP->ProcessEvents(SensorSignals);

    PassFilter = std::vector<bool>(SensorSignals.size(), true);
}

void AMercury_si::reconstructEvents(QVariantList ignoreSensorsByEvent)
{
    if (!RecMP)
    {
        abort("Reconstructor was not created");
        return;
    }

    const size_t numEvents  = SensorSignals.size();
    const size_t numSensors = SensorSignals.front().size();
    std::vector<std::vector<bool>> ignoreSens(numEvents);

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        QVariantList vlIgnores = ignoreSensorsByEvent[iEv].toList();

        if (vlIgnores.size() != numSensors)
        {
            abort("reconstructEvents with ignoreSensorsByEvent: inconsistent number of sensors in SensorSignals and ignoreSensorsByEvent arrays");
            return;
        }

        ignoreSens[iEv].resize(numSensors);
        for (qsizetype iSens = 0; iSens < numSensors; iSens++)
            ignoreSens[iEv][iSens] = vlIgnores[iSens].toBool();
    }

    RecMP->ProcessEvents(SensorSignals, ignoreSens);

    PassFilter = std::vector<bool>(numEvents, true);
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

QVariantList AMercury_si::getRecXYZE(bool ignoreFilter)
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

    const size_t numEvents = x.size();
    if (numEvents != y.size() || numEvents != z.size() || numEvents != e.size())
    {
        abort("getRecXYZE: Unexpected mismatch in xyze array sizes");
        return res;
    }
    if (!ignoreFilter && numEvents != PassFilter.size())
    {
        abort("getRecXYZE: unexpected mismatch in SensorSignals and PassFilter sizes");
        return res;
    }

    bool bStatistical = (dynamic_cast<RecMinuitMP*>(RecMP));
    double thisZ = Z0;
    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        if (!ignoreFilter && !PassFilter[iEv]) continue;

        double energy;
        if (bStatistical)
        {
            energy = (good[iEv] == 0 ? e[iEv] : 0);
            thisZ = z[iEv];
        }
        else
            energy = 1.0;

        res.emplaceBack(QVariantList{x[iEv], y[iEv], thisZ, energy});
    }
    return res;
}

QVariantList AMercury_si::getRecStats(bool ignoreFilter)
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

    const size_t numEvents = status.size();
    if (numEvents != chi2.size() || numEvents != cov_xx.size() || numEvents != cov_yy.size() || numEvents != cov_xy.size())
    {
        abort("getRecStats: Unexpected mismatch in array sizes");
        return res;
    }
    if (!ignoreFilter && numEvents != PassFilter.size())
    {
        abort("getRecStats: unexpected mismatch in SensorSignals and PassFilter sizes");
        return res;
    }

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        if (!ignoreFilter && !PassFilter[iEv]) continue;

        if (status[iEv] == 0) res.emplaceBack(QVariantList{status[iEv], chi2[iEv] / dof[iEv], cov_xx[iEv], cov_yy[iEv], cov_xy[iEv]});
        else                  res.emplaceBack(QVariantList{status[iEv], 0,                    0,           0,           0});
    }
    return res;
}

void AMercury_si::clearEventFilter()
{
    EventFilter.clear();
}

void AMercury_si::setFilterRecSuccess()
{
    EventFilter.SuccessRec = true;
}

void AMercury_si::setFilterByEnergy(double eMin, double eMax)
{
    EventFilter.ByEnergy = true;
    EventFilter.EnergyMin = eMin;
    EventFilter.EnergyMax = eMax;
}

void AMercury_si::setFilterByChi2(double chi2Min, double chi2Max)
{
    EventFilter.ByChi2 = true;
    EventFilter.Chi2Min = chi2Min;
    EventFilter.Chi2Max = chi2Max;
}

QString AMercury_si::applyFilter()
{
    if (!RecMP)
    {
        abort("Reconstructor was not created");
        return "";
    }

    const std::vector<int>    & good = RecMP->rec_status;
    const std::vector<double> & e    = RecMP->rec_e;
    const std::vector<double> & chi  = RecMP->rec_chi2min;
    const std::vector<int>    & dof  = RecMP->rec_dof;

    const size_t numEvents = good.size();
    if (numEvents != PassFilter.size())
    {
        abort("Reconstruction was not performed");
        return "";
    }

    bool bStatistical = (dynamic_cast<RecMinuitMP*>(RecMP));
    int numGood = 0;
    int numSuc = 0;
    int killByStat = 0;
    int killByE = 0;
    int killByChi2 = 0;
    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        const bool recSuccess = (good[iEv] == 0);
        if (recSuccess) numSuc++;
        if (EventFilter.SuccessRec && !recSuccess)
        {
            PassFilter[iEv] = false;
            killByStat++;
            continue;
        }

        if (EventFilter.ByChi2)
        {
            if (!bStatistical || !recSuccess)
            {
                PassFilter[iEv] = false;
                killByChi2++;
                continue;
            }
            const double chi2 = chi[iEv] / dof[iEv];
            if (chi2 < EventFilter.Chi2Min || chi2 > EventFilter.Chi2Max)
            {
                PassFilter[iEv] = false;
                killByChi2++;
                continue;
            }
        }

        if (EventFilter.ByEnergy)
        {
            double energy;
            if (!recSuccess) energy = 0;
            else if (!bStatistical) energy = 1.0;
            else energy = e[iEv];
            if (energy < EventFilter.EnergyMin || energy > EventFilter.EnergyMax)
            {
                PassFilter[iEv] = false;
                killByE++;
                continue;
            }
        }

        // passed all filters
        PassFilter[iEv] = true;
        numGood++;
    }

    QString txt = QString("Events provided: %0").arg(numEvents);
    txt += QString("  Reconstruction success: %0").arg(numSuc);
    txt += QString("\nPassing the filter: %0").arg(numGood);
    if (EventFilter.SuccessRec) txt += QString("\n  killed by status: %0").arg(killByStat);
    if (EventFilter.ByChi2)     txt += QString("\n  killed by chi2: %0").arg(killByChi2);
    if (EventFilter.ByEnergy)   txt += QString("\n  killed by energy: %0").arg(killByE);

    return txt;
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
    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        h->Fill(energy[iEv], 1);
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
    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        h->Fill(chi2[iEv] / dof[iEv], 1);
    }
    emit AScriptHub::getInstance().requestDraw(h, "hist", true);
}

void AMercury_si::plotStatusHist()
{
    const std::vector<int> & status = RecMP->rec_status;

    TH1D * h = new TH1D("", "status", 2, 0, 2);
    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        h->Fill( (status[iEv] == 0 ? 0 : 1), 1);
    }

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

void AMercury_si::importTruePositions(QVariantList truePositions)
{
    const size_t num = truePositions.size();
    if (num == 0)
    {
        abort("truePositions array is empty");
        return;
    }

    XTruePositions.resize(num);
    YTruePositions.resize(num);
    ZTruePositions.resize(num);
    for (size_t iEv = 0; iEv < num; iEv++)
    {
        QVariantList el = truePositions[iEv].toList();
        if (el.size() < 2)
        {
            abort("truePositions array should contain arrays of XYs or XYZs");
            return;
        }
        bool ok0, ok1;
        XTruePositions[iEv] = el[0].toDouble(&ok0);
        YTruePositions[iEv] = el[1].toDouble(&ok1);
        bool ok2 = true;
        if (el.size() > 2) ZTruePositions[iEv] = el[2].toDouble(&ok2);
        else               ZTruePositions[iEv] = Z0;
        if (ok0 && ok1 && ok2) continue;

        abort("Bad x, y or z value format in TruePositions array");
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
void AMercury_si::showReconstructedPositions()
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const std::vector<double> & x    = RecMP->rec_x;
    const std::vector<double> & y    = RecMP->rec_y;
    const std::vector<double> & z    = RecMP->rec_z;

    const int numEvents = x.size();
    if (numEvents != PassFilter.size())
    {
        abort("Reconstruction was not yet performed");
        return;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(EGeoMarkerType::PosReconstructed, 20, 1, 1); // properties are auto-updated
    for (int iEv = 0; iEv < numEvents; iEv++)
    {
        if (!PassFilter[iEv]) continue;
        markers->SetNextPoint(x[iEv], y[iEv], z[iEv]);
    }

    AScriptHub & ScrHub = AScriptHub::getInstance();
    ScrHub.prepareToWait();
    emit ScrHub.requestAddMarkers(markers);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AMercury_si::showTruePositions(bool invertFilterStatus)
{
    const int numEvents = XTruePositions.size();
    if (numEvents != PassFilter.size())
    {
        abort("Mismatch in event numbers: true positions and filter status");
        return;
    }

    AGeoMarkerClass * markers = new AGeoMarkerClass(EGeoMarkerType::PosTrue, 20, 1, 1); // properties are auto-updated
    for (int iEv = 0; iEv < numEvents; iEv++)
    {
        if (PassFilter[iEv] == invertFilterStatus) continue; // no inv, fail rec: (false == false) --> continue
        markers->SetNextPoint(XTruePositions[iEv], YTruePositions[iEv], ZTruePositions[iEv]);
    }

    AScriptHub & ScrHub = AScriptHub::getInstance();
    ScrHub.prepareToWait();
    emit ScrHub.requestAddMarkers(markers);
    ScrHub.waitForGuiCallFinished(Lang);
}

void AMercury_si::showEventExplorer()
{
    if (!RecMP)
    {
        abort("Reconstructor was not created yet");
        return;
    }

    const size_t numEvents = SensorSignals.size();
    if (numEvents == 0)
    {
        abort("The array with events for showEventExplorer is empty");
        return;
    }
    const size_t numSens = SensorSignals.front().size();

    bool bHaveTrue = false;
    if (!XTruePositions.empty())
    {
        if (numEvents != XTruePositions.size())
        {
            abort("showEventExplorer: mismatch in the number of events in SensorSignals and TruePositions");
            return;
        }
        bHaveTrue = true;
    }

    std::vector<std::vector<double>>  * amplitudes = new std::vector<std::vector<double>>(numEvents); // will be owned by the Explorer

    std::vector<std::array<double,3>> * trues = nullptr;
    if (bHaveTrue) trues = new std::vector<std::array<double,3>>(numEvents); // will be owned by the Explorer

    for (size_t iEv = 0; iEv < numEvents; iEv++)
    {
        amplitudes->at(iEv).resize(numSens);
        for (size_t iSens = 0; iSens < numSens; iSens++)
            amplitudes->at(iEv)[iSens] = SensorSignals[iEv][iSens];

        if (bHaveTrue)
        {
            trues->at(iEv)[0] = XTruePositions[0];
            trues->at(iEv)[1] = XTruePositions[1];
            trues->at(iEv)[2] = XTruePositions[2];
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

    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        hist->    Fill(x[iEv], y[iEv], energy[iEv]);
        histNorm->Fill(x[iEv], y[iEv], 1);
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

    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        hist-> Fill(x[iEv], y[iEv], chi2[iEv] / dof[iEv]);
        histNorm->Fill(x[iEv], y[iEv], 1);
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

    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        hist->Fill (x[iEv], y[iEv], (status[iEv] == 0 ? 0 : 1));
        histNorm->Fill(x[iEv], y[iEv], 1);
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

    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        hist->Fill(x[iEv], y[iEv], 1);
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

    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        hist-> Fill(x[iEv], y[iEv], (vsX ? recX[iEv] - x[iEv] : recY[iEv] - y[iEv]));
        histNorm->Fill(x[iEv], y[iEv], 1);
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
    for (size_t iEv = 0; iEv < status.size(); iEv++)
    {
        if (!PassFilter[iEv]) continue;
        if (status[iEv] != 0) continue;
        histB-> Fill(x[iEv], y[iEv], (vsX ? recX[iEv] - x[iEv] : recY[iEv] - y[iEv]));
        histNormB->Fill(x[iEv], y[iEv], 1);
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
