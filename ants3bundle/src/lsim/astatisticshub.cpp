#include "astatisticshub.h"

AStatisticsHub & AStatisticsHub::getInstance()
{
    static AStatisticsHub instance;
    return instance;
}

