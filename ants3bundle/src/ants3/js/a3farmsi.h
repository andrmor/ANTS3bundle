#ifndef A3FARMSI_H
#define A3FARMSI_H

#include <QObject>
#include <QString>

class AFarmHub;

class A3FarmSI : public QObject
{
    Q_OBJECT

public:
    A3FarmSI(QObject * parent = nullptr);

public slots:

    void clearNodes();
    void addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor = 1.0);

private:
    AFarmHub & FarmHub;
};

#endif // A3FARMSI_H
