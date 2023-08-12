#include "apetcoincidencefinder.h"

APetCoincidenceFinder::APetCoincidenceFinder(size_t numScint, const std::string & eventsFileName, bool binaryInput) :
    NumScint(numScint)
{
    Files.push_back({eventsFileName, binaryInput});
}

void APetCoincidenceFinder::findCoincidences(const std::string & coincFileName)
{
    /*
    Config.InputFileName     = "BuilderOutput.bin"; Config.BinaryInput  = true;
    //Config.InputFileName     = "BuilderOutput.txt"; Config.BinaryInput = false;
    Config.LutFileName       = "LUT.txt";

    // output
    //Config.OutputFileName    = "CoincPairs.bin"; Config.BinaryOutput = true;
    Config.OutputFileName    = "CoincPairs.txt"; Config.BinaryOutput = false;
    Config.HeaderFileName    = "Header.hlm";
    Config.ExportLutFileName = "CrystalLUT.txt";

    // finder
    Config.FinderMethod      = FinderMethods::Basic;
    // Basic    - no energy splitting allowed, strict multiple rejection
    // Advanced - energy is allowed to be split within the same assembly, flexible multiple rejection

    // next two settings have effect only if "Advanced" FinderMethod is selected
    Config.GroupByAssembly = true;
    Config.RejectMultiples = RejectionMethods::EnergyWindow;
        // None        - multiples are allowed, the coincidence pair takes two strongest depositions
        // All         - all coincidences with the multiplicities larger than two are discarded
        // EnergyWndow - multiple coincidence is discarded only if there are more than two energy-accepted depositions

    Config.RejectSameHead    = true;
    Config.CoincidenceWindow = 4.0;      // [ns]

    Config.EnergyFrom        = 0.511 * 0.95; // [MeV]
    Config.EnergyTo          = 0.511 * 1.05;

    Config.TimeFrom          = 0;  // [ns]
    Config.TimeTo            = 1e50;
    */

    /*
    Lut LUT(Config.WorkingDirectory + '/' + Config.LutFileName);

    std::vector<EventRecord>     Events;
    std::vector<CoincidencePair> Pairs;

    const bool EnforceEnergyInReader = (Config.FinderMethod == FinderMethods::Basic);
    Reader reader(EnforceEnergyInReader);
    std::string error = reader.read(Events);
    if (!error.empty())
    {
        out(error);
        exit(2);
    }

    out("-->Sorting events");
    std::sort(Events.begin(), Events.end());

    out("-->Finding coincidences");
    switch (Config.FinderMethod)
    {
    case FinderMethods::Basic :
    {
        Finder1 cf(Events, LUT);
        cf.findCoincidences(Pairs);
    }
    break;
    case FinderMethods::Advanced :
    {
        Finder2 cf(Events, LUT);
        cf.findCoincidences(Pairs);
    }
    break;
    default:
        out("Unknown finder method");
        exit(20);
    }

    Writer writer;
    error = writer.write(Pairs);
    if (!error.empty())
    {
        out(error);
        exit(3);
    }

    Config.saveHeaderFile(Pairs.size());
*/
}

