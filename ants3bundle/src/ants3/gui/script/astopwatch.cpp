#include "astopwatch.h"

AStopWatch::AStopWatch()
{
    start();
}

void AStopWatch::start()
{
    startTime = std::chrono::steady_clock::now();
}

double AStopWatch::getSecondsFromStart() const
{
    std::chrono::steady_clock::time_point timeMarkNow = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(timeMarkNow - startTime).count() * 1e-6;
}
