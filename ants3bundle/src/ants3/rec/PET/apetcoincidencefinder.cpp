#include "apetcoincidencefinder.h"

#include <fstream>
#include <sstream>

#include <QDebug>

APetCoincidenceFinder::APetCoincidenceFinder(size_t numScint, const std::string & eventsFileName, bool binaryInput) :
    NumScint(numScint)
{
    Files.push_back({eventsFileName, binaryInput});
}

bool APetCoincidenceFinder::findCoincidences(const std::string & coincFileName)
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
    */

    //Lut LUT(Config.WorkingDirectory + '/' + Config.LutFileName);

    std::vector<APetEventRecord>     Events;
    std::vector<APetCoincidencePair> Pairs;

    const bool EnforceEnergyInReader = (FinderMethod == FinderMethods::Basic);
    bool ok = read(Events, EnforceEnergyInReader);
    if (!ok) return false;

    qDebug() << "-->Sorting events";
    std::sort(Events.begin(), Events.end());

    qDebug() << "-->Finding coincidences";
    switch (FinderMethod)
    {
    case FinderMethods::Basic :
        findCoincidences(Events, Pairs);
        break;
    case FinderMethods::Advanced :
    {
        //Finder2 cf(Events, LUT);
        //cf.findCoincidences(Pairs);
    }
    break;
    default:
        qDebug() << "Unknown finder method";
        exit(20);
    }

    ok = write(Pairs, coincFileName, false);
    return ok;
}

bool APetCoincidenceFinder::read(std::vector<APetEventRecord> & events, bool bEnforceEnergyRange)
{
    for (const auto & pair : Files)
    {
        const std::string & fileName = pair.first;
        const bool binary = pair.second;
        qDebug() << "Input file:"<< fileName << "  binary?" << binary;

        std::ifstream * inStream;  // !!!*** drop pointer
        if (binary) inStream = new std::ifstream(fileName, std::ios::in | std::ios::binary);
        else        inStream = new std::ifstream(fileName);

        if (!inStream->is_open() || inStream->fail() || inStream->bad())
        {
            ErrorString = "Failed to open input file " + fileName;
            return false;
        }

        int iScint = 0;
        if (binary)
        {
            char ch;
            while (inStream->get(ch))
            {
                if (inStream->eof()) break;

                if (ch == (char)0xEE)
                {
                    inStream->read((char*)&iScint, sizeof(int));
                }
                else if (ch == (char)0xFF)
                {
                    APetEventRecord hit(iScint, 0, 0);
                    inStream->read((char*)&hit.Time,   sizeof(double));
                    inStream->read((char*)&hit.Energy, sizeof(double));

                    qDebug() << "Extracted values:" << hit.Time<< hit.Energy;

                    if (hit.Time < TimeFrom || hit.Time > TimeTo) continue;
                    if (bEnforceEnergyRange)
                        if (hit.Energy < EnergyFrom || hit.Energy > EnergyTo) continue;

                    events.push_back(hit);
                }
            }
        }
        else
        {
            std::string line;

            char dummy;
            double time, depo;

            while (!inStream->eof())
            {
                getline(*inStream, line);
                qDebug() << line;
                if (line.empty()) break;

                std::stringstream ss(line);

                if (line[0] == '#')
                {
                    //new scintillator
                    ss >> dummy >> iScint;
                }
                else
                {
                    //another node
                    ss >> time >> depo;
                    qDebug() << "Extracted time and depo values:"<< time<< depo;

                    if (time < TimeFrom || time > TimeTo) continue;
                    if (bEnforceEnergyRange)
                        if (depo < EnergyFrom || depo > EnergyTo) continue;

                    events.push_back( APetEventRecord(iScint, time, depo) );
                    qDebug() << "  -->Added to HitRecords";
                }
            }
        }

        inStream->close();
        qDebug() << "<-Read completed";
        qDebug() << "Loaded" << events.size()<<"events";
    }

    return true;
}

void APetEventRecord::print()
{
    qDebug() << "iScint:"<< iScint<< " Energy:"<< Energy<< " Time:"<< Time;
}

