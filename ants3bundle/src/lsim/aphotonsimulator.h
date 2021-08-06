#ifndef APHOTONSIMULATOR_H
#define APHOTONSIMULATOR_H

#include "aphoton.h"

#include <QObject>
#include <QString>

class APhotonSimSettings;
class ANodeRecord;
class AOneEvent;

class APhotonSimulator : public QObject
{
    Q_OBJECT

public:
    APhotonSimulator(const QString & fileName, const QString & dir, int id);

public slots:
    void start();

private slots:
    void onProgressTimer();

protected:
    QString ConfigFN;
    QString WorkingDir;
    int     ID;

    const APhotonSimSettings & SimSet;

    AOneEvent * Event;

    APhoton Photon;

    int     EventsProcessed = 0; //  !!!*** update copy in reporter thread using a queued signal/slot
    bool bStopRequested = false;
    bool bHardAbortWasTriggered = false;
    bool fSuccess;

private:
    void loadConfig();
    void setupNodeBased();
    void simulateNodeBased();
    void terminate(const QString & reason);
//    void simulateOneNode(const ANodeRecord & node);
//    void generateAndTracePhotons(AScanRecord *scs, double time0, int iPoint);
};

#endif // APHOTONSIMULATOR_H
