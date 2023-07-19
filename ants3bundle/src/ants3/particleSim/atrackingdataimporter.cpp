#include "atrackingdataimporter.h"
#include "aeventtrackingrecord.h"

#include <QDebug>

#include <fstream>
#include <sstream>

#include "TGeoManager.h"
#include "TGeoNode.h"

ATrackingDataImporter::ATrackingDataImporter(const QString & fileName) :
    FileName(fileName)
{
    //qDebug() << " DI--> Init";
    bBinaryInput = !isAscii();
    //qDebug() << " DI--> Binary?" << bBinaryInput;

    prepareImportResources(FileName);

    CurrentStatus = Initialization;
    bool ok = processFile();
    //qDebug() << " DI--> Init result:" << ok << "Current status should be 1:" << int(CurrentStatus);
}

ATrackingDataImporter::~ATrackingDataImporter()
{
    clearImportResources();
}

#include <QFile>
#include <QTextStream>
bool ATrackingDataImporter::isAscii() // !!!***
{
    QFile file(FileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) return true;

    QTextStream stream(&file);
    const QString first = stream.read(1);
    file.close();

    if (first == "#") return true;
    return false;
}

void ATrackingDataImporter::toStringVector(const std::string & line, std::vector<std::string> & vec) const
{
    vec.clear();
    std::stringstream ss(line);
    std::string s;
    while (getline(ss, s, ' '))
        vec.push_back(s);

    if (ss.eof()) return;

    if (ss.fail())
    {
        qCritical() << "Unexpected format of a line"; // !!!***
        exit(2111);
    }
}

bool ATrackingDataImporter::extractEvent(int iEvent, AEventTrackingRecord * EventRecord)
{
    ErrorString.clear();
    CurrentEventRecord = EventRecord;

    //qDebug() << " DI--> Asked to get event #" << iEvent << " Currently at:" << CurrentEvent;
    if (iEvent == CurrentEvent) return readCurrentEvent();

    if (iEvent < 0)
    {
        ErrorString = "Event number should be >= 0";
        return false;
    }
    if (FileEndEvent != -1 && iEvent > FileEndEvent)
    {
        ErrorString = "Event number is larger than the last event index in the file";
        return false;
    }

    bool ok = gotoEvent(iEvent);
    if (!ok) return false;

    return readCurrentEvent();
}

bool ATrackingDataImporter::gotoEvent(int iEvent)
{
    if (CurrentEvent == iEvent) return true;

    if (iEvent < CurrentEvent)
    {
        inStream->clear();
        inStream->seekg(0);
        CurrentEvent = -1;
    }

    SeekEvent = iEvent;
    return processFile(true);
}

bool ATrackingDataImporter::readCurrentEvent()
{
    //qDebug() << " DI-->Reading current event..." << ErrorString ;
    CurrentEventRecord->clear();
    return processFile();
}

bool ATrackingDataImporter::processFile(bool SeekMode)
{
    if (!ErrorString.isEmpty()) return false;

    while (!isEndReached())
    {
        if (!ErrorString.isEmpty()) return false;

        readBuffer();
        //if (bBinaryInput)
        //{
            if (inStream->eof()) break;
        //}
        //else
        //{
        //    if (currentLine.empty()) continue;  // !!!*** merge with binary?
        //}

        if      (isNewEvent())
        {
            //qDebug() << " DI--> New event detected";
            processNewEvent(SeekMode);
            if (SeekMode && CurrentEvent != SeekEvent) continue;
            else break;
        }
        else if (CurrentStatus == Initialization)
        {
            ErrorString = "Invalid format of the file";
            return false;
        }
        else if (isNewTrack()) processNewTrack(SeekMode);
        else                   processNewStep(SeekMode);
    }

    if (!ErrorString.isEmpty()) return false;
    if (isEndReached()) FileEndEvent = CurrentEvent;
    if (SeekMode && isEndReached())
    {
        ErrorString = QString("File end reached (cannot find event # %0)").arg(SeekEvent);
        return false;
    }
    if (isErrorInPromises())
    {
        ErrorString = "Untreated promises of secondaries remained on event finish!";
        return false;
    }

    return true;
}

bool ATrackingDataImporter::isEndReached() const
{
    return inStream->eof();
}

