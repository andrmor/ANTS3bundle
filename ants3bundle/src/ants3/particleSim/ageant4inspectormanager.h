#ifndef AGEANT4INSPECTORMANAGER_H
#define AGEANT4INSPECTORMANAGER_H

#include "afarmnoderecord.h"

#include <QObject>
#include <QString>

#include <vector>

class ADispatcherInterface;
class A3WorkDistrConfig;

class AG4MaterialRecord
{
public:
    double  Density = 0;
    QString Formula;
};

class AGeant4InspectorManager : public QObject
{
    Q_OBJECT
public:
    static AGeant4InspectorManager & getInstance();

    void abort();
    void abortedByScript() {bAborted = true;}

private:
    AGeant4InspectorManager();
    ~AGeant4InspectorManager(){}

    AGeant4InspectorManager(const AGeant4InspectorManager&)            = delete;
    AGeant4InspectorManager(AGeant4InspectorManager&&)                 = delete;
    AGeant4InspectorManager& operator=(const AGeant4InspectorManager&) = delete;
    AGeant4InspectorManager& operator=(AGeant4InspectorManager&&)      = delete;

public:
    QString RequestFileName = "request.json";
    QString ResultsFileName = "response.json";

    QString ErrorString;

public slots:
    bool inspectMaterial(const QString & matName, AG4MaterialRecord & reply);

signals:
    void finished(bool bSuccess);

protected:
    ADispatcherInterface & Dispatch;
    std::vector<QString>   OutputFiles;
    bool                   bAborted = false;

    bool configureForInspectMaterial(const QString & matName, std::vector<AFarmNodeRecord> & RunPlan, A3WorkDistrConfig & Request);
};

#endif // AGEANT4INSPECTORMANAGER_H
