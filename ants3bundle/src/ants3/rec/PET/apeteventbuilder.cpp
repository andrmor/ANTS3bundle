#include "apeteventbuilder.h"

#include <math.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ios>

#include <QDebug>

DepositionNodeRecord::DepositionNodeRecord(double Time, double Energy) :
    time(Time), energy(Energy) {}

void DepositionNodeRecord::merge(const DepositionNodeRecord & other)
{
    if (other.energy <= 0) return;

    const double newEnergy = energy + other.energy;
    time = (time * energy  +  other.time * other.energy) / newEnergy;
    energy = newEnergy;
}

bool DepositionNodeRecord::isCluster(const DepositionNodeRecord &other, double maxTimeDelta) const
{
    if ( fabs(time - other.time) > maxTimeDelta ) return false;
    return true;
}

bool DepositionNodeRecord::operator<(const DepositionNodeRecord &other) const
{
    return (time < other.time);
}

EventRecord::EventRecord(double Time, double Energy) :
    time(Time), energy(Energy) {}

void EventRecord::merge(const DepositionNodeRecord & cluster)
{
    if (cluster.energy <= 0) return;
    energy = energy + cluster.energy;
    time = std::min(time, cluster.time);
}

bool EventRecord::operator<(const EventRecord & other) const
{
    return (time < other.time);
}

APetEventBuilder::APetEventBuilder(size_t numScint) :
    NumScint(numScint)
{
    RandEngine = new std::mt19937_64(Config.Seed + 1);
}

APetEventBuilder::~APetEventBuilder()
{
    delete RandEngine;
    delete gauss;
    if (outStream)
    {
        outStream->close();
        delete outStream;
    }
}

void APetEventBuilder::addInputFile(const std::string & fileName, bool binary)
{
    Files.push_back({fileName, binary});
}

double APetEventBuilder::makeEvents(const std::string & outputFileName, bool binaryOutput)
{
    if (Files.empty())
    {
        ErrorString = "No input files were provided!";
        return 0;
    }

    outStream = new std::ofstream();
    if (binaryOutput) outStream->open(outputFileName, std::ios::out | std::ios::binary);
    else              outStream->open(outputFileName);

    if (!outStream->is_open() || outStream->fail() || outStream->bad())
    {
        ErrorString = "Failed to open output file " + outputFileName;
        return false;
    }

    gauss = new std::normal_distribution<double>(0, Config.CTR / 2.355 / sqrt(2.0));

    double numEv = 0;
    for (const std::pair<double,double> & timeRange : Config.TimeRanges)
    {
        qDebug() << "==> Processing time range from" << timeRange.first << " ns to" << timeRange.second << " ns";

        std::vector<std::vector<DepositionNodeRecord>> Nodes;
        Nodes.resize(NumScint);
        std::vector<std::vector<EventRecord>> Events;
        Events.resize(NumScint);

        read(timeRange, Nodes);
        qDebug() << "  -->Reading completed";

        DepositionClusterer clusterer(Nodes, Config.ClusterTime);
        clusterer.cluster();
        qDebug() <<  "-->Clustering completed";

        build(Nodes, Events, Config.IntegrationTime, Config.DeadTime);
        qDebug() << "  -->Event building completed";

        if (Config.EnergyResolution != 0)
        {
            applyBlur(Events);
            qDebug() << "  -->Energy blurring completed";
        }

//        qDebug() << " --- final events:";
//        for (size_t iScint = 0; iScint < Events.size(); iScint++)
//            for (const EventRecord & e : Events[iScint])
//                qDebug() << iScint << e.time << e.energy;

        numEv += write(Events, binaryOutput);
        qDebug() << "  -->Events saved:" << numEv;
    }

    return numEv;
}