AImporterEventStart ATrackingDataImporter::getEventStart() const
{
    AImporterEventStart es;
    es.CurrentEvent = CurrentEvent;
    es.Position = inStream->tellg();
    return es;
}

void ATrackingDataImporter::setPositionInFile(const AImporterEventStart & es)
{
    CurrentEvent = es.CurrentEvent;
    inStream->seekg(es.Position);//, std::ios_base::beg);
}

int ATrackingDataImporter::countEvents()
{
    bool ok = gotoEvent(0);
    if (!ok) return 0;

    int numEvents = 1;
    while (gotoEvent(numEvents)) numEvents++;

    inStream->clear();
    gotoEvent(0);
    return numEvents;
}

void ATrackingDataImporter::readBuffer()
{
    if (bBinaryInput)
    {
        // EE - new event, F0 - new track, F8 - trasnportation step, FF - non-transport step
        *inStream >> binHeader;
        //qDebug() << "    header-->" << QString::number(binHeader, 16);
        if (inStream->eof()) return; //this is the proper way to reach end of file
        if (inStream->fail())
            ErrorString = "Error in header char input";
    }
    else
        //currentLine = inTextStream->readLine();
        std::getline(*inStream, currentLine);
}

bool ATrackingDataImporter::isNewEvent()
{
    if (!ErrorString.isEmpty()) return false;

    if (bBinaryInput)
    {
        // EE - new event, F0 - new track, F8 - trasnportation step, FF - non-transport step
        return (binHeader == 0xEE);
    }
    else
        //return currentLine.startsWith('#');
        return currentLine[0] == '#';
}

bool ATrackingDataImporter::isNewTrack()
{
    if (!ErrorString.isEmpty()) return false;

    if (bBinaryInput)
    {
        // EE - new event, F0 - new track, F8 - trasnportation step, FF - non-transport step
        return (binHeader == 0xF0);
    }
    else
        //return currentLine.startsWith('>');
        return currentLine[0] == '>'; // !!!*** case currentLine is empty!
}

void ATrackingDataImporter::prepareImportResources(const QString & FileName)
{
    if (bBinaryInput) inStream = new std::ifstream(FileName.toLatin1().data(), std::ios::in | std::ios::binary);
    else              inStream = new std::ifstream(FileName.toLatin1().data());

    if (!inStream->is_open())
    {
        ErrorString = "Failed to open file " + FileName;
        return;
    }
}

void ATrackingDataImporter::clearImportResources()
{
    delete inStream; inStream = nullptr;
}

int ATrackingDataImporter::extractEventId()
{
    int evId;
    if (bBinaryInput)
    {
        inStream->read((char*)&evId, sizeof(int));
        if (inStream->fail()) ErrorString = "Error in header char input";
        //qDebug() << " DI--> Extracted id of the event:" << evId << "  error?" << !ErrorString.isEmpty();
    }
    else
    {
        //qDebug() << "DI-->"<<currentLine;
        //currentLine.remove(0, 1);
        currentLine.erase(currentLine.begin());
        //bool bOK = false;
        //evId = currentLine.toInt(&bOK);
        //if (!bOK) ErrorString = "Error in conversion of event number to integer";
        evId = std::stoi(currentLine);
    }
    return evId;
}

void ATrackingDataImporter::readNewTrack(bool SeekMode)
{
    if (bBinaryInput)
    {
        //format:
        //trackId(int) parentTrackId(int) PartName(string0) X(double) Y(double) Z(double) time(double) kinEnergy(double) NextMat(int) NextVolNmae(string0) NextVolIndex(int)

        inStream->read((char*)&BtrackId,        sizeof(int));
        inStream->read((char*)&BparentTrackId,  sizeof(int));
        readString(BparticleName);
        inStream->read((char*)Bpos,           3*sizeof(double));
        inStream->read((char*)&Btime,           sizeof(double));
        inStream->read((char*)&BkinEnergy,      sizeof(double));
        inStream->read((char*)&BnextMat,        sizeof(int));
        readString(BnextVolName);
        inStream->read((char*)&BnextVolIndex,   sizeof(int));

        if (inStream->fail())
        {
            ErrorString = "Unexpected format of a new track binary record";
            return;
        }
    }
    else
    {
        //qDebug() << "NT:"<<currentLine;

        if (SeekMode) return;

        //currentLine.remove(0, 1);
        currentLine.erase(currentLine.begin());

        //TrackID ParentTrackID ParticleName   X Y Z Time E iMat VolName VolIndex
        //   0           1           2         3 4 5   6  7   8     9       10
        //inputSL = currentLine.split(' ', Qt::SkipEmptyParts);
        toStringVector(currentLine, inputSL);
        if (inputSL.size() != 11) ErrorString = "Bad format in new track line";
    }
}

