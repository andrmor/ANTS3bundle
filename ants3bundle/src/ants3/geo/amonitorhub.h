#ifndef AMONITORHUB_H
#define AMONITORHUB_H

#include <vector>

class AMonitor;
class QString;

class AMonitorHub
{
public:
    static AMonitorHub & getInstance();
    static const AMonitorHub & getConstInstance();

private:
    AMonitorHub(){}
    ~AMonitorHub();

    AMonitorHub(const AMonitorHub&)            = delete;
    AMonitorHub(AMonitorHub&&)                 = delete;
    AMonitorHub& operator=(const AMonitorHub&) = delete;
    AMonitorHub& operator=(AMonitorHub&&)      = delete;

public:
    std::vector<AMonitor*> Monitors;

    void init();

    void appendFromFile(const QString & fileName);

private:
    void clear();
};

#endif // AMONITORHUB_H