bool APetEventBuilder::read(const std::pair<double,double> & timeRange, std::vector<std::vector<DepositionNodeRecord>> & nodes)
{
    qDebug() << "Reading input files...";

    std::string particle;
    int iMat;
    double depo_keV, x,y,z, time;
    int iScint;

    // format: particle imat depo_keV x y z time iScint

    for (const auto & filePair : Files)
    {
        const std::string fn = filePair.first;
        const bool binary = filePair.second;
        qDebug() << "    Input file:" << fn.data() << " binary?" << binary;

        std::ifstream inStream;
        if (binary) inStream.open(fn, std::ios::in | std::ios::binary);
        else        inStream.open(fn);

        if (!inStream.is_open() || inStream.fail() || inStream.bad())
        {
            ErrorString = "Cannot open input file:\n" + fn;
            return false;
        }

        if (binary)
        {
            char ch;
            int iEvent;
            while (inStream.get(ch))
            {
                if (inStream.eof()) break;

                if (ch == (char)0xEE)
                {
                    inStream.read((char*)&iEvent, sizeof(int));
                    continue;
                }
                else if (ch == (char)0xFF)
                {
                    //std::string pn;
                    particle.clear();

                    while (inStream >> ch)
                    {
                        if (ch == (char)0x00) break;
                        particle += ch;
                    }

                    //particle = pn.data();
                    inStream.read((char*)&iMat,     sizeof(int));
                    inStream.read((char*)&depo_keV, sizeof(double));
                    inStream.read((char*)&x,        sizeof(double));
                    inStream.read((char*)&y,        sizeof(double));
                    inStream.read((char*)&z,        sizeof(double));
                    inStream.read((char*)&time,     sizeof(double));
                    inStream.read((char*)&iScint,   sizeof(int));
                    //if (stream.fail())
                    //{
                    //    AErrorHub::addError("ADepoRecord::readBinary: Unexpected format of a line in the binary file with the deposition data");
                    //    return false;
                    //}

                    if (time > timeRange.first && time < timeRange.second)
                    {
                        DepositionNodeRecord tmp{time, depo_keV};
                        if (!nodes[iScint].empty() && nodes[iScint].back().isCluster(tmp, Config.MaxTimeDeltaCluster))
                            nodes[iScint].back().merge(tmp);
                        else
                            nodes[iScint].push_back(std::move(tmp));
                    }
                }
            }
        }
        else
        {
            std::string line;

            while (!inStream.eof())
            {
                getline(inStream, line);
                //qDebug() << "  Loaded line:" << line;
                if (line.empty()) break;

                std::stringstream ss(line);

                const bool bNewEvent = (line[0] == '#');
                if (bNewEvent) continue;

                ss >> particle >> iMat >> depo_keV >> x >> y >> z >> time >> iScint;
                //qDebug() << "Extracted values:" << particle << iMat << depo_keV << x << y << z << time << iScint;

                if (time > timeRange.first && time < timeRange.second)
                {
                    DepositionNodeRecord tmp{time, depo_keV};
                    if (!nodes[iScint].empty() && nodes[iScint].back().isCluster(tmp, Config.MaxTimeDeltaCluster))
                        nodes[iScint].back().merge(tmp);
                    else
                        nodes[iScint].push_back(std::move(tmp));
                }
            }
        }

        inStream.close();
    }
    return true;
}

DepositionClusterer::DepositionClusterer(std::vector<std::vector<DepositionNodeRecord>> & nodes, double clusterTime) :
    Nodes(nodes), ClusterTime(clusterTime) {}

void DepositionClusterer::cluster()
{
    qDebug() << "Starting clustering (" << ClusterTime << "ns)";

    int numMergesClusterGlobal = 0;
    for (size_t iScint = 0; iScint < Nodes.size(); iScint++)
    {
        //out("Clustering for scint #:", iScint);
        std::vector<DepositionNodeRecord> & nvec = Nodes[iScint];
        if (nvec.empty()) continue;

//        if (true)
//        {
//            qDebug() << "==> #"<< iScint<< " nodes:"<< nvec.size();
//            qDebug() << " --- before clustering:";
//            outNodes(nvec);
//        }

        std::sort(nvec.begin(), nvec.end());

        int numMerges = 0;
        do
        {
            numMerges = doCluster(nvec);
            numMergesClusterGlobal += numMerges;
            //if (true) qDebug() << "Number of merges in clustering:"<< numMerges;
        }
        while (numMerges != 0);

//        if (true)
//        {
//            qDebug() << " --- after clustering:";
//            outNodes(Nodes[iScint]);
//        }
    }
    qDebug() << "-- Num clustering merges in total:"<< numMergesClusterGlobal;

    qDebug() << "\n<-Clustering done\n";
}