bool ATrackingDataImporter::isPrimaryRecord() const
{
    if (bBinaryInput)
        return (BparentTrackId == 0);
    else
    {
        //int parTrIndex = inputSL[1].toInt();
        int parTrIndex = std::stoi(inputSL[1]);
        return (parTrIndex == 0);
    }
}

AParticleTrackingRecord * ATrackingDataImporter::createAndInitParticleTrackingRecord() const
{
    AParticleTrackingRecord * r = nullptr;

    if (bBinaryInput)
    {
        r = AParticleTrackingRecord::create(BparticleName.data());

        ATransportationStepData * step = new ATransportationStepData(Bpos,          // pos
                                                                     Btime,         // time
                                                                     BkinEnergy,    // E
                                                                     0,             // depoE
                                                                     "C");          // process = 'C' which is "Creation"
        step->setVolumeInfo(BnextVolName.data(), BnextVolIndex, BnextMat);
        r->addStep(step);
    }
    else
    {
        //TrackID ParentTrackID ParticleName   X Y Z Time E iMat VolName VolIndex
        //   0           1           2         3 4 5   6  7   8     9       10
        r = AParticleTrackingRecord::create(inputSL[2].data());

        ATransportationStepData * step = new ATransportationStepData(std::stod(inputSL[3]), // X
                                                                     std::stod(inputSL[4]), // Y
                                                                     std::stod(inputSL[5]), // Z
                                                                     std::stod(inputSL[6]), // time
                                                                     std::stod(inputSL[7]), // E
                                                                     0,                       // depoE
                                                                     "C");                    // process = 'C' which is "Creation"
        //step->setVolumeInfo(inputSL.at(9), inputSL.at(10).toInt(), inputSL.at(8).toInt());
        step->setVolumeInfo(inputSL[9].data(), std::stoi(inputSL[10]), std::stoi(inputSL[8]));
        r->addStep(step);
    }

    return r;
}

int ATrackingDataImporter::getNewTrackIndex() const
{
    if (bBinaryInput)
        return BtrackId;
    else
        return std::stoi(inputSL[0]);
}

void ATrackingDataImporter::updatePromisedSecondary(AParticleTrackingRecord *secrec)
{
    if (bBinaryInput)
    {
        secrec->updatePromisedSecondary(BparticleName.data(),  // p_name
                                        BkinEnergy,     // E
                                        Bpos[0],        // X
                                        Bpos[1],        // Y
                                        Bpos[2],        // Z
                                        Btime,          // time
                                        BnextVolName.data(),   //VolName
                                        BnextVolIndex,  //VolIndex
                                        BnextMat        //MatIndex
                                        );
    }
    else
    {
        //TrackID ParentTrackID ParticleName   X Y Z Time E iMat VolName VolIndex
        //   0           1           2         3 4 5   6  7   8     9       10
        secrec->updatePromisedSecondary(inputSL[2].data(),            // p_name
                                        std::stod(inputSL[7]),  // E
                                        std::stod(inputSL[3]),  // X
                                        std::stod(inputSL[4]),  // Y
                                        std::stod(inputSL[5]),  // Z
                                        std::stod(inputSL[6]),  // time
                                        inputSL[9].data(),            //VolName
                                        std::stoi(inputSL[10]),   //VolIndex
                                        std::stoi(inputSL[8])     //MatIndex
                                        );
    }
}

