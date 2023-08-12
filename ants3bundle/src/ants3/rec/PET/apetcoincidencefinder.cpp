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

    /*
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
    return true;
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
