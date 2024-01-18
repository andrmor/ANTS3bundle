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

    QString formReportString() const;

protected:
    // statistics
    size_t EmptyEvents = 0;
    std::map<QString,ADepoStatRecord> SeenParticles;
    std::pair<double,double> MinMaxDepoEnergy;              // [keV]
    std::pair<double,double> MinMaxEnergyPerEvent;          // [keV]
    std::pair<double,double> MinMaxTime;                    // [ns]
    std::array<std::pair<double,double>, 3> MinMaxPosition; // [mm]

protected:
    void dummyReadBinaryDataUntilNewEvent() override;
    void fillStatisticsForCurrentEvent() override;
    void clearStatistics() override;

private:
    //APhotonDepoSettings & Settings;

    ADepoRecord TmpRecord;

};

#endif // ADEPOSITIONFILEHANDLER_H
