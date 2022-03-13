#ifndef ATHREADPOOL_H
#define ATHREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class AThreadPool
{
public:
    AThreadPool(size_t numThreads);
    ~AThreadPool();

    void addJob(std::function<void()> newJob);

    bool isFull();
    bool isIdle();

private:
    size_t NumThreads;
    std::vector<std::thread> Workers;
    std::queue<std::function<void()>> Jobs;

    std::mutex Mutex;
    std::condition_variable Condition;

    size_t NumBusyThreads = 0;

    bool   StopRequested = false;
};

#endif // ATHREADPOOL_H
