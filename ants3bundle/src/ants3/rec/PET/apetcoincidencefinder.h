#ifndef APETCOINCIDENCEFINDER_H
#define APETCOINCIDENCEFINDER_H

#include <string>
#include <vector>

struct APetEventRecord
{
    APetEventRecord(int scint, double time, double energy) : iScint(scint), Time(time), Energy(energy) {}
    APetEventRecord(){}

    bool operator<(const APetEventRecord & other) const {return Time < other.Time;}

    void print();

    int    iScint;
    double Time;
    double Energy;
};

struct APetCoincidencePair
{
    APetCoincidencePair(const APetEventRecord & Record1, const APetEventRecord & Record2) {Records[0] = Record1; Records[1] = Record2;}
    APetCoincidencePair(){}

    APetEventRecord Records[2];
};

enum class FinderMethods {Basic, Advanced};
// Basic    - no energy splitting allowed, strict multiple rejection
// Advanced - energy is allowed to be split within the same assembly, flexible multiple rejection

enum class RejectionMethods {None, All, EnergyWindow};
// None        - multiples are allowed, the coincidence pair takes two strongest depositions
// All         - all coincidences with the multiplicities larger than two are discarded
// EnergyWndow - multiple coincidence is discarded only if there are more than two energy-accepted depositions

class APetCoincidenceFinder
{
public:
    APetCoincidenceFinder(size_t numScint, const std::string & eventsFileName, bool binaryInput);

    bool findCoincidences(const std::string & coincFileName);

    std::string ErrorString;

private:
    int NumScint;
    std::vector<std::pair<std::string, bool>> Files;

    FinderMethods FinderMethod = FinderMethods::Basic;

    bool   GroupByAssembly   = true;
    RejectionMethods RejectMultiples = RejectionMethods::None;

    bool   RejectSameHead    = true;
    double CoincidenceWindow = 4.0; // [ns]

    // consider events only in this time range [ns]
    double TimeFrom          = 0;
    double TimeTo            = 1e20;

    // consider events only in this energy range [MeV]
    double EnergyFrom     = 0.95 * 511.0;
    double EnergyTo       = 1.05 * 511.0;

    bool   read(std::vector<APetEventRecord> & events, bool bEnforceEnergyRange);
    void   findCoincidences(std::vector<APetEventRecord> & events, std::vector<APetCoincidencePair> & pairs);
    size_t findNextEventOutsideCoinsidenceWindow(std::vector<APetEventRecord> & events, size_t iCurrentEvent);
    bool   write(std::vector<APetCoincidencePair> & pairs, const std::string & fileName, bool binary);
};

#endif // APETCOINCIDENCEFINDER_H