void ATrackingDataImporter::readNewStep(bool SeekMode)
{
    if (bBinaryInput)
    {
        // format for "T" processes:
        // bin:   [FF or F8] ProcName0 X Y Z Time KinE DirectDepoE iMatTo VolNameTo0 VolIndexTo numSec [secondaries]
        // for non-"T" process, iMatTo VolNameTo  VolIndexTo are absent
        if (binHeader == 0xF8 || binHeader == 0xFF)  //transport or non-transport
        {
            readString(BprocessName);
            inStream->read((char*)Bpos,           3*sizeof(double));
            inStream->read((char*)&Btime,           sizeof(double));
            inStream->read((char*)&BkinEnergy,      sizeof(double));
            inStream->read((char*)&BdepoEnergy,     sizeof(double));
            if (binHeader == 0xF8)
            {
                inStream->read((char*)&BnextMat,      sizeof(int));
                readString(BnextVolName);
                inStream->read((char*)&BnextVolIndex, sizeof(int));
            }
            int numSec = 0;
            inStream->read((char*)&numSec,            sizeof(int));
            BsecVec.resize(numSec);
            for (int i=0; i<numSec; i++)
                inStream->read((char*)&BsecVec[i],    sizeof(int));

            if (inStream->fail())
                if (!inStream->eof())
                {
                    ErrorString = "Unexpected format of a step in history/track binary file";
                    return;
                }
        }
        else ErrorString = "Unexpected header char for a step in history/track binary file";
    }
    else
    {
        //qDebug() << "PS:"<<currentLine;

        // format for "T" processes:
        // ProcName X Y Z Time KinE DirectDepoE iMatTo VolNameTo VolIndexTo [secondaries]
        //     0    1 2 3   4    5      6          7       8           9             ...
        // not that if energy depo is present on T step, it is in the previous volume!

        // format for all other processes:
        // ProcName X Y Z Time KinE DirectDepoE [secondaries]
        //     0    1 2 3   4    5      6           ...

        if (SeekMode) return;

        //inputSL = currentLine.split(' ', Qt::SkipEmptyParts);
        toStringVector(currentLine, inputSL);
        if (inputSL.size() < 7)
        {
            ErrorString = "Bad format in step line";
            return;
        }
        if (inputSL.front() == "T" && inputSL.size() < 10)
        {
            ErrorString = "Bad format in tracking line (transportation step)";
            return;
        }
    }
}

void ATrackingDataImporter::addHistoryStep()
{
    ATrackingStepData * step = ( isTransportationStep() ? createHistoryTransportationStep()
                                                        : createHistoryStep() );
    CurrentParticleRecord->addStep(step);

    readSecondaries();
    for (int index : BsecVec)
    {
        //if (PromisedSecondaries.contains(index))
        if (PromisedSecondaries.find(index) != PromisedSecondaries.end())
        {
            ErrorString = QString("Error: secondary with index %1 was already promised").arg(index);
            return;
        }
        AParticleTrackingRecord * sr = AParticleTrackingRecord::create(); //empty!
        step->Secondaries.push_back(CurrentParticleRecord->countSecondaries());
        CurrentParticleRecord->addSecondary(sr);
        //PromisedSecondaries.insert(index, sr);
        PromisedSecondaries[index] = sr;
    }
}

ATrackingStepData *ATrackingDataImporter::createHistoryTransportationStep() const
{
    ATransportationStepData * step;

    if (bBinaryInput)
    {
        step = new ATransportationStepData(Bpos[0],         // X
                                           Bpos[1],         // Y
                                           Bpos[2],         // Z
                                           Btime,           // time
                                           BkinEnergy,      // energy
                                           BdepoEnergy,     // depoE
                                           BprocessName.data());   // pr

        step->setVolumeInfo(BnextVolName.data(), BnextVolIndex, BnextMat);
    }
    else
    {
        // ProcName X Y Z Time KinE DirectDepoE iMatTo VolNameTo VolIndexTo [secondaries]
        //     0    1 2 3   4    5      6          7       8           9         ...

        step = new ATransportationStepData(std::stod(inputSL[1]), // X
                                           std::stod(inputSL[2]), // Y
                                           std::stod(inputSL[3]), // Z
                                           std::stod(inputSL[4]), // time
                                           std::stod(inputSL[5]), // energy
                                           std::stod(inputSL[6]), // depoE
                                           inputSL[0].data());    // pr

        step->setVolumeInfo(inputSL[8].data(), std::stoi(inputSL[9]), std::stod(inputSL[7]));
    }

    return step;
}

