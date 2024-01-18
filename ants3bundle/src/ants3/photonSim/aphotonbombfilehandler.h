#ifndef APHOTONBOMBFILEHANDLER_H
#define APHOTONBOMBFILEHANDLER_H

#include "afilehandlerbase.h"
#include "aphotonsimsettings.h"
#include "anoderecord.h"

#include <QString>

#include <array>

class APhotonBombFileHandler : public AFileHandlerBase
{
public:
    APhotonBombFileHandler(ABombFileSettings & settings);

    QString formReportString() const;

protected:
    // statistics
    size_t Bombs = 0;
    size_t Photons = 0;
    size_t BombsWithAutoNumPhotons = 0;
    size_t EmptyEvents = 0;
    std::pair<double,double> MinMaxBombsPerEvent;
    std::pair<double,double> MinMaxPhotonsPerBomb;
    std::pair<double,double> MinMaxPhotonsPerEvent;
    std::pair<double,double> MinMaxTime;                    // [ns]
    std::array<std::pair<double,double>, 3> MinMaxPosition; // [mm]

protected:
    void dummyReadBinaryDataUntilNewEvent() override;
    void clearStatistics() override;
    void fillStatisticsForCurrentEvent() override;

private:
    //ABombFileSettings & Settings; // need? BaseSettings is enough?

    ANodeRecord TmpRecord;
};

#endif // APHOTONBOMBFILEHANDLER_H
