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

    APhotonTracer * Tracer;
    AOneEvent * Event;

    APhoton Photon;

    int  EventsToDo = 0;
    int  EventsDone = 0; //  !!!*** update copy in reporter thread using a queued signal/slot

    bool bStopRequested = false;
    bool bHardAbortWasTriggered = false;
    bool fSuccess;

    //output
    QFile       * FileSensorSignals   = nullptr;
    QTextStream * StreamSensorSignals = nullptr;

private:
    void loadConfig();
    void setupCommonProperties();
    void setupPhotonBombs();
    void simulatePhotonBombs();
    void terminate(const QString & reason);
    void simulateOneNode(ANodeRecord & node);
    void generateAndTracePhotons(const ANodeRecord * node);
    bool simulateSingle();
    bool simulateGrid();
    bool simulateFlood();
    bool simulateCustomNodes();
    bool simulateBombsFromFile();
    int  getNumPhotonsThisBomb();

    QString openOutput();
    void    saveEventMarker();
    void    saveEventOutput();
};

#endif // APHOTONSIMULATOR_H
