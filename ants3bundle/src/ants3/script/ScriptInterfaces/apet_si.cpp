#include "apet_si.h"
#include "ageometryhub.h"
#include "avector.h"
#include "afiletools.h"
#include "ascripthub.h"
#include "apeteventbuilder.h"
#include "apetcoincidencefinder.h"

#include <iostream>
#include <fstream>
#include <ostream>
#include <ios>

#include <QDebug>
#include <QProcess>
#include <QDir>

APet_si::APet_si() :
    AScriptInterface()
{
    Help["createScanner"] = "Creates description file and crystal LUT file with name given by scannerName argument in the CASTOR's "
                            "config/scanner directory using the scintillators defined in the current geometry.\n"
                            "scannerRadius, crystalDepth and crystalSize arguments provide the radius of the scanner (from center to the front surface of scintillators),"
                            "length and xy_size of the scintillator crystals, respectively (all in mm)\n"
                            "minAngle_deg defined the minimum angle in degrees between the scintillators for a coincidence pair to be considered.";

    Help["configureBuilderTimeWindows"] = "Defines time windows during which the scanner records activity data. The number of windows is arbitrary, each element of "
                                          "the argument array is an array of {timeFrom,timeTo}, both in ns. Each time range is processed independently, so even one "
                                          "range can be split in second ones to speed up processing";
    Help["configureBuilderClustering"] = "maxTimeDeltaCluster defines the time threshold for pre-clustering procedure: if two neighboring deposition records in the same scintillator are"
                                         "closer in time than this time, they will be merged already in the deposition file reading phase.\nclusterTime"
                                         "is used similarly in the custering phase, after sorting the deposition nodes by time."
                                         "integrationTime and deadTime are used in eventt building phase: deposition clusters are added up to an event during integrationTime: "
                                         "energy is the sum of these of the individfual clusters and the event time is given by the earlist cluster. During the time after the end of the inegrationTime"
                                         "and before the end of the deadTime from the event start, all despoition nodes are disreguarded.";
    Help["configureBuilderSeed"] = "Configures the seed of the random generator needed to simulate CTR and energy resolution";
    Help["configureBuilderCTR"] = "Configures the Coincidence Timing Resolution of the scanner in ns: all events will be 'blurred' assuming Gaussian distribution to result in this CTR for the coincidence pairs";
    Help["configureBuilderEnergies"] = "Configures energy resolution (the ratio of FWHM to the mean value) of the sanner and energy threshold in keV below which an event is ignored (not saved to the output file)."
                                       "Note that the energy values of all events will be blurred assuming Gaussian distribution and uing the provided energy resolution";
    Help["buildEventsFromDeposition"] = "Build events using energy deposition records in a provided ANTS3 deposition file and creates an output find with the constructed events according to the configuration settings "
                                        "configured using the methods starting from configureBuilder...";


    Help["configureCoincidenceWindow"] = "Configures the maximum time window in ns for defining two events as a coincidence";
    Help["configureCoincidenceGlobalTimeRange"] = "Coincidences will be ignored if they have the time stamp outside of the defined time window";
    Help["configureCoincidenceEnergyWindow"] = "Coincidences will be ignored if at least the of the events has energy value outside of the defined energy range (in keV)";
    Help["findCoincidences"] = "Find coincidences for the scanner name scannerName using the event datafile eventsFileName and save the results in the file coincFileName "
                               "with or without time-of-flight information according to the writeToF flag. Th process is configured using the methods with names starting from configureCoincidence...";

    Help["configureReconstructionVoxels"] = "Configure Castor voxels: number of them in x, y, and z direction, as well as the voxels size (x, y and z) in mm. The center voxel is cituated in the center of the scanner ring";
    Help["reconstructConfigureAlgorithm"] = "For list-mode PET datasets, CASToR can be configured to use DEPIERRO95, MLEM (default) and OSL algorithms.\n"
                                            "If MLEM is selected, but subsets are used (see configureReconstructionIterations), the algorithm \"naturally becomes OSEM\"";
    Help["configureReconstructionIterations"] = "configures the number of iteration and number of subsets used by Castor";
    Help["configureReconstructionGaussianConvolver"] = "Castor is configured to use Gaussian convolver with 'psf' option. Using this method it is possible to configure the FWHMs (transaxial and axias) as weel as the number of sigmas included in the for the kernel (rcommended from 3.0 to 5.0)";
    Help["configureReconstructionIgnoreTOF"] = "If set to true, ignores ToF data if present in the input file with coincidences";
    Help["reconstruct"] = "Perform image reconstruction using castor package using input file coincFileName and placing output in the directory QString outDir"
                          "If castor was compiled with OpenMP use numThreads argument to provide th enumber of threads. The reconstruction process is further "
                          "configured using the methods with names starting from configureReconstruction...";

    Help["loadImage"] = "Load image from CASToR output file (file suffix .hdr).\n"
                        "The method returns an array of two arrays.\n"
                        "The first one is a 3D array of image voxels [ix][iy][iz]\n"
                        "The second one is array of image parameters: [ [NumBinsX,NumBinsY,NumBinsZ], [mmPerPixelX,mmPerPixelY,mmPerPixelZ], [OffsetX_mm,OffsetY_mm,OffsetZ_mm] ]\n"
                        "Note that for visualization a special option is provided: use grwin.show3D(fileName), which opens a dedicated 3D graph viewer";
}

