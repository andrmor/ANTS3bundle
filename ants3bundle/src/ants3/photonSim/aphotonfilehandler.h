#ifndef APHOTONFILEHANDLER_H
#define APHOTONFILEHANDLER_H

#include "afilehandlerbase.h"
#include "aphotonsimsettings.h"
#include "aphoton.h"

class APhotonFileHandler : public AFileHandlerBase
{
public:
    APhotonFileHandler(APhotonFileSettings & settings);

    QString formReportString() const;

protected:
    // statistics
    size_t EmptyEvents = 0;
    size_t Photons = 0;
    std::pair<double,double> MinMaxPhotonsPerEvent;
    std::pair<double,double> MinMaxTime;                         // [ns]
    std::array<std::pair<double,double>, 3> MinMaxPosition;      // [mm]
    std::array<std::pair<double,double>, 3> MinMaxUnitDirection;
    size_t NumberMinus1WaveIndex = 0;
    std::pair<double,double> MinMaxWaveIndex;

protected:
    void dummyReadBinaryDataUntilNewEvent() override;
    void clearStatistics() override;
    void fillStatisticsForCurrentEvent() override;

private:
    //APhotonFileSettings & Settings;
    APhoton TmpRecord;

};

#endif // APHOTONFILEHANDLER_H
