#ifndef AFARM_SI_H
#define AFARM_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>

class AFarmHub;

class AFarm_SI : public AScriptInterface
{
    Q_OBJECT

public:
    AFarm_SI();

    AScriptInterface * cloneBase() const {return new AFarm_SI();}

public slots:
    void clearNodes();
    void addNode(QString Name, QString Address, int Port, int Cores, double SpeedFactor = 1.0);

private:
    AFarmHub & FarmHub;
};

#endif // AFARM_SI_H
