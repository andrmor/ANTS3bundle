#ifndef ADEMOMANAGER_H
#define ADEMOMANAGER_H

#include "afarmnoderecord.h"

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

    void abort();
    void abortedByScript() {bAborted = true;}

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


public slots:
    bool run(int numLocalProc = -1);

signals:
    void finished(bool bSuccess);

protected:
    ADispatcherInterface & Dispatch;
    std::vector<QString>   OutputFiles;
    bool                   bAborted = false;

    bool configure(std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
};

#endif // ADEMOMANAGER_H
