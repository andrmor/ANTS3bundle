#ifndef APET_SI_H
#define APET_SI_H

#include "ascriptinterface.h"

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
    void buildEventsFromDeposition(QString depositionFileName, QString eventsFileName);
    void findCoincidences(QString eventsFileName, QString coincFileName, bool writeToF);

    void reconstructConfigureVoxels(int numX, int numY, int numZ, double sizeX, double sizeY, double sizeZ);
    void reconstruct(QString coincFileName, QString outDir, int numThreads);

    QVariantList loadImage(QString fileName);

private slots:
    void onReadReady();

private:
    bool makeLUT(QString fileName);

    QProcess * Process = nullptr;

    std::array<size_t, 3> NumVoxels  = {128, 128, 128};
    std::array<double, 3> SizeVoxels = {3.0, 3.0, 3.0};
};

#endif // APET_SI_H
