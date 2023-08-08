#ifndef ASTOPWATCH_H
#define ASTOPWATCH_H

#include <chrono>

class AStopWatch
{
public:
    AStopWatch();

    void   start();                     // !!!*** add mutex for multithread usage
    double getSecondsFromStart() const; // "granulation" is 1 mks

private:
    std::chrono::steady_clock::time_point startTime;
};

#endif // ASTOPWATCH_H
