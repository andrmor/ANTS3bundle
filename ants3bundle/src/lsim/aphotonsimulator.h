#ifndef APHOTONSIMULATOR_H
#define APHOTONSIMULATOR_H

#include "aphoton.h"

#include <QObject>
#include <QString>

class APhotonSimSettings;
class ANodeRecord;
class AOneEvent;
class APhotonTracer;
class QFile;
class QTextStream;
class ARandomHub;
class AProgressReporter;
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

    QThread           * ProgressThread   = nullptr;
    AProgressReporter * ProgressReporter = nullptr;

    APhoton Photon;

    int CurrentEvent = 0;
    int  EventsToDo  = 0;
    int  EventsDone  = 0; //  !!!*** update copy in reporter thread using a queued signal/slot

    bool bStopRequested = false;
    bool bHardAbortWasTriggered = false;
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
    void loadConfig();
    void setupCommonProperties();

    void setupPhotonBombs();
    void simulatePhotonBombs();

    void setupFromDepo();
    void simulateFromDepo();

    void terminate(const QString & reason);
    void simulatePhotonBombCluster(ANodeRecord & node);
    void generateAndTracePhotons(const ANodeRecord * node);
    bool simulateSingle();
    bool simulateGrid();
    bool simulateFlood();
    bool simulateCustomNodes();
    bool simulateBombsFromFile();
    int  getNumPhotonsThisBomb();

    QString openOutput();
    void    saveEventMarker();
    void    saveSensorSignals();
    void    savePhotonBomb(ANodeRecord *node);
};

class AProgressReporter : public QObject
{
    Q_OBJECT
public:
    AProgressReporter(int & eventsDone, double msInterval) : EventsDone(eventsDone), Interval(msInterval) {}

    void start();
    void stop() {bRun = false;}

    int    & EventsDone;
    double   Interval = 100;
    bool     bRun     = true;
};

#endif // APHOTONSIMULATOR_H