int DepositionClusterer::doCluster(std::vector<DepositionNodeRecord> & nvec)
{
    int iMergeCounter = 0;

    std::vector<DepositionNodeRecord> nvecClustered;
    nvecClustered.reserve(nvec.size());
    nvecClustered.push_back(nvec.front());

    for (size_t iNode = 1; iNode < nvec.size(); iNode++)
    {
        const DepositionNodeRecord & newNode = nvec[iNode];

        bool bFoundOldCluster = false;
        for (DepositionNodeRecord & oldCluster : nvecClustered)
        {
            if (oldCluster.isCluster(newNode, ClusterTime))
            {
                oldCluster.merge(newNode);
                bFoundOldCluster = true;
                iMergeCounter++;
                break;
            }
        }
        if (!bFoundOldCluster) nvecClustered.push_back(newNode);
    }

    nvec = nvecClustered;
    return iMergeCounter;
}

void DepositionClusterer::outNodes(const std::vector<DepositionNodeRecord> & nvec)
{
    for (const DepositionNodeRecord & n : nvec)
        qDebug() << "-->" << n.time << n.energy;
}

void APetEventBuilder::build(std::vector<std::vector<DepositionNodeRecord>> & clusters,
                             std::vector<std::vector<EventRecord>> & events,
                             double integrationTime, double deadTime)
{
    qDebug() << "Starting event building (" << integrationTime<< deadTime << ")...";

    for (size_t iScint = 0; iScint < clusters.size(); iScint++)
    {
        //out("Event building for scint #:", iScint);

        std::vector<DepositionNodeRecord> & cvec = clusters[iScint];
        if (cvec.empty()) continue;

//        if (true)
//        {
//            qDebug() << "==> #"<< iScint<< " clusters:"<< cvec.size();
//            qDebug() << " --- input clusters:";
//            for (const DepositionNodeRecord & c : cvec) qDebug() << c.time<< c.energy;
//        }

        std::sort(cvec.begin(), cvec.end()); // !!!*** need?

        std::vector<EventRecord> evVec;
        evVec.reserve(cvec.size());

        evVec.push_back( EventRecord(cvec.front().time, cvec.front().energy) );
        for (size_t iClust = 1; iClust < cvec.size(); iClust++)
        {
            DepositionNodeRecord & clust     = cvec[iClust];
            EventRecord          & lastEvent = evVec.back();
            if      (clust.time < lastEvent.time + integrationTime)
                lastEvent.merge(clust);
            else if (clust.time < lastEvent.time + deadTime)
                continue;
            else
                evVec.push_back( EventRecord(clust.time, clust.energy) );
        }

//        if (true)
//        {
//            qDebug() << " --- constructed events:";
//            for (const EventRecord & e : evVec) qDebug() << e.time << e.energy;
//        }

        events[iScint] = evVec;
    }

    qDebug() << "\n<-Event building done\n";
}

void APetEventBuilder::applyBlur(std::vector<std::vector<EventRecord>> & events) const
{
    for (std::vector<EventRecord> & evec : events)
        for (EventRecord & ev : evec)
        {
            //ev.energy = blurValue(ev.energy);
            std::normal_distribution<double> ND{ev.energy, ev.energy * Config.EnergyResolution / 2.355};
            ev.energy = ND(*RandEngine);
        }
}

size_t APetEventBuilder::write(std::vector<std::vector<EventRecord>> & events, bool binary)
{
    if (!outStream)
    {
        qCritical() << "Output stream does not exist!";
        exit(1);
    }

    qDebug() << "->Writing events to file...";

    size_t numEventsWritten = 0;
    for (size_t iScint = 0; iScint < events.size(); iScint++)
    {
        std::vector<EventRecord> & evec = events[iScint];
        if (evec.empty()) continue;

        // scint header
        if (binary)
        {
            *outStream << char(0xEE);
            outStream->write((char*)&iScint, sizeof(int));
        }
        else
            *outStream << "# " << iScint << '\n';

        // events
        for (EventRecord & ev : evec)
        {
            if (ev.energy < Config.EnergyThreshold) continue;

            if (Config.CTR != 0) blurTime(ev.time);

            if (binary)
            {
                *outStream << char(0xFF);
                outStream->write((char*)&ev.time,   sizeof(double));
                outStream->write((char*)&ev.energy, sizeof(double));
            }
            else
                *outStream << ev.time << " " << ev.energy << '\n';

        }
        numEventsWritten += evec.size();
    }

    qDebug() << "\n<-Write completed\n";
    return numEventsWritten;
}

void APetEventBuilder::blurTime(double & time)
{
    time += (*gauss)(*RandEngine);
}
