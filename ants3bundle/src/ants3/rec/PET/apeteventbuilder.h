#ifndef APETEVENTBUILDER_H
#define APETEVENTBUILDER_H

#include "apeteventbuilderconfig.h"

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

// !!!*** std::endl --> '\n'
class APetEventBuilder
{
public:
    APetEventBuilder(size_t numScint, const std::string & fileName, bool binaryInput);
    ~APetEventBuilder();

    bool makeEvents(const std::string & outputFileName, bool binaryOutput);

    void configure(const APetEventBuilderConfig & config) {Config = config;}

private:
    int NumScint;
    std::vector<std::pair<std::string, bool>> Files;

    std::ofstream * outStream  = nullptr;
    std::mt19937_64 * RandEngine = nullptr;
    std::normal_distribution<double> * gauss = nullptr;

    std::string ErrorString;

    APetEventBuilderConfig Config;

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