void APet_si::createScanner(QString scannerName, double scannerRadius, double crystalDepth, double crystalSize, double minAngle_deg)
{
    QString castorStr;
    const QStringList environment = QProcess::systemEnvironment();
    for (const QString & s : environment)
        if (s.contains("CASTOR_CONFIG="))
        {
            castorStr = s;
            castorStr.remove("CASTOR_CONFIG=");
            break;
        }

    if (castorStr.isEmpty())
    {
        abort("System environmental variable CASTOR_CONFIG is not configured!");
        return;
    }
    qDebug() << castorStr;

    QDir castorDir(castorStr);
    if (!castorDir.exists())
    {
        abort("System environmental variable CASTOR_CONFIG points to non-existent directory!");
        return;
    }

    QDir scannerDir(castorStr + "/scanner");
    if (!scannerDir.exists())
    {
        abort("Scanner directory not found!");
        return;
    }

    size_t numScint = AGeometryHub::getConstInstance().countScintillators();
    if (numScint == 0)
    {
        abort("Current configuration does not have defined scintillators!");
        return;
    }

    bool ok = makeLUT(castorStr + "/scanner/" + scannerName + ".lut");
    if (!ok) return;

    QString numScintStr = QString::number(numScint);

    QString header =
        "modality: PET\n"
        "scanner name: " + scannerName + "\n"
        "scanner radius: " + QString::number(scannerRadius) + "\n"
        "number of layers: 1\n"
        "number of elements: " + numScintStr + "\n"
        "number of crystals in layer: " + numScintStr + "\n"
        "crystals size depth: " + QString::number(crystalDepth) + "\n"
        "crystals size transaxial: " + QString::number(crystalSize) + "\n"
        "crystals size axial: " + QString::number(crystalSize) + "\n"
        "voxels number transaxial: 256\n"
        "voxels number axial: 128\n"
        "field of view transaxial: 200.0\n"
        "field of view axial: 100.0\n"
        "min angle difference: " + QString::number(minAngle_deg) + "\n"
        "description: Auto-generated by ANTS3\n";

    ftools::saveTextToFile(header, castorStr + "/scanner/" + scannerName + ".hscan");
}

bool APet_si::makeLUT(QString fileName)
{
    std::ofstream outStream(fileName.toLatin1().data(), std::ios::out | std::ios::binary );
    if (!outStream.is_open())
    {
        abort("Cannot open file for writing: " + fileName);
        return false;
    }

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    std::vector<AVector3> pos;
    GeoHub.getScintillatorPositions(pos);
    std::vector<AVector3> ori;
    GeoHub.getScintillatorOrientations(ori);

    for (size_t iScint = 0; iScint < pos.size(); iScint++)
    {
        for (size_t i = 0; i < 3; i++)
        {
            float tmp = (float)pos[iScint][i];
            outStream.write((char*)&tmp, sizeof(float));
        }
        for (size_t i = 0; i < 3; i++)
        {
            float tmp = (float)ori[iScint][i];
            outStream.write((char*)&tmp, sizeof(float));
        }
    }
    outStream.close();
    return true;
}

