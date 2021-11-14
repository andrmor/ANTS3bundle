#ifndef APHOTONSIMULATOR_H
#define APHOTONSIMULATOR_H

#include "aphoton.h"

#include <QObject>
#include <QString>

#include "TVector3.h"

class APhotonSimSettings;
class ANodeRecord;
class AOneEvent;
class APhotonTracer;
class QFile;
class QTextStream;
class ARandomHub;
class ADepositionFileHandler;
class AS1Generator;

class APhotonSimulator : public QObject
{
    Q_OBJECT

public:
    APhotonSimulator(const QString & dir, const QString & fileName, int id);
    ~APhotonSimulator();

public slots:
    void start();

protected:
    QString WorkingDir;
    QString ConfigFN;
    int     ID;

    APhotonSimSettings & SimSet;
    ARandomHub         & RandomHub;

    APhotonTracer * Tracer = nullptr;
    AOneEvent     * Event  = nullptr;

    APhoton Photon;

    int CurrentEvent = 0;
    int  EventsToDo  = 0;
    int  EventsDone  = 0;

    bool bStopRequested = false;  // no need
    bool bHardAbortWasTriggered = false; // no need
    bool fSuccess;

    //output
    QFile       * FileSensorSignals   = nullptr;
    QTextStream * StreamSensorSignals = nullptr;
    QFile       * FilePhotonBombs     = nullptr;
    QTextStream * StreamPhotonBombs   = nullptr;
    QFile       * FileTracks          = nullptr;
    QTextStream * StreamTracks        = nullptr;    // Tracer handles saving tracks to that stream

    //depo
    ADepositionFileHandler * DepoHandler = nullptr;
    AS1Generator           * S1Gen       = nullptr;

private:
    void    loadConfig();
    void    setupCommonProperties();

    void    setupPhotonBombs();
    void    simulatePhotonBombs();

    void    setupFromDepo();
    void    simulateFromDepo();

    void    terminate(const QString & reason);

    int     getNumPhotonsThisBomb();  // !!!*** custom
    void    simulatePhotonBombCluster(ANodeRecord & node); // !!!***  cluster is not needed anynore? just single node?
    void    generateAndTracePhotons(const ANodeRecord * node);
    bool    isInsideLimitingVolume(const double * r);    // no optimization: assuming they will not be used together \|
    bool    isInsideLimitingMaterial(const double * r);  // no optimization: assuming they will not be used together /|

    bool    simulateSingle();
    bool    simulateGrid();
    bool    simulateFlood();
    bool    simulateBombsFromFile();

    QString openOutput();
    void    saveEventMarker();
    void    saveSensorSignals();
    void    savePhotonBomb(ANodeRecord *node); // make a static method of the corresponding file handler !!!***
    void    reportProgress();

private:
    TVector3 ColDirUnitary;
    double   CosConeAngle;
    TString  LimitToVolume;  // !!!*** change to pointer?
    int      LimitToMaterial;
};

#endif // APHOTONSIMULATOR_H
