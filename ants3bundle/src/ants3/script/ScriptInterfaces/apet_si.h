#ifndef APET_SI_H
#define APET_SI_H

#include "ascriptinterface.h"
#include "apeteventbuilderconfig.h"
#include "apetcoincidencefinderconfig.h"

#include <QObject>
#include <QVariantList>

class QProcess;

class APet_si : public AScriptInterface
{
    Q_OBJECT

public:
    APet_si();

    AScriptInterface * cloneBase() const {return new APet_si();}

public slots:
    void createScanner(QString scannerName, double scannerRadius, double crystalDepth, double crystalSize, double minAngle_deg);

    void configureBuilderTimeWindows(QVariantList arrayOfTimeFromAndTimeTo);
    void configureBuilderClustering(double maxTimeDeltaCluster, double clusterTime, double integrationTime, double deadTime);
    void configureBuilderSeed(int seed);
    void configureBuilderCTR(double CTR_ns);
    void configureBuilderEnergies(double energyResolution_fraction, double energyThreshold_keV);
    void buildEventsFromDeposition(QString depositionFileName, QString eventsFileName);

    void configureCoincidenceWindow(double coincidenceWindow_ns);
    void configureCoincidenceGlobalTimeRange(double from_ns, double to_ns);
    void configureCoincidenceEnergyWindow(double energyFrom_keV, double energyTo_keV);
    void findCoincidences(QString scannerName, QString eventsFileName, QString coincFileName, bool writeToF);

    void configureReconstructionVoxels(int numX, int numY, int numZ, double sizeX, double sizeY, double sizeZ);
    void configureReconstructionAlgorithm(QString algorithmName);
    void configureReconstructionIterations(int numIteration, int numSubsets);
    void configureReconstructionGaussianConvolver(double transaxialFWHM_mm, double axialFWHM_mm, double numberOfSigmas);
    void configureReconstructionIgnoreTOF(bool flag);
    void reconstruct(QString coincFileName, QString outDir, int numThreads);

    QVariantList loadImage(QString fileName);

private slots:
    void onReadReady();

private:
    bool makeLUT(QString fileName);

    QProcess * Process = nullptr;

    APetEventBuilderConfig      BuilderConfig;
    APetCoincidenceFinderConfig CoincidenceConfig;

    std::array<size_t, 3> NumVoxels  = {128, 128, 128};
    std::array<double, 3> SizeVoxels = {3.0, 3.0, 3.0};
    QString AlgorithmName = "MLEM";
    int NumIterations = 10;
    int NumSubsets = 16;
    double TransaxialFWHM_mm = 2.0;
    double AxialFWHM_mm = 2.5;
    double NumberOfSigmas = 3.5;
    bool   IgnoreTOF = false;
};

#endif // APET_SI_H
