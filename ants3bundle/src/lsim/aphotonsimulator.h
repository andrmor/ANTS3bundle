#ifndef APHOTONSIMULATOR_H
#define APHOTONSIMULATOR_H

#include <QObject>
#include <QString>

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

    int     EventsProcessed = 0;

private:
    void terminate(const QString & reason);
};

#endif // APHOTONSIMULATOR_H