void APet_si::configureBuilderTimeWindows(QVariantList arrayOfTimeFromAndTimeTo)
{
    std::vector<std::pair<double,double>> timeRanges;
    if (arrayOfTimeFromAndTimeTo.size() == 2 && arrayOfTimeFromAndTimeTo[0].toList().size() == 0 && arrayOfTimeFromAndTimeTo[1].toList().size() == 0)
    {
        bool ok1, ok2;
        double from = arrayOfTimeFromAndTimeTo[0].toDouble(&ok1);
        double to   = arrayOfTimeFromAndTimeTo[1].toDouble(&ok2);
        if (!ok1 || !ok2 || from < 0 || to < 0 || from >= to)
        {
            abort("arrayOfTimeFromAndTimeTo should contain arrays of [timeFrom, timeTo]. Both values should be positive and from < to");
            return;
        }
        timeRanges.push_back({from, to});
    }
    else
    {
        for (int i = 0; i < arrayOfTimeFromAndTimeTo.size(); i++)
        {
            QVariantList range = arrayOfTimeFromAndTimeTo[i].toList();
            if (range.size() != 2)
            {
                abort("arrayOfTimeFromAndTimeTo should contain arrays of [timeFrom, timeTo]");
                return;
            }
            bool ok1, ok2;
            double from = range[0].toDouble(&ok1);
            double to   = range[1].toDouble(&ok2);
            if (!ok1 || !ok2 || from < 0 || to < 0 || from >= to)
            {
                abort("arrayOfTimeFromAndTimeTo should contain arrays of [timeFrom, timeTo]. Both values should be positive and from < to");
                return;
            }
            timeRanges.push_back({from, to});
        }
    }

    if (timeRanges.empty())
    {
        abort("arrayOfTimeFromAndTimeTo should contain arrays of [timeFrom, timeTo]. It cannot be empty.");
        return;
    }
    BuilderConfig.TimeRanges = timeRanges;
    qDebug() << "Time ranges:" << BuilderConfig.TimeRanges.size();
}

void APet_si::configureBuilderClustering(double maxTimeDeltaCluster, double clusterTime, double integrationTime, double deadTime)
{
    BuilderConfig.MaxTimeDeltaCluster = maxTimeDeltaCluster;
    BuilderConfig.ClusterTime         = clusterTime;
    BuilderConfig.IntegrationTime     = integrationTime;
    BuilderConfig.DeadTime            = deadTime;
}

void APet_si::configureBuilderSeed(int seed)
{
    BuilderConfig.Seed = seed;
}

void APet_si::configureBuilderCTR(double CTR_ns)
{
    BuilderConfig.CTR = CTR_ns;
}

void APet_si::configureBuilderEnergies(double energyResolution_fraction, double energyThreshold_keV)
{
    BuilderConfig.EnergyResolution = energyResolution_fraction;
    BuilderConfig.EnergyThreshold  = energyThreshold_keV;
}

void APet_si::addDepositionFile(QString depositionFileName, bool binary)
{
    DepoFiles.push_back( {depositionFileName.toLatin1().data(), binary} );
}

double APet_si::buildEventsFromDeposition(QString eventsFileName, bool binary)
{
    size_t numScint = AGeometryHub::getConstInstance().countScintillators();
    APetEventBuilder eb(numScint);
    for (const auto & pair : DepoFiles) eb.addInputFile(pair.first, pair.second);
    eb.configure(BuilderConfig);
    return eb.makeEvents(eventsFileName.toLatin1().data(), binary);
}

// --------------

void APet_si::configureCoincidenceWindow(double coincidenceWindow_ns)
{
    CoincidenceConfig.CoincidenceWindow = coincidenceWindow_ns;
}

void APet_si::configureCoincidenceGlobalTimeRange(double from_ns, double to_ns)
{
    CoincidenceConfig.TimeFrom = from_ns;
    CoincidenceConfig.TimeTo   = to_ns;
}

void APet_si::configureCoincidenceEnergyWindow(double energyFrom_keV, double energyTo_keV)
{
    CoincidenceConfig.EnergyFrom = energyFrom_keV;
    CoincidenceConfig.EnergyTo   = energyTo_keV;
}

double APet_si::findCoincidences(QString scannerName, QString eventsFileName, bool binary, QString coincFileName, bool writeToF)
{
    size_t numScint = AGeometryHub::getConstInstance().countScintillators();
    APetCoincidenceFinder cf(scannerName, numScint, eventsFileName, binary);
    cf.configure(CoincidenceConfig);
    double numCoinc = cf.findCoincidences(coincFileName, writeToF);
    if (!cf.ErrorString.isEmpty()) abort(cf.ErrorString);
    return numCoinc;
}

// ----------------

void APet_si::configureReconstructionVoxels(int numX, int numY, int numZ, double sizeX, double sizeY, double sizeZ)
{
    if (numX < 1 || numY < 1 || numZ < 1)
    {
        abort("Number of voxel for PET reconstruction should be positive");
        return;
    }

    if (sizeX <= 0 || sizeY <= 0 || sizeZ <= 0)
    {
        abort("Voxel sizes for PET reconstruction should be positive");
        return;
    }

    NumVoxels  = {numX,  numY,  numZ};
    SizeVoxels = {sizeX, sizeY, sizeZ};
}

void APet_si::configureReconstructionAlgorithm(QString algorithmName)
{
    AlgorithmName = algorithmName;
}

void APet_si::configureReconstructionIterations(int numIteration, int numSubsets)
{
    NumIterations = numIteration;
    NumSubsets = numSubsets;
}

