#ifndef APHOTONSIMMANAGER_H
#define APHOTONSIMMANAGER_H

#include "a3farmnoderecord.h"

#include <QObject>

class A3DispInterface;
class A3WorkDistrConfig;

class APhotonSimManager : public QObject
{
    Q_OBJECT

public:
    APhotonSimManager(A3DispInterface & Dispatch, QObject * parent);
    ~APhotonSimManager();

    QString ResultsFileName = "simresult.txt";

    QString ErrorString;

public slots:
    bool simulate(int numLocalProc = -1);

signals:
    void simFinished();

protected:
    A3DispInterface & Dispatch;

    std::vector<QString> OutputFiles;

    bool configureSimulation(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
};

#endif // APHOTONSIMMANAGER_H
