#ifndef APETEVENTBUILDER_H
#define APETEVENTBUILDER_H

#include <string>
#include <vector>
#include <random>

struct DepositionNodeRecord
{
    DepositionNodeRecord(double Time, double Energy);
    DepositionNodeRecord(){}

    void merge(const DepositionNodeRecord & other);
    bool isCluster(const DepositionNodeRecord & other, double maxTimeDelta) const;

    bool operator<(const DepositionNodeRecord & other) const;

    double time;
    double energy = 0;
};

struct EventRecord
{
    EventRecord(double Time, double Energy);

    void merge(const DepositionNodeRecord & cluster);

    bool operator<(const EventRecord & other) const;

    double time;
    double energy;
};

class APetEventBuilder
{
public:
    APetEventBuilder(size_t numScint, const std::string & fileName, bool binaryInput);
    ~APetEventBuilder();

    bool makeEvents(const std::string & outputFileName, bool binaryOutput);

    // !!!*** TODO
    //void configure(times, thresholds, CTR, etc);

private:
    int NumScint;
    std::vector<std::pair<std::string, bool>> Files;

    std::ofstream * outStream  = nullptr;
    std::mt19937_64 * RandEngine = nullptr;
    std::normal_distribution<double> * gauss = nullptr;

    std::string ErrorString;

    std::vector<std::pair<double,double>> TimeRanges = {{0,1e50}};
    const double MaxTimeDeltaCluster = 0.01;  // ns
    double ClusterTime               = 0.1;   // ns
    double IntegrationTime           = 40.0;  // ns
    double DeadTime                  = 100.0; // ns
    long   Seed                      = 1234;
    double CTR                       = 0.2;   // ns ->coincidence timing resolution
    double EnergyResolution          = 0.13;  // energy resolution (fraction, FWHM)
    double EnergyThreshold           = 10.0;  // keV


    bool read(const std::pair<double,double> & timeRange, std::vector<std::vector<DepositionNodeRecord>> & nodes);
    void build(std::vector<std::vector<DepositionNodeRecord>> & clusters, std::vector<std::vector<EventRecord>> & events, double integrationTime, double deadTime);
    void applyBlur(std::vector<std::vector<EventRecord>> & events) const;
    void write(std::vector<std::vector<EventRecord>> & events, bool binary); // only for energy above threshold, and simulates CTR blur
    void blurTime(double & time);
};

class DepositionClusterer
{
public:
    DepositionClusterer(std::vector< std::vector<DepositionNodeRecord>> & nodes, double clusterTime);

    void cluster();

private:
    std::vector< std::vector<DepositionNodeRecord>> & Nodes;
    double ClusterTime;

private:
    int  doCluster(std::vector<DepositionNodeRecord> & nvec);
    void outNodes(const std::vector<DepositionNodeRecord> &nvec);
};

#endif // APETEVENTBUILDER_H
