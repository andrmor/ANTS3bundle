#ifndef APETCOINCIDENCEFINDER_H
#define APETCOINCIDENCEFINDER_H

#include <string>
#include <vector>

class APetCoincidenceFinder
{
public:
    APetCoincidenceFinder(size_t numScint, const std::string & eventsFileName, bool binaryInput);

    void findCoincidences(const std::string & coincFileName);

private:
    int NumScint;
    std::vector<std::pair<std::string, bool>> Files;
};

#endif // APETCOINCIDENCEFINDER_H