void APet_si::configureReconstructionGaussianConvolver(double transaxialFWHM_mm, double axialFWHM_mm, double numberOfSigmas)
{
    TransaxialFWHM_mm = transaxialFWHM_mm;
    AxialFWHM_mm = axialFWHM_mm;
    NumberOfSigmas = numberOfSigmas;
}

void APet_si::configureReconstructionIgnoreTOF(bool flag)
{
    IgnoreTOF = flag;
}

#include <QProcess>
#include <QApplication>
#include <QThread>
void APet_si::reconstruct(QString coincFileName, QString outDir, int numThreads)
{
    Process = new QProcess();
    Process->setProcessChannelMode(QProcess::MergedChannels);

    bool isRunning = true;
    QObject::connect(Process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [&isRunning](){isRunning = false; qDebug() << "----CASTOR FINISHED!-----";});
    //QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){ /* ... */ });

    QObject::connect(Process, &QProcess::readyReadStandardOutput, this, &APet_si::onReadReady);

    //castor-recon -df Tor.cdh -opti MLEM -it 10:16 -proj joseph -conv gaussian,2.0,2.5,3.5::psf -dim 128,128,128 -vox 3.0,3.0,3.0 -dout /home/andr/WORK/ANTS3/castor/Out/Tor/images
    QString program = "castor-recon";
    QStringList args;
    args << "-df" << coincFileName;
    if (numThreads > 1) args << "-th" << QString::number(numThreads);
    args << "-opti" << AlgorithmName;
    args << "-it" << QString("%1:%2").arg(NumIterations).arg(NumSubsets);
    args << "-proj" << "joseph";                    // !
    //args << "-conv" << "gaussian,2.0,2.5,3.5::psf";
    args << "-conv" << QString("gaussian,%1,%2,%3::psf").arg(TransaxialFWHM_mm).arg(AxialFWHM_mm).arg(NumberOfSigmas);
    if (IgnoreTOF) args << "-ignore-TOF";
    //args << "-dim" << "128,128,128";
    args << "-dim" << QString("%1,%2,%3").arg(NumVoxels[0]).arg(NumVoxels[1]).arg(NumVoxels[2]);
    //args << "-vox" << "3.0,3.0,3.0";
    args << "-vox" << QString("%1,%2,%3").arg(SizeVoxels[0]).arg(SizeVoxels[1]).arg(SizeVoxels[2]);
    args << "-dout" << outDir;

    qDebug() << "Starting external process:" << program << " with arguments:\n" << args;

    Process->start(program, args);
    bool ok = Process->waitForStarted(1000);
    if (!ok)
    {
        abort("Failed to start reconstruction using castor-recon");
        return;
    }

    while (isRunning)
    {
        //bool ok = Process->waitForFinished(100); // ms
        //if (ok) break;

        QThread::usleep(100);
        QApplication::processEvents();

        if (AScriptHub::isAborted(Lang))
        {
            Process->terminate();
            break;
        }
    }

    QString err = Process->errorString();
    if (!err.isEmpty() && err != "Unknown error")
        abort("Reconstruction failed:\n" + err);
}

void APet_si::onReadReady()
{
    const QString in = Process->readAllStandardOutput();

    const QStringList input = in.split('\n', Qt::SkipEmptyParts);
    for (const QString & message : input)
        AScriptHub::getInstance().outputText(message, Lang);
}

#include <QVariantList>
#include "acastorimageloader.h"
QVariantList APet_si::loadImage(QString fileName)
{
    QVariantList vl;

    ACastorImageLoader il;
    QString err = il.loadImage(fileName);
    if (!err.isEmpty())
    {
        abort("Castor image loader error:\n" + err);
        return vl;
    }

    QVariantList data; data.reserve(il.NumBinsX);
    for (int ix = 0; ix < il.NumBinsX; ix++)
    {
        QVariantList vlX; vlX.reserve(il.NumBinsY);
        for (int iy = 0; iy < il.NumBinsY; iy++)
        {
            QVariantList vlY; vlX.reserve(il.NumBinsZ);
            for (int iz = 0; iz < il.NumBinsZ; iz++)
                vlY.push_back(il.Data[ix][iy][iz]);
            vlX.push_back(vlY);
        }
        data.push_back(vlX);
    }
    vl.push_back(data);

    QVariantList parameters;
        QVariantList num;
        num << il.NumBinsX << il.NumBinsY << il.NumBinsZ;
    parameters.push_back(num);
        QVariantList mmPerPix;
        mmPerPix << il.mmPerPixelX << il.mmPerPixelY << il.mmPerPixelZ;
    parameters.push_back(mmPerPix);
        QVariantList offset;
        offset << il.OffsetX << il.OffsetY << il.OffsetZ;
    parameters.push_back(offset);

    vl.push_back(parameters);

    return vl;
}
