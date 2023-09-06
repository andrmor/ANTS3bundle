#ifndef APETEVENTBUILDERCONFIG_H
#define APETEVENTBUILDERCONFIG_H

#include <vector>

class APetEventBuilderConfig
{
public:
    std::vector<std::pair<double,double>> TimeRanges = {{0,1e50}};  // {from,to}  // ns
    double MaxTimeDeltaCluster       = 0.01;  // ns
    double ClusterTime               = 0.1;   // ns
    double IntegrationTime           = 40.0;  // ns
    double DeadTime                  = 100.0; // ns
    long   Seed                      = 1234;
    double CTR                       = 0.2;   // ns ->coincidence timing resolution
    double EnergyResolution          = 0.13;  // energy resolution (fraction, FWHM)
    double EnergyThreshold           = 10.0;  // keV
};

#endif // APETEVENTBUILDERCONFIG_H
