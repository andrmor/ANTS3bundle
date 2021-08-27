#ifndef APHOTONSIMSI_H
#define APHOTONSIMSI_H

#include <QObject>

class APhotonSimManager;

class APhotonSimSI : public QObject
{
    Q_OBJECT

public:
    explicit APhotonSimSI(QObject * parent = nullptr);

public slots:
    QString simulate(bool updateGui);

private:
    APhotonSimManager & SimMan;

};

#endif // APHOTONSIMSI_H
