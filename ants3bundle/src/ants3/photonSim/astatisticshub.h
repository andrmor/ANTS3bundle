#ifndef ASTATISTICSHUB_H
#define ASTATISTICSHUB_H

#include "aphotonstatistics.h"

class AStatisticsHub
{
public:
    static AStatisticsHub & getInstance();

private:
    AStatisticsHub(){}
    ~AStatisticsHub(){}

    AStatisticsHub(const AStatisticsHub&)            = delete;
    AStatisticsHub(AStatisticsHub&&)                 = delete;
    AStatisticsHub& operator=(const AStatisticsHub&) = delete;
    AStatisticsHub& operator=(AStatisticsHub&&)      = delete;

public:
    APhotonStatistics SimStat;
};

#endif // ASTATISTICSHUB_H
