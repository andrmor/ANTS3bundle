#ifndef A3PARTICLESIMMANAGER_H
#define A3PARTICLESIMMANAGER_H

#include "a3farmnoderecord.h"

#include <QObject>
#include <QStringList>

#include <vector>

class A3DispInterface;
class A3WorkDistrConfig;

class A3ParticleSimManager : public QObject
{
    Q_OBJECT

public:
    A3ParticleSimManager(A3DispInterface & Dispatch, QObject * parent);
    ~A3ParticleSimManager();

    QString ResultsFileName = "simresult.txt";

    QString ErrorString;

public slots:
    bool simulate(int numLocalProc = -1);

signals:
    void simFinished();

protected:
    A3DispInterface & Dispatch;

    QStringList OutputFiles;

    bool configureParticleSimulation(std::vector<A3FarmNodeRecord> &RunPlan, A3WorkDistrConfig & Request);
};

#endif // A3PARTICLESIMMANAGER_H
