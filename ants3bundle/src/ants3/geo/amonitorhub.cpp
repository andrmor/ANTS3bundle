#include "amonitorhub.h"
#include "amonitor.h"
#include "ageometryhub.h"
#include "ageoobject.h"

AMonitorHub & AMonitorHub::getInstance()
{
    static AMonitorHub instance;
    return instance;
}

const AMonitorHub & AMonitorHub::getConstInstance()
{
    return getInstance();
}

AMonitorHub::~AMonitorHub()
{
    clear();
}

void AMonitorHub::appendFromFile(const QString & fileName)
{
    /*   !!!***
       if (Monitors.size() != from.Monitors.size())
       {
           qWarning() << "Cannot append monitor data - size mismatch:\n" <<
                         "Monitors here and in 'from':" << Monitors.size() << from.Monitors.size();
       }
       else
       {
           for (size_t i = 0; i < Monitors.size(); i++)
               Monitors[i]->appendDataFromAnotherMonitor(from.Monitors[i]);
       }
   */
}

void AMonitorHub::clear()
{
    for (AMonitorData & md : Monitors) delete md.Monitor;
    Monitors.clear();
}

