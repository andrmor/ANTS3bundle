#ifndef ADEMOMANAGER_H
#define ADEMOMANAGER_H

#include "a3farmnoderecord.h"

#include <QObject>
#include <QString>

#include <vector>

class ADispatcherInterface;
class A3WorkDistrConfig;

class ADemoManager : public QObject
{
    Q_OBJECT
public:
    static ADemoManager & getInstance();

private:
    ADemoManager();
    ~ADemoManager(){}

    ADemoManager(const ADemoManager&)            = delete;
    ADemoManager(ADemoManager&&)                 = delete;
    ADemoManager& operator=(const ADemoManager&) = delete;
    ADemoManager& operator=(ADemoManager&&)      = delete;

public:
    QString ResultsFileName = "demoresult.txt";

    QString ErrorString;

    bool bAbortRequested = false;

public slots:
    bool run(int numLocalProc = -1);

signals:
    void finished();

protected:
    ADispatcherInterface & Dispatch;

    std::vector<QString>   OutputFiles;

    bool configure(std::vector<A3FarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
};

#endif // ADEMOMANAGER_H