ATrackingStepData * ATrackingDataImporter::createHistoryStep() const
{
    ATrackingStepData * step;

    if (bBinaryInput)
    {
        step = new ATrackingStepData(Bpos,            // [x,y,z]
                                     Btime,           // time
                                     BkinEnergy,      // energy
                                     BdepoEnergy,     // depoE
                                     BprocessName.data());   // pr
    }
    else
    {
        // ProcName X Y Z Time KinE DirectDepoE [secondaries]
        //     0    1 2 3   4    5      6           ...

        step = new ATrackingStepData(std::stod(inputSL[1]), // X
                                     std::stod(inputSL[2]), // Y
                                     std::stod(inputSL[3]), // Z
                                     std::stod(inputSL[4]), // time
                                     std::stod(inputSL[5]), // energy
                                     std::stod(inputSL[6]), // depoE
                                     inputSL[0].data());    // pr
    }

    step->extractTargetIsotope();

    return step;
}

void ATrackingDataImporter::readSecondaries()
{
    if (bBinaryInput)
    {
        //already done;
    }
    else
    {
        BsecVec.clear();
        const int secIndex = (inputSL.at(0) == "T" ? 10 : 7);
        for (int i = secIndex; i < inputSL.size(); i++)
            BsecVec.push_back( std::stoi(inputSL[i]) );
    }
}

bool ATrackingDataImporter::isTransportationStep() const
{
    if (bBinaryInput)
        return (binHeader == 0xF8);
    else
    {
        return (inputSL.at(0) == "T");
    }
}

void ATrackingDataImporter::readString(std::string & str) const
{
    char ch;
    str.clear();
    while (*inStream >> ch)
    {
        if (ch == char(0x00)) break;
        str += ch;
    }
}

void ATrackingDataImporter::processNewEvent(bool SeekMode)
{
    if (!ErrorString.isEmpty()) return;

    if (CurrentStatus == ExpectingStep)
    {
        ErrorString = "Unexpected start of event - single step in one record";
        return;
    }
    if (isErrorInPromises()) return; // container of promises should be empty at the end of event

    int evId = extractEventId();
    if (!ErrorString.isEmpty()) return;

    if (CurrentStatus == Initialization || SeekMode)
        CurrentEvent = evId;
    else
        CurrentEvent++;

    if (evId != CurrentEvent)
    {
        ErrorString = QString("Expected event #%1, but received #%2").arg(CurrentEvent).arg(evId);
        return;
    }

    CurrentStatus = ExpectingTrack;
}

void ATrackingDataImporter::processNewTrack(bool SeekMode)
{
    if (!ErrorString.isEmpty()) return;

    if (CurrentStatus == ExpectingEvent)
    {
        ErrorString = "Unexpected start of track - waiting new event";
        return;
    }
    if (CurrentStatus == ExpectingStep)
    {
        ErrorString = "Unexpected start of track - waiting for 1st step";
        return;
    }

    //qDebug() << " DI-->Reading new track";
    readNewTrack(SeekMode);
    if (!ErrorString.isEmpty()) return;

    if (!SeekMode)
    {
        // if primary (parent track == 0), create a new primary record in this event
        // else a pointer to empty record should be in the list of promised secondaries -> update the record
        if (isPrimaryRecord())
        {
            AParticleTrackingRecord * r = createAndInitParticleTrackingRecord();
            CurrentEventRecord->addPrimaryRecord(r);
            CurrentParticleRecord = r;
        }
        else
        {
            int trIndex = getNewTrackIndex();
            AParticleTrackingRecord * secrec = PromisedSecondaries[trIndex];  // !!!*** searches twice: here and below in erase!
            if (!secrec)
            {
                ErrorString = "Promised secondary not found!";
                return;
            }

            updatePromisedSecondary(secrec);
            CurrentParticleRecord = secrec;
            //PromisedSecondaries.remove(trIndex);
            PromisedSecondaries.erase(PromisedSecondaries.find(trIndex));
        }
    }

    CurrentStatus = ExpectingStep;
}

void ATrackingDataImporter::processNewStep(bool SeekMode)
{
    if (!ErrorString.isEmpty()) return;

    readNewStep(SeekMode);
    if (!ErrorString.isEmpty()) return;

    if (!SeekMode)
    {
        if (!CurrentParticleRecord)
        {
            ErrorString = "Attempt to add step when particle record does not exist";
            return;
        }
        addHistoryStep();
    }

    CurrentStatus = TrackOngoing;
}

bool ATrackingDataImporter::isErrorInPromises()
{
    return !PromisedSecondaries.empty();
}
