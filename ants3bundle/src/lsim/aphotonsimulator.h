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

class APhotonSimulator : public QObject
{
    Q_OBJECT

public:
    APhotonSimulator(const QString & fileName, const QString & dir, int id);
    ~APhotonSimulator();

public slots:
    void start();

private slots:
    void onProgressTimer();

protected:
    QString ConfigFN;
    QString WorkingDir;
    int     ID;

    const APhotonSimSettings & SimSet;

    APhotonTracer * Tracer = nullptr;
    AOneEvent     * Event  = nullptr;

    APhoton Photon;

    int CurrentEvent = 0;
    int  EventsToDo  = 0;
    int  EventsDone  = 0; //  !!!*** update copy in reporter thread using a queued signal/slot

    const int TrackOutputPrecision = 10000;

    bool bStopRequested = false;
    bool bHardAbortWasTriggered = false;
    bool fSuccess;

    //output
    QFile       * FileSensorSignals   = nullptr;
    QTextStream * StreamSensorSignals = nullptr;
    QFile       * FilePhotonBombs     = nullptr;
    QTextStream * StreamPhotonBombs   = nullptr;
    QFile       * FileTracks          = nullptr;
    QTextStream * StreamTracks        = nullptr;

private:
    void loadConfig();
    void setupCommonProperties();
    void setupPhotonBombs();
    void simulatePhotonBombs();
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
    void    saveTrack();
};

#endif // APHOTONSIMULATOR_H
