#ifndef APET_SI_H
#define APET_SI_H

#include "ascriptinterface.h"

#include <QObject>

class APet_si : public AScriptInterface
{
    Q_OBJECT

public:
    APet_si();

    AScriptInterface * cloneBase() const {return new APet_si();}

public slots:
    void createScanner(QString scannerName, double scannerRadius, double crystalDepth, double crystalSize);
    void buildEventsFromDeposition(QString depositionFileName, QString eventsFileName);
    void findCoincidences(QString eventsFileName, QString coincFileName, bool writeToF);

private:
    bool makeLUT(QString fileName);
};

#endif // APET_SI_H
