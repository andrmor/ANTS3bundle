#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "aphotonsimsettings.h"
#include "afilehandlerbase.h"
#include "adeporecord.h"

#include <QString>
#include <map>
#include <array>

struct ADepoStatRecord
{
    size_t NumberOfTimes = 0;
    double TotalDepo     = 0;
    size_t InNumEvents   = 0;
};

class ADepositionFileHandler : public AFileHandlerBase
{
public:
    ADepositionFileHandler(APhotonDepoSettings & depoSettings);

    // statistics
    size_t EmptyEvents = 0;
    std::map<QString,ADepoStatRecord> SeenParticles;
    std::pair<double,double> MinMaxDepoEnergy;     // [keV]
    std::pair<double,double> MinMaxEnergyPerEvent; // [keV]
    std::pair<double,double> MinMaxTime;           // [ns]
    std::array<std::pair<double,double>, 3> MinMaxPosition; // [mm]

    // Deposition statistics
    bool    collectStatistics();
    QString formReportString() const;

private:
    APhotonDepoSettings & Settings;

    ADepoRecord TmpRecord;

    double OnStartLimit = 1e99;

    void clearStatistics();
    void fillStatisticsForCurrentEvent();
};

#endif // ADEPOSITIONFILEHANDLER_H
