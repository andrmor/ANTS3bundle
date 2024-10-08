#include "athreadpool.h"

#include <memory>
#include <functional>
#include <stdexcept>

AThreadPool::AThreadPool(size_t numThreads) : NumThreads(numThreads)
{
    for (size_t i = 0; i < numThreads; i++)
    {
        Workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->Mutex);
                        Condition.wait(lock,
                            [this]{ return StopRequested || !Jobs.empty(); });

                        if (StopRequested && Jobs.empty()) return;

                        task = std::move(Jobs.front());
                        Jobs.pop();
                    }

                    task();

                    {
                        std::lock_guard<std::mutex> lock(this->Mutex);
                        NumBusyThreads--;
                    }
                }
            }
        );
    }
}

AThreadPool::~AThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(Mutex);
        StopRequested = true;
        std::queue<std::function<void()>> empty;
        std::swap(Jobs, empty);
    }

    Condition.notify_all();

    for(std::thread & worker: Workers)
        worker.join();
}

void AThreadPool::addJob(std::function<void ()> newJob)
{
    {
        std::unique_lock<std::mutex> lock(Mutex);
        if (StopRequested) throw std::runtime_error("Cannot add job for already stopped AThreadPool");
        NumBusyThreads++;
        Jobs.push(newJob);
    }

    Condition.notify_one();
}

bool AThreadPool::isFull()
{
    std::lock_guard<std::mutex> lock(Mutex);
    return (NumBusyThreads == NumThreads);
}

bool AThreadPool::isIdle()
{
    if (!Jobs.empty()) return false; // fast

    std::lock_guard<std::mutex> lock(Mutex);
    return (NumBusyThreads == 0);
}