void APetCoincidenceFinder::findCoincidences(std::vector<APetEventRecord> & events, std::vector<APetCoincidencePair> & pairs)
{
    size_t numSingles    = 0;
    size_t numBadAngular = 0;

    for (size_t iCurrentEvent = 0; iCurrentEvent < events.size() - 1; iCurrentEvent++)
    {
        const APetEventRecord & thisEvent = events[iCurrentEvent];
        //qDebug() << iCurrentEvent<< "-->"<< thisEvent.iScint<< thisEvent.Time;

        size_t iNextEvent = iCurrentEvent + 1;
        const APetEventRecord & nextEvent = events[iNextEvent];

        if (nextEvent.Time > thisEvent.Time + CoincidenceWindow)
        {
            //large time gap, not interested in this hit
            numSingles++;
            continue;
        }

        //nextEvent is within the time window

        if (nextEvent.iScint == thisEvent.iScint) // should not happen with reasonably long integration / dead times
        {
            // same scint, not ineterested in this hit
            iCurrentEvent = findNextEventOutsideCoinsidenceWindow(events, iCurrentEvent) - 1; //cycle will auto-increment the index // number of events should be 2 or larger!
            continue;
        }

        //check that the nextnext is outside the window, otherwise disreguard all within the window
        size_t iCheckEvent = iNextEvent + 1;
        if (iCheckEvent >= events.size() || events[iCheckEvent].Time > thisEvent.Time + CoincidenceWindow)
        {
            //if (RejectSameHead)  // !!!*** TODO
            {
                //found a good coincidence!
                pairs.push_back(APetCoincidencePair(thisEvent, nextEvent));
            }
            //else numBadAngular++;

            iCurrentEvent = iNextEvent; // will be auto-incremented by the cycle
            continue;
        }

        //pass over all piled-up hits
        iCurrentEvent = findNextEventOutsideCoinsidenceWindow(events, iCurrentEvent) - 1; //cycle will auto-increment the index
    }

    qDebug() << "Found"<< pairs.size()<<"coincidences";
    qDebug() << "  Num singles:"<< numSingles;
    qDebug() << "  Num rejected based on detector head:"<< numBadAngular;
}

size_t APetCoincidenceFinder::findNextEventOutsideCoinsidenceWindow(std::vector<APetEventRecord> & events, size_t iCurrentEvent)
{
    size_t iOtherHit = iCurrentEvent;

    while ( events[iOtherHit].Time < (events[iCurrentEvent].Time + CoincidenceWindow) ) // dummy first cycle for safety
    {
        iOtherHit++;
        if (iOtherHit >= events.size()) break;
    }

    return iOtherHit;
}

bool APetCoincidenceFinder::write(std::vector<APetCoincidencePair> & pairs, const std::string & fileName, bool binary)
{
    std::ofstream * outStream = new std::ofstream;  // !!!*** remove pointer
    if (binary) outStream->open(fileName, std::ios::out | std::ios::binary);
    else        outStream->open(fileName);

    if (!outStream->is_open())
    {
        ErrorString = "Cannot open file " + fileName + " for writing";
        delete outStream; outStream = nullptr;
        return false;
    }

    qDebug() << "->Writing coincidences"<< "("<< pairs.size()<< ")"<< "to file " << fileName << "  binary?" << binary;

    for (const APetCoincidencePair & cp : pairs)
    {
        float dt    = (cp.Records[0].Time - cp.Records[1].Time) * 1000.0; // in ps
        float time0 = cp.Records[0].Time * 1e-6; // in ms

        if (binary)
        {
            outStream->write((char*)&cp.Records[0].iScint, sizeof(int));
            outStream->write((char*)&cp.Records[1].iScint, sizeof(int));
            outStream->write((char*)&dt,                   sizeof(float));
            outStream->write((char*)&time0,                sizeof(float));
        }
        else
        {
            *outStream << cp.Records[0].iScint << ','
                       << cp.Records[1].iScint << ','
                       << dt                   << ','
                       << time0                << '\n';
        }
    }

    outStream->close();
    delete outStream;

    qDebug() << "\n<-Coincidences are saved\n";

    return "";
}


